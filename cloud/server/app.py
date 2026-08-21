#!/usr/bin/env python3
# PhysFX Engine —— 视频世界编辑器（Video World Editor）
# Copyright (c) 2026 向伟典
#
# 本文件遵循 Apache License 2.0 开源协议，许可证全文见仓库根目录 LICENSE 文件。
# 按 Apache-2.0 要求，修改本文件的衍生版本须在文件中保留显著修改声明。
#
# SPDX-License-Identifier: Apache-2.0

"""Dependency-light Phase 6 cloud service.

This reference server intentionally uses only the Python standard library. It
is suitable for a single-machine deployment and keeps queue/storage/auth
interfaces replaceable for a later FastAPI/Redis deployment.
"""

from __future__ import annotations

import argparse
import hashlib
import hmac
import json
import os
import secrets
import sqlite3
import threading
import time
import uuid
from http import HTTPStatus
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
from typing import Any


class Store:
    def __init__(self, root: Path, quota_seconds: int = 600, retention_seconds: int = 86400) -> None:
        self.root = root
        self.root.mkdir(parents=True, exist_ok=True)
        self.db = sqlite3.connect(self.root / "physfx.sqlite3", check_same_thread=False)
        self.db.row_factory = sqlite3.Row
        self.lock = threading.Lock()
        self.quota_seconds = quota_seconds
        self.retention_seconds = retention_seconds
        with self.db:
            self.db.executescript(
                """
                CREATE TABLE IF NOT EXISTS users (
                    id TEXT PRIMARY KEY, email TEXT UNIQUE NOT NULL,
                    key_hash TEXT NOT NULL, created_at INTEGER NOT NULL,
                    used_seconds INTEGER NOT NULL DEFAULT 0
                );
                CREATE TABLE IF NOT EXISTS jobs (
                    id TEXT PRIMARY KEY, user_id TEXT NOT NULL, state TEXT NOT NULL,
                    created_at INTEGER NOT NULL, expires_at INTEGER NOT NULL,
                    input_name TEXT NOT NULL DEFAULT '', artifact_name TEXT NOT NULL DEFAULT '',
                    message TEXT NOT NULL DEFAULT ''
                );
                CREATE TABLE IF NOT EXISTS templates (
                    id TEXT PRIMARY KEY, version TEXT NOT NULL, publisher TEXT NOT NULL,
                    package_path TEXT NOT NULL, signature_status TEXT NOT NULL,
                    downloads INTEGER NOT NULL DEFAULT 0, revenue_share_bps INTEGER NOT NULL DEFAULT 0
                );
                """
            )

    @staticmethod
    def _hash_key(key: str) -> str:
        return hashlib.sha256(key.encode("utf-8")).hexdigest()

    def register(self, email: str) -> str:
        if "@" not in email:
            raise ValueError("邮箱地址无效")
        key = "pfx_" + secrets.token_urlsafe(24)
        with self.lock, self.db:
            self.db.execute(
                "INSERT OR IGNORE INTO users(id,email,key_hash,created_at) VALUES(?,?,?,?)",
                (str(uuid.uuid4()), email, self._hash_key(key), int(time.time())),
            )
        return key

    def user(self, key: str) -> sqlite3.Row | None:
        return self.db.execute("SELECT * FROM users WHERE key_hash=?", (self._hash_key(key),)).fetchone()

    def submit(self, key: str, payload: dict[str, Any]) -> dict[str, Any]:
        user = self.user(key)
        if user is None:
            raise PermissionError("API Key 无效")
        with self.lock:
            if int(user["used_seconds"]) >= self.quota_seconds:
                raise RuntimeError("本月云渲染配额已用尽；本地渲染仍完整可用")
            job_id = str(uuid.uuid4())
            now = int(time.time())
            self.db.execute(
                "INSERT INTO jobs(id,user_id,state,created_at,expires_at,input_name,message) VALUES(?,?,?,?,?,?,?)",
                (job_id, user["id"], "queued", now, now + self.retention_seconds,
                 str(payload.get("input", "")), "任务已入队"),
            )
            self.db.commit()
        return {"job_id": job_id, "state": "queued", "quota_seconds": self.quota_seconds - int(user["used_seconds"])}

    def query(self, key: str, job_id: str) -> dict[str, Any]:
        user = self.user(key)
        row = self.db.execute("SELECT * FROM jobs WHERE id=? AND user_id=?", (job_id, user["id"] if user else "")).fetchone()
        if user is None or row is None:
            raise KeyError("任务不存在")
        return dict(row)

    def cancel(self, key: str, job_id: str) -> None:
        user = self.user(key)
        if user is None:
            raise PermissionError("API Key 无效")
        with self.lock, self.db:
            self.db.execute("UPDATE jobs SET state='cancelled',message='用户取消' WHERE id=? AND user_id=?", (job_id, user["id"]))

    def delete_expired(self) -> int:
        now = int(time.time())
        with self.lock, self.db:
            rows = self.db.execute("SELECT id FROM jobs WHERE expires_at<?", (now,)).fetchall()
            self.db.execute("DELETE FROM jobs WHERE expires_at<?", (now,))
        return len(rows)

    def templates(self, keyword: str = "") -> list[dict[str, Any]]:
        rows = self.db.execute(
            "SELECT id,version,publisher,package_path,signature_status,downloads,revenue_share_bps FROM templates "
            "WHERE id LIKE ? OR publisher LIKE ? ORDER BY id", (f"%{keyword}%", f"%{keyword}%")
        ).fetchall()
        return [dict(row) for row in rows]


