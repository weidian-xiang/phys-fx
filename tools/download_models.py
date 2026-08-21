#!/usr/bin/env python3
"""
 * PhysFX Engine —— 视频世界编辑器（Video World Editor）
 * Copyright (c) 2026 向伟典
 *
 * 本文件遵循 Apache License 2.0 开源协议，许可证全文见仓库根目录 LICENSE 文件。
 * 按 Apache-2.0 要求，修改本文件的衍生版本须在文件中保留显著修改声明。
 *
 * SPDX-License-Identifier: Apache-2.0
"""

"""下载并校验模型权重；权重目录被 Git 忽略。"""

import argparse
import hashlib
import json
import re
import shutil
import sys
import urllib.error
import urllib.request
import zipfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
MODELS = ROOT / "models"
LOCK_FILE = ROOT / "docs" / "model-lock.json"


def digest(path: Path) -> str:
    sha = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            sha.update(block)
    return sha.hexdigest()


def valid_sha256(value: object) -> bool:
    return isinstance(value, str) and re.fullmatch(r"[0-9a-fA-F]{64}", value) is not None


def valid_positive_int(value: object) -> bool:
    return isinstance(value, int) and not isinstance(value, bool) and value > 0


def validate_record(record: object) -> str | None:
    if not isinstance(record, dict):
        return "记录不是对象"
    for key in ("id", "filename", "url", "revision", "format", "license", "tensor_contract"):
        if not isinstance(record.get(key), str) or not record[key].strip():
            return f"缺少 {key}"
    if Path(record["filename"]).name != record["filename"]:
        return "filename 不得包含目录"
    if not valid_sha256(record.get("sha256")):
        return "sha256 不是 64 位十六进制值"
    if not valid_positive_int(record.get("bytes")):
        return "bytes 必须是正整数"
    if not record["url"].startswith(("https://", "http://")):
        return "url 必须是 HTTP(S) 地址"
    return None


def download(
    url: str,
    target: Path,
    expected: str,
    expected_bytes: int | None = None,
    archive_member: str | None = None,
) -> bool:
    temporary = target.with_suffix(target.suffix + ".part")
    archive_temporary = target.with_suffix(target.suffix + ".archive.part")
    download_target = archive_temporary if archive_member else temporary
    try:
        request = urllib.request.Request(
            url,
            headers={
                "User-Agent": "PhysFX-model-fetch/1.0",
                "Accept": "application/octet-stream",
            },
        )
        with urllib.request.urlopen(request, timeout=120) as response, download_target.open("wb") as output:
            shutil.copyfileobj(response, output, length=1024 * 1024)
    except (OSError, ValueError, urllib.error.URLError) as exc:
        print(f"模型下载失败: {exc}; 可手动放置到 {target}", file=sys.stderr)
        temporary.unlink(missing_ok=True)
        archive_temporary.unlink(missing_ok=True)
        return False
    if archive_member:
        try:
            with zipfile.ZipFile(archive_temporary) as archive:
                member = archive.getinfo(archive_member)
                if member.is_dir() or Path(member.filename).name != member.filename:
                    raise ValueError(f"不安全或无效的 ZIP 成员: {member.filename}")
                with archive.open(member) as source, temporary.open("wb") as output:
                    shutil.copyfileobj(source, output, length=1024 * 1024)
        except (OSError, ValueError, zipfile.BadZipFile, KeyError) as exc:
            print(f"模型 ZIP 解包失败: {exc}", file=sys.stderr)
            temporary.unlink(missing_ok=True)
            archive_temporary.unlink(missing_ok=True)
            return False
        archive_temporary.unlink(missing_ok=True)
    if expected_bytes is not None and temporary.stat().st_size != expected_bytes:
        print(
            f"模型大小不匹配: 期望 {expected_bytes}，实际 {temporary.stat().st_size}",
            file=sys.stderr,
        )
        temporary.unlink(missing_ok=True)
        return False
    actual = digest(temporary)
    if actual.lower() != expected.lower():
        print(f"模型 SHA256 不匹配: 期望 {expected}，实际 {actual}", file=sys.stderr)
        temporary.unlink(missing_ok=True)
        return False
    temporary.replace(target)
    print(f"模型已准备: {target} ({actual})")
    return True


def install_lock(selected_ids: set[str] | None = None, force: bool = False) -> int:
    try:
        payload = json.loads(LOCK_FILE.read_text(encoding="utf-8"))
        records = payload["models"]
    except (OSError, json.JSONDecodeError, KeyError, TypeError) as exc:
        print(f"模型锁文件无效: {exc}", file=sys.stderr)
        return 2
    invalid = []
    for record in records:
        error = validate_record(record)
        if error:
            invalid.append(f"{record.get('id', 'unknown')}: {error}")
    if invalid:
        print("模型供应链未锁定，拒绝下载: " + ", ".join(invalid), file=sys.stderr)
        print("必须先记录固定 revision、官方下载 URL、文件大小、SHA256、许可和张量契约。", file=sys.stderr)
        return 2
    MODELS.mkdir(parents=True, exist_ok=True)
    for record in records:
        if selected_ids is not None and record["id"] not in selected_ids:
            continue
        target = MODELS / Path(record["filename"]).name
        expected = record["sha256"]
        expected_bytes = record["bytes"]
        if (not force and target.exists() and target.stat().st_size == expected_bytes and
                digest(target).lower() == expected.lower()):
            print(f"模型已校验: {target} ({expected})")
            continue
        print(f"准备下载 {record['id']} revision={record['revision']} format={record['format']}")
        if not download(
            record["url"],
            target,
            expected,
            expected_bytes,
            record.get("archive_member"),
        ):
            return 1
    return 0


def main() -> int:
    parser = argparse.ArgumentParser(description="准备 PhysFX 分割跟踪模型权重")
    parser.add_argument("--url", help="模型下载地址")
    parser.add_argument("--sha256", help="模型 SHA256")
    parser.add_argument("--name", default="model.onnx", help="保存文件名")
    parser.add_argument("--lock", action="store_true", help="按 docs/model-lock.json 下载全部锁定模型")
    parser.add_argument("--force", action="store_true", help="重新下载并校验已存在的锁定模型")
    parser.add_argument("--model", action="append", dest="model_ids",
                        help="只下载指定锁记录，可重复传入")
    args = parser.parse_args()
    if args.lock:
        return install_lock(set(args.model_ids) if args.model_ids else None, force=args.force)
    if Path(args.name).name != args.name:
        print("模型文件名不得包含目录", file=sys.stderr)
        return 2
    if args.url and not valid_sha256(args.sha256):
        print("下载模型必须提供 64 位 SHA256", file=sys.stderr)
        return 2
    MODELS.mkdir(parents=True, exist_ok=True)
    target = MODELS / args.name
    if not args.url:
        print("未指定模型 URL；请阅读 docs/model-cards.md 后使用 --url 与 --sha256。")
        return 0
    return 0 if download(args.url, target, args.sha256) else 1


if __name__ == "__main__":
    raise SystemExit(main())
