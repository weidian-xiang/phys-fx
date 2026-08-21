#!/usr/bin/env python3
# PhysFX Engine —— 视频世界编辑器（Video World Editor）
# Copyright (c) 2026 向伟典
# SPDX-License-Identifier: Apache-2.0

"""Audit the frozen C++ include boundary and exported symbol declarations."""

from __future__ import annotations

import argparse
import re
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
MODULES = (
    "core",
    "perception",
    "semantics",
    "editing",
    "physics",
    "compositing",
    "neural_render",
    "plugins",
    "platform",
    "cloud",
    "pipeline",
)


def public_headers(root: Path) -> list[Path]:
    headers: list[Path] = []
    for module in MODULES:
        headers.extend(sorted((root / module / "include" / "physfx" / module).rglob("*.h")))
    return headers


def audit(root: Path) -> list[str]:
    errors: list[str] = []
    headers = public_headers(root)
    if not headers:
        return ["未发现公共头文件"]

    api_markers = 0
    for header in headers:
        relative = header.relative_to(root).as_posix()
        text = header.read_text(encoding="utf-8")
        api_markers += text.count("PHYSFX_API")
        for line_number, line in enumerate(text.splitlines(), 1):
            include = re.search(r'#include\s+[<"]([^>"]+)[>"]', line)
            if include and ("/src/" in include.group(1) or include.group(1).startswith("apps/")):
                errors.append(f"{relative}:{line_number}: 公共头引用内部路径 {include.group(1)}")
            class_match = re.match(r"class\s+(?!PHYSFX_API\b)([A-Za-z_]\w*)", line)
            if class_match and class_match.group(1) != "Result":
                errors.append(
                    f"{relative}:{line_number}: 非模板公共 class 缺少 PHYSFX_API"
                )
            if re.match(r"\[\[nodiscard\]\]\s+(?!inline\b|PHYSFX_API\b)", line):
                errors.append(f"{relative}:{line_number}: 公共自由函数缺少 PHYSFX_API")
            if re.match(r"bool\s+[A-Za-z_]\w*\s*\(", line):
                errors.append(f"{relative}:{line_number}: 公共 bool 自由函数缺少 PHYSFX_API")

    export_header = root / "core/include/physfx/core/Export.h"
    version_header = root / "core/include/physfx/core/Version.h"
    if not export_header.exists() or "#define PHYSFX_API" not in export_header.read_text(
        encoding="utf-8"
    ):
        errors.append("Export.h 缺少 PHYSFX_API 定义")
    if not version_header.exists() or "namespace v1" not in version_header.read_text(
        encoding="utf-8"
    ):
        errors.append("Version.h 缺少 physfx::v1 入口")
    if api_markers < 25:
        errors.append(f"PHYSFX_API 标记数量异常：{api_markers}")
    return errors


def main() -> int:
    parser = argparse.ArgumentParser(description="PhysFX v1 公共 API 审计")
    parser.add_argument("--root", type=Path, default=ROOT)
    args = parser.parse_args()
    errors = audit(args.root.resolve())
    if errors:
        print("PUBLIC_API_AUDIT=FAIL")
        for error in errors:
            print(f"- {error}")
        return 1
    print(f"PUBLIC_API_AUDIT=PASS headers={len(public_headers(args.root.resolve()))}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
