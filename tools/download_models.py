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
import sys
import urllib.request
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


def download(url: str, target: Path, expected: str) -> bool:
    temporary = target.with_suffix(target.suffix + ".part")
    try:
        urllib.request.urlretrieve(url, temporary)
    except (OSError, ValueError) as exc:
        print(f"模型下载失败: {exc}; 可手动放置到 {target}", file=sys.stderr)
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


def install_lock() -> int:
    try:
        payload = json.loads(LOCK_FILE.read_text(encoding="utf-8"))
        records = payload["models"]
    except (OSError, json.JSONDecodeError, KeyError, TypeError) as exc:
        print(f"模型锁文件无效: {exc}", file=sys.stderr)
        return 2
    invalid = [record.get("id", "unknown") for record in records
               if not valid_sha256(record.get("sha256")) or not record.get("revision")]
    if invalid:
        print("模型供应链未锁定，拒绝下载: " + ", ".join(invalid), file=sys.stderr)
        print("必须先记录固定 revision、稳定下载 URL、64 位 SHA256 和许可审查结论。",
              file=sys.stderr)
        return 2
    MODELS.mkdir(parents=True, exist_ok=True)
    for record in records:
        target = MODELS / Path(record["filename"]).name
        expected = record["sha256"]
        if target.exists() and digest(target).lower() == expected.lower():
            print(f"模型已校验: {target} ({expected})")
            continue
        if not download(record["url"], target, expected):
            return 1
    return 0


def main() -> int:
    parser = argparse.ArgumentParser(description="准备 PhysFX 分割跟踪模型权重")
    parser.add_argument("--url", help="模型下载地址")
    parser.add_argument("--sha256", help="模型 SHA256")
    parser.add_argument("--name", default="model.onnx", help="保存文件名")
    parser.add_argument("--lock", action="store_true", help="按 docs/model-lock.json 下载全部锁定模型")
    args = parser.parse_args()
    if args.lock:
        return install_lock()
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
