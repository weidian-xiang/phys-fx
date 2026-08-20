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

from __future__ import annotations

import argparse
import hashlib
import sys
import urllib.request
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
MODELS = ROOT / "models"


def digest(path: Path) -> str:
    sha = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            sha.update(block)
    return sha.hexdigest()


def main() -> int:
    parser = argparse.ArgumentParser(description="准备 PhysFX 分割跟踪模型权重")
    parser.add_argument("--url", help="模型下载地址")
    parser.add_argument("--sha256", help="模型 SHA256")
    parser.add_argument("--name", default="model.onnx", help="保存文件名")
    args = parser.parse_args()
    MODELS.mkdir(parents=True, exist_ok=True)
    target = MODELS / args.name
    if not args.url:
        print("未指定模型 URL；请阅读 docs/model-cards.md 后使用 --url 与 --sha256。")
        return 0
    try:
        urllib.request.urlretrieve(args.url, target)
    except (OSError, ValueError) as exc:
        print(f"模型下载失败: {exc}; 可手动放置到 {target}", file=sys.stderr)
        return 1
    actual = digest(target)
    if args.sha256 and actual.lower() != args.sha256.lower():
        print(f"模型 SHA256 不匹配: 期望 {args.sha256}，实际 {actual}", file=sys.stderr)
        target.unlink(missing_ok=True)
        return 1
    print(f"模型已准备: {target} ({actual})")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
