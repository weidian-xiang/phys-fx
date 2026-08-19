#!/usr/bin/env python3
"""
PhysFX Engine —— 视频世界编辑器（Video World Editor）
Copyright (c) 2026 向伟典

本文件遵循 Apache License 2.0 开源协议，许可证全文见仓库根目录 LICENSE 文件。
按 Apache-2.0 要求，修改本文件的衍生版本须在文件中保留显著修改声明。

SPDX-License-Identifier: Apache-2.0
"""

from __future__ import annotations

import argparse
from pathlib import Path


OWNER = "向伟典"
MARKER = "SPDX-License-Identifier: Apache-2.0"

CPP_HEADER = f"""/*
 * PhysFX Engine —— 视频世界编辑器（Video World Editor）
 * Copyright (c) 2026 {OWNER}
 *
 * 本文件遵循 Apache License 2.0 开源协议，许可证全文见仓库根目录 LICENSE 文件。
 * 按 Apache-2.0 要求，修改本文件的衍生版本须在文件中保留显著修改声明。
 *
 * SPDX-License-Identifier: Apache-2.0
 */
"""

HASH_HEADER = f"""# PhysFX Engine —— 视频世界编辑器（Video World Editor）
# Copyright (c) 2026 {OWNER}
#
# 本文件遵循 Apache License 2.0 开源协议，许可证全文见仓库根目录 LICENSE 文件。
# 按 Apache-2.0 要求，修改本文件的衍生版本须在文件中保留显著修改声明。
#
# SPDX-License-Identifier: Apache-2.0
"""

SOURCE_SUFFIXES = {
    ".c",
    ".cc",
    ".cpp",
    ".cxx",
    ".h",
    ".hh",
    ".hpp",
    ".py",
    ".cmake",
}
# JSON、CMakePresets 和 vcpkg manifest 必须保持严格 JSON 语法，版权归属由 NOTICE/COPYRIGHT 覆盖。
SOURCE_NAMES = {"CMakeLists.txt"}
SKIP_PARTS = {".git", ".venv", ".idea", "build", "build-python", "build-python313", "out"}
SKIP_PREFIXES = {
    ("third_party", "cache"),
    ("third_party", "packages"),
    ("third_party", "ffmpeg"),
    ("third_party", "onnxruntime"),
    ("models",),
}


def is_source(path: Path) -> bool:
    return path.name in SOURCE_NAMES or path.suffix.lower() in SOURCE_SUFFIXES


def iter_sources(root: Path):
    for path in sorted(root.rglob("*")):
        if not path.is_file() or not is_source(path):
            continue
        try:
            relative_parts = path.relative_to(root).parts
        except ValueError:
            continue
        if any(part in SKIP_PARTS for part in relative_parts):
            continue
        if any(relative_parts[: len(prefix)] == prefix for prefix in SKIP_PREFIXES):
            continue
        yield path


def header_for(path: Path) -> str:
    return CPP_HEADER if path.suffix.lower() in {".c", ".cc", ".cpp", ".cxx", ".h", ".hh", ".hpp"} else HASH_HEADER


def has_header(text: str) -> bool:
    return MARKER in text[:1200]


def add_header(path: Path, text: str) -> str:
    header = header_for(path)
    if path.suffix.lower() == ".py" and text.startswith("#!"):
        first_line, separator, remainder = text.partition("\n")
        return f"{first_line}\n{header}\n{remainder}"
    return f"{header}\n{text}"


def main() -> int:
    parser = argparse.ArgumentParser(description="检查或补齐 PhysFX 源文件版权头")
    parser.add_argument("--root", type=Path, default=Path(__file__).resolve().parents[1])
    parser.add_argument("--fix", action="store_true", help="自动补齐缺失版权头")
    args = parser.parse_args()
    root = args.root.resolve()
    missing: list[Path] = []
    fixed = 0
    for path in iter_sources(root):
        text = path.read_text(encoding="utf-8")
        if has_header(text):
            continue
        missing.append(path)
        if args.fix:
            path.write_text(add_header(path, text), encoding="utf-8", newline="\n")
            fixed += 1
    if args.fix:
        print(f"版权头补齐: {fixed} 个文件")
    if missing and not args.fix:
        print("缺少版权头的文件:")
        for path in missing:
            print(f"- {path.relative_to(root)}")
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
