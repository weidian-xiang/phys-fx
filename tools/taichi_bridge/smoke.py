#!/usr/bin/env python3
# PhysFX Engine —— 视频世界编辑器（Video World Editor）
# Copyright (c) 2026 向伟典
#
# 本文件遵循 Apache License 2.0 开源协议，许可证全文见仓库根目录 LICENSE 文件。
# 按 Apache-2.0 要求，修改本文件的衍生版本须在文件中保留显著修改声明。
#
# SPDX-License-Identifier: Apache-2.0

"""Taichi 烟雾桥的最小可验证入口；真实求解器将在 Phase 4 后续提交接入。"""

from __future__ import annotations

import argparse
import json
import math
from pathlib import Path


def main() -> int:
    parser = argparse.ArgumentParser(description="PhysFX Taichi 烟雾桥")
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--steps", type=int, default=1)
    args = parser.parse_args()
    try:
        import taichi as ti  # type: ignore
    except ImportError:
        print("未安装 taichi；请执行 python -m pip install taichi")
        return 2
    ti.init(arch=ti.cpu, offline_cache=True)
    density = [max(0.0, math.exp(-step / max(args.steps, 1))) for step in range(args.steps)]
    args.output.write_text(json.dumps({"version": 1, "density": density}), encoding="utf-8")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
