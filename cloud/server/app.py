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
    PLANS: dict[str, dict[str, Any]] = {
        "free": {"name": "免费档", "price_cents": 0, "quota_seconds": 600},
        "pro": {"name": "专业档", "price_cents": 1999, "quota_seconds": 7200},
        "team": {"name": "团队档（Phase 8 预留）", "price_cents": 9999, "quota_seconds": 36000},
    }

    def __init__(self, root: Path, quota_seconds: int = 600, retention_seconds: int = 86400,
                 billing_enabled: bool | None = None, webhook_secret: str | None = None,
                 admin_key: str | None = None) -> None:
        self.root = root
        self.root.mkdir(parents=True, exist_ok=True)
        self.db = sqlite3.connect(self.root / "physfx.sqlite3", check_same_thread=False)
        self.db.row_factory = sqlite3.Row
        self.lock = threading.Lock()
        self.quota_seconds = quota_seconds
        self.retention_seconds = retention_seconds
        self.billing_enabled = (os.environ.get("PHYSFX_BILLING_ENABLED", "0") == "1"
                                if billing_enabled is None else billing_enabled)
        self.webhook_secret = webhook_secret or os.environ.get("PHYSFX_STRIPE_WEBHOOK_SECRET", "")
        self.admin_key = admin_key or os.environ.get("PHYSFX_BILLING_ADMIN_KEY", "")
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
                    message TEXT NOT NULL DEFAULT '', estimated_seconds INTEGER NOT NULL DEFAULT 60
                );
                CREATE TABLE IF NOT EXISTS templates (
                    id TEXT PRIMARY KEY, version TEXT NOT NULL, publisher TEXT NOT NULL,
                    package_path TEXT NOT NULL, signature_status TEXT NOT NULL,
                    downloads INTEGER NOT NULL DEFAULT 0, revenue_share_bps INTEGER NOT NULL DEFAULT 0
                );
                CREATE TABLE IF NOT EXISTS orders (
                    id TEXT PRIMARY KEY, user_id TEXT NOT NULL, plan TEXT NOT NULL,
                    provider TEXT NOT NULL, idempotency_key TEXT NOT NULL,
                    amount_cents INTEGER NOT NULL, state TEXT NOT NULL,
                    created_at INTEGER NOT NULL, paid_at INTEGER NOT NULL DEFAULT 0,
                    UNIQUE(user_id, idempotency_key)
                );
                CREATE TABLE IF NOT EXISTS subscriptions (
                    id TEXT PRIMARY KEY, user_id TEXT NOT NULL, plan TEXT NOT NULL,
                    provider TEXT NOT NULL, state TEXT NOT NULL, current_period_end INTEGER NOT NULL,
                    UNIQUE(user_id, provider)
                );
                CREATE TABLE IF NOT EXISTS billing_ledger (
                    id TEXT PRIMARY KEY, user_id TEXT NOT NULL, kind TEXT NOT NULL,
                    amount_seconds INTEGER NOT NULL, amount_cents INTEGER NOT NULL DEFAULT 0,
                    idempotency_key TEXT UNIQUE NOT NULL, reference_id TEXT NOT NULL,
                    created_at INTEGER NOT NULL
                );
                CREATE TABLE IF NOT EXISTS webhook_events (
                    event_id TEXT PRIMARY KEY, provider TEXT NOT NULL, received_at INTEGER NOT NULL
                );
                CREATE TABLE IF NOT EXISTS refunds (
                    id TEXT PRIMARY KEY, order_id TEXT NOT NULL, user_id TEXT NOT NULL,
                    reason TEXT NOT NULL, created_at INTEGER NOT NULL, amount_cents INTEGER NOT NULL
                );
                """
            )
            columns = {row[1] for row in self.db.execute("PRAGMA table_info(jobs)").fetchall()}
            if "estimated_seconds" not in columns:
                self.db.execute("ALTER TABLE jobs ADD COLUMN estimated_seconds INTEGER NOT NULL DEFAULT 60")

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
            requested_seconds = max(1, min(int(payload.get("estimated_seconds", 60)), 86400))
            subscription = self.db.execute(
                "SELECT plan FROM subscriptions WHERE user_id=? AND state='active' AND current_period_end>?",
                (user["id"], int(time.time()))).fetchone()
            quota = (self.PLANS[subscription["plan"]]["quota_seconds"] if subscription else self.quota_seconds)
            reserved = self.db.execute(
                "SELECT COALESCE(SUM(estimated_seconds),0) AS seconds FROM jobs "
                "WHERE user_id=? AND state IN ('queued','running')", (user["id"],)).fetchone()["seconds"]
            if int(user["used_seconds"]) + int(reserved) + requested_seconds > quota:
                raise RuntimeError("本月云渲染配额不足；请减少任务时长或升级订阅，本地渲染仍完整可用")
            job_id = str(uuid.uuid4())
            now = int(time.time())
            self.db.execute(
                "INSERT INTO jobs(id,user_id,state,created_at,expires_at,input_name,message,estimated_seconds) VALUES(?,?,?,?,?,?,?,?)",
                (job_id, user["id"], "queued", now, now + self.retention_seconds,
                 str(payload.get("input", "")), "任务已入队", requested_seconds),
            )
            self.db.execute("UPDATE users SET used_seconds=used_seconds+? WHERE id=?",
                            (requested_seconds, user["id"]))
            self.db.commit()
            position = self.db.execute(
                "SELECT COUNT(*) AS position FROM jobs WHERE user_id=? AND state='queued' AND created_at<=?",
                (user["id"], now)).fetchone()["position"]
        return {"job_id": job_id, "state": "queued", "queue_position": int(position),
                "estimated_wait_seconds": max(0, int(position) - 1) * 60,
                "quota_seconds": max(0, quota - int(user["used_seconds"]) - requested_seconds)}

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
            row = self.db.execute("SELECT state,estimated_seconds FROM jobs WHERE id=? AND user_id=?",
                                  (job_id, user["id"])).fetchone()
            if row is None: raise KeyError("任务不存在")
            if row["state"] == "queued":
                self.db.execute("UPDATE users SET used_seconds=MAX(0,used_seconds-?) WHERE id=?",
                                (int(row["estimated_seconds"]), user["id"]))
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

    def billing_plans(self) -> dict[str, Any]:
        return {"enabled": self.billing_enabled, "provider": "stripe-sandbox",
                "plans": [{"id": key, **value} for key, value in self.PLANS.items()]}

    def create_order(self, key: str, plan: str, idempotency_key: str) -> dict[str, Any]:
        if not self.billing_enabled:
            raise RuntimeError("支付功能尚未开通；当前使用免费模式，本地渲染不受影响")
        user = self.user(key)
        if user is None: raise PermissionError("API Key 无效")
        if plan not in self.PLANS or plan == "free": raise ValueError("订阅档位无效；请选择专业档或团队档")
        if not idempotency_key or len(idempotency_key) > 128: raise ValueError("缺少有效幂等键")
        with self.lock, self.db:
            existing = self.db.execute("SELECT * FROM orders WHERE user_id=? AND idempotency_key=?",
                                       (user["id"], idempotency_key)).fetchone()
            if existing is not None: return dict(existing)
            order = (str(uuid.uuid4()), user["id"], plan, "stripe-sandbox", idempotency_key,
                     self.PLANS[plan]["price_cents"], "pending", int(time.time()), 0)
            self.db.execute("INSERT INTO orders VALUES(?,?,?,?,?,?,?,?,?)", order)
        return dict(self.db.execute("SELECT * FROM orders WHERE id=?", (order[0],)).fetchone())

    def apply_webhook(self, payload: dict[str, Any], signature: str) -> dict[str, Any]:
        if not self.billing_enabled: raise RuntimeError("支付功能尚未开通；Webhook 已拒绝")
        body = json.dumps(payload, ensure_ascii=False, separators=(",", ":"), sort_keys=True).encode()
        if self.webhook_secret and not hmac.compare_digest(
                hmac.new(self.webhook_secret.encode(), body, hashlib.sha256).hexdigest(), signature):
            raise PermissionError("Webhook 签名无效")
        event_id = str(payload.get("id", "")); order_id = str(payload.get("order_id", ""))
        if not event_id or not order_id: raise ValueError("Webhook 缺少 id 或 order_id")
        with self.lock, self.db:
            if self.db.execute("SELECT 1 FROM webhook_events WHERE event_id=?", (event_id,)).fetchone():
                return {"event_id": event_id, "duplicate": True}
            order = self.db.execute("SELECT * FROM orders WHERE id=?", (order_id,)).fetchone()
            if order is None: raise KeyError("订单不存在")
            self.db.execute("INSERT INTO webhook_events VALUES(?,?,?)", (event_id, "stripe-sandbox", int(time.time())))
            event_type = str(payload.get("type", ""))
            if event_type in {"payment_succeeded", "invoice_paid"}:
                self.db.execute("UPDATE orders SET state='paid',paid_at=? WHERE id=?", (int(time.time()), order_id))
                seconds = self.PLANS[order["plan"]]["quota_seconds"]
                self.db.execute("INSERT OR IGNORE INTO subscriptions VALUES(?,?,?,?,?,?)",
                                (str(uuid.uuid4()), order["user_id"], order["plan"], "stripe-sandbox", "active",
                                 int(time.time()) + 30 * 86400))
                self.db.execute("UPDATE subscriptions SET plan=?,state='active',current_period_end=? "
                                "WHERE user_id=? AND provider='stripe-sandbox'",
                                (order["plan"], int(time.time()) + 30 * 86400, order["user_id"]))
                self.db.execute("INSERT OR IGNORE INTO billing_ledger VALUES(?,?,?,?,?,?,?,?)",
                                (str(uuid.uuid4()), order["user_id"], "quota_credit", seconds,
                                 order["amount_cents"], "quota:" + order_id, "order:" + order_id,
                                 order["paid_at"] or int(time.time())))
            elif event_type == "payment_failed":
                self.db.execute("UPDATE orders SET state='failed' WHERE id=?", (order_id,))
            else: raise ValueError("不支持的支付事件类型")
        return {"event_id": event_id, "duplicate": False, "state": event_type}

    def refund(self, admin_key: str, order_id: str, reason: str) -> dict[str, Any]:
        if not self.billing_enabled: raise RuntimeError("支付功能尚未开通；无需退款")
        if not self.admin_key or not hmac.compare_digest(admin_key, self.admin_key): raise PermissionError("管理员凭据无效")
        if not reason.strip(): raise ValueError("退款必须填写原因")
        with self.lock, self.db:
            order = self.db.execute("SELECT * FROM orders WHERE id=?", (order_id,)).fetchone()
            if order is None or order["state"] != "paid": raise ValueError("订单不存在或未支付")
            existing = self.db.execute("SELECT * FROM refunds WHERE order_id=?", (order_id,)).fetchone()
            if existing is not None: return dict(existing)
            refund = (str(uuid.uuid4()), order_id, order["user_id"], reason, int(time.time()), order["amount_cents"])
            self.db.execute("INSERT INTO refunds VALUES(?,?,?,?,?,?)", refund)
            self.db.execute("UPDATE orders SET state='refunded' WHERE id=?", (order_id,))
            self.db.execute("UPDATE subscriptions SET state='refunded' WHERE user_id=?", (order["user_id"],))
            self.db.execute("INSERT OR IGNORE INTO billing_ledger VALUES(?,?,?,?,?,?,?,?)",
                            (str(uuid.uuid4()), order["user_id"], "refund_debit", 0, -order["amount_cents"],
                             "refund:" + order_id, "refund:" + order_id, int(time.time())))
        return dict(self.db.execute("SELECT * FROM refunds WHERE id=?", (refund[0],)).fetchone())

    def reconciliation(self, day: str | None = None) -> list[dict[str, Any]]:
        query = "SELECT * FROM billing_ledger"
        args: tuple[Any, ...] = ()
        if day:
            query += " WHERE created_at>=? AND created_at<?"
            start = int(time.mktime(time.strptime(day, "%Y-%m-%d")))
            args = (start, start + 86400)
        return [dict(row) for row in self.db.execute(query + " ORDER BY created_at", args).fetchall()]


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
            if self.path == "/v1/billing/orders":
                body = self._body()
                return self._json(HTTPStatus.CREATED, self.store.create_order(
                    self._key(), str(body.get("plan", "")), self.headers.get("Idempotency-Key", "")))
            if self.path == "/v1/billing/webhook":
                return self._json(HTTPStatus.OK, self.store.apply_webhook(
                    self._body(), self.headers.get("Stripe-Signature", "")))
            if self.path == "/v1/billing/refunds":
                body = self._body()
                return self._json(HTTPStatus.OK, self.store.refund(
                    self.headers.get("X-Admin-Key", ""), str(body.get("order_id", "")), str(body.get("reason", ""))))
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
            if self.path == "/v1/billing/plans":
                return self._json(HTTPStatus.OK, self.store.billing_plans())
            if self.path.startswith("/v1/billing/reconciliation"):
                day = self.path.split("?day=", 1)[1] if "?day=" in self.path else None
                return self._json(HTTPStatus.OK, {"entries": self.store.reconciliation(day)})
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
