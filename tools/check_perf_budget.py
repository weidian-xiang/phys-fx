#!/usr/bin/env python3
# PhysFX Engine —— 视频世界编辑器（Video World Editor）
# Copyright (c) 2026 向伟典
# SPDX-License-Identifier: Apache-2.0

from __future__ import annotations

import argparse
import re
from pathlib import Path


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--baseline", type=Path, required=True)
    args = parser.parse_args()
    text = args.baseline.read_text(encoding="utf-8")
    required = ["CPU", "P50", "P95", "GUI", "Taichi", "20%", "50%"]
    missing = [item for item in required if item not in text]
    if missing:
        raise SystemExit("性能预算缺少: " + ", ".join(missing))
    if "300 ms" not in text and "300ms" not in text:
        raise SystemExit("性能预算缺少 300 ms 预览目标")
    values = [float(value) for value in re.findall(r"GUI_BENCHMARK_FPS\s*=\s*([0-9.]+)", text)]
    if values and values[-1] < 15.0:
        raise SystemExit("GUI 预览低于 15 fps 门槛")
    print("PERFORMANCE_BUDGET=PASS regression_warning=20% regression_failure=50%")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
