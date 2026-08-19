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

"""准备可选实体依赖；默认只检查并给出明确的人工放置指引。"""

from __future__ import annotations

import argparse
import hashlib
import json
import shutil
import sys
import urllib.request
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
DEPS = ROOT / "third_party" / "cache"

MANIFEST = {
    "ffmpeg": {
        "version": "7.x-lgpl-shared",
        "url": "https://ffmpeg.org/download.html",
        "license": "LGPL-2.1-or-later",
        "destination": "ffmpeg",
    },
    "onnxruntime": {
        "version": "1.18.x",
        "url": "https://github.com/microsoft/onnxruntime/releases",
        "license": "MIT",
        "destination": "onnxruntime",
    },
}


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def main() -> int:
    parser = argparse.ArgumentParser(description="准备 PhysFX 可选第三方依赖")
    parser.add_argument("--component", choices=["ffmpeg", "onnxruntime", "all"], default="all")
    parser.add_argument("--manifest", type=Path, help="下载清单 JSON（可选）")
    parser.add_argument("--download", action="store_true", help="下载清单中明确的 URL")
    args = parser.parse_args()
    selected = MANIFEST if args.component == "all" else {args.component: MANIFEST[args.component]}
    DEPS.mkdir(parents=True, exist_ok=True)
    if args.manifest:
        try:
            selected = json.loads(args.manifest.read_text(encoding="utf-8"))
        except (OSError, json.JSONDecodeError) as exc:
            print(f"依赖清单读取失败: {exc}", file=sys.stderr)
            return 2
    for name, item in selected.items():
        destination = DEPS / item.get("destination", name)
        destination.mkdir(parents=True, exist_ok=True)
        print(f"{name}: 版本 {item.get('version', '未指定')}，许可 {item.get('license', '未指定')}")
        if args.download and item.get("archive_url"):
            archive = destination / Path(item["archive_url"]).name
            try:
                urllib.request.urlretrieve(item["archive_url"], archive)
            except (OSError, ValueError) as exc:
                print(f"下载失败: {exc}; 请手动下载并放入 {destination}", file=sys.stderr)
                return 1
            expected = item.get("sha256")
            if expected and sha256(archive).lower() != expected.lower():
                print(f"SHA256 校验失败: {archive}", file=sys.stderr)
                return 1
            print(f"已下载并校验: {archive}")
        elif not any(destination.iterdir()):
            print(f"未发现实体包。请按 third_party/versions.md 放入: {destination}")
    print("依赖准备检查完成；未启用的实体依赖不会影响默认构建。")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
