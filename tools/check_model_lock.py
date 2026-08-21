#!/usr/bin/env python3
"""
 * PhysFX Engine —— 视频世界编辑器（Video World Editor）
 * Copyright (c) 2026 向伟典
 *
 * 本文件遵循 Apache License 2.0 开源协议，许可证全文见仓库根目录 LICENSE 文件。
 * 按 Apache-2.0 要求，修改本文件的衍生版本须在文件中保留显著修改声明。
 *
 * SPDX-License-Identifier: Apache-2.0
 */
"""

# Validate all locked model files before a needs_models test is allowed to pass.

import argparse
import hashlib
import json
from pathlib import Path


def digest(path: Path) -> str:
    value = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            value.update(block)
    return value.hexdigest()


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--root", type=Path, required=True)
    args = parser.parse_args()
    lock_path = args.root / "docs" / "model-lock.json"
    payload = json.loads(lock_path.read_text(encoding="utf-8"))
    records = payload["models"]
    failures = []
    try:
        import torch
    except ImportError as exc:
        print(f"[FAIL] PyTorch is required for needs_models: {exc}")
        return 1
    for record in records:
        path = args.root / "models" / record["filename"]
        if not path.is_file():
            failures.append(f"{record['id']}: missing {path}")
            continue
        actual_size = path.stat().st_size
        actual_sha = digest(path)
        if actual_size != record["bytes"]:
            failures.append(f"{record['id']}: bytes expected={record['bytes']} actual={actual_size}")
        if actual_sha.lower() != record["sha256"].lower():
            failures.append(f"{record['id']}: sha256 expected={record['sha256']} actual={actual_sha}")
            continue
        try:
            checkpoint = torch.load(path, map_location="cpu", weights_only=False)
            if not hasattr(checkpoint, "keys") or not checkpoint:
                failures.append(f"{record['id']}: checkpoint has no parameter entries")
                continue
            parameter_count = len(checkpoint)
        except Exception as exc:  # PyTorch exposes format errors as several exception types.
            failures.append(f"{record['id']}: PyTorch load failed: {exc}")
            continue
        print(f"[PASS] {record['id']} bytes={actual_size} sha256={actual_sha} parameters={parameter_count}")
    if failures:
        for failure in failures:
            print(f"[FAIL] {failure}")
        return 1
    print(f"[PASS] {len(records)} locked model files verified")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