class Handler(BaseHTTPRequestHandler):
    server_version = "PhysFXCloud/0.5"

    def _json(self, status: int, body: dict[str, Any]) -> None:
        data = json.dumps(body, ensure_ascii=False).encode("utf-8")
        self.send_response(status)
        self.send_header("Content-Type", "application/json; charset=utf-8")
        self.send_header("Content-Length", str(len(data)))
        self.end_headers()
        self.wfile.write(data)

    def _body(self) -> dict[str, Any]:
        length = int(self.headers.get("Content-Length", "0"))
        return json.loads(self.rfile.read(length) or b"{}")

    def _key(self) -> str:
        return self.headers.get("X-API-Key", "")

    @property
    def store(self) -> Store:
        return self.server.store  # type: ignore[attr-defined]

    def do_POST(self) -> None:  # noqa: N802
        try:
            if self.path == "/v1/register":
                key = self.store.register(str(self._body().get("email", "")))
                return self._json(HTTPStatus.CREATED, {"api_key": key, "telemetry": "off"})
            if self.path == "/v1/render/jobs":
                return self._json(HTTPStatus.ACCEPTED, self.store.submit(self._key(), self._body()))
            if self.path.startswith("/v1/render/jobs/") and self.path.endswith("/cancel"):
                self.store.cancel(self._key(), self.path.split("/")[4])
                return self._json(HTTPStatus.OK, {"state": "cancelled"})
            return self._json(HTTPStatus.NOT_FOUND, {"error": "路径不存在"})
        except PermissionError as exc:
            return self._json(HTTPStatus.UNAUTHORIZED, {"error": str(exc)})
        except (KeyError, ValueError, RuntimeError) as exc:
            return self._json(HTTPStatus.BAD_REQUEST, {"error": str(exc)})

    def do_GET(self) -> None:  # noqa: N802
        try:
            if self.path.startswith("/v1/render/jobs/"):
                job_id = self.path.split("/")[4]
                return self._json(HTTPStatus.OK, self.store.query(self._key(), job_id))
            if self.path.startswith("/v1/templates"):
                keyword = self.path.split("?q=", 1)[1] if "?q=" in self.path else ""
                return self._json(HTTPStatus.OK, {"templates": self.store.templates(keyword)})
            return self._json(HTTPStatus.NOT_FOUND, {"error": "路径不存在"})
        except (PermissionError, KeyError):
            return self._json(HTTPStatus.UNAUTHORIZED, {"error": "API Key 或任务无效"})

    def do_DELETE(self) -> None:  # noqa: N802
        if self.path != "/v1/privacy/expired":
            return self._json(HTTPStatus.NOT_FOUND, {"error": "路径不存在"})
        deleted = self.store.delete_expired()
        return self._json(HTTPStatus.OK, {"deleted_jobs": deleted, "retention_seconds": self.store.retention_seconds})

    def log_message(self, format: str, *args: Any) -> None:
        return


def build_server(host: str = "127.0.0.1", port: int = 8765, root: Path | None = None) -> ThreadingHTTPServer:
    server = ThreadingHTTPServer((host, port), Handler)
    server.store = Store(root or Path(os.environ.get("PHYSFX_CLOUD_ROOT", ".physfx-cloud")))  # type: ignore[attr-defined]
    return server


def main() -> int:
    parser = argparse.ArgumentParser(description="PhysFX 单机云渲染服务")
    parser.add_argument("--host", default="127.0.0.1")
    parser.add_argument("--port", type=int, default=8765)
    parser.add_argument("--root", type=Path, default=None)
    args = parser.parse_args()
    server = build_server(args.host, args.port, args.root)
    print(f"PhysFX cloud listening on http://{args.host}:{args.port}")
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        return 0
    finally:
        server.server_close()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
