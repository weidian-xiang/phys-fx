#!/usr/bin/env python3
# PhysFX Engine —— 视频世界编辑器（Video World Editor）
# Copyright (c) 2026 向伟典
#
# 本文件遵循 Apache License 2.0 开源协议，许可证全文见仓库根目录 LICENSE 文件。
# 按 Apache-2.0 要求，修改本文件的衍生版本须在文件中保留显著修改声明。
#
# SPDX-License-Identifier: Apache-2.0

"""Generate a stable semi-Lagrangian smoke density field.

The JSON contract is independent of the optional Taichi installation, so CI and
the zero-dependency C++ bridge can validate the same finite density output.
"""

import argparse
import json
import math
import time
from pathlib import Path


def advance_fallback(
    density: list[float],
    width: int,
    height: int,
    mode: str,
    source_x: int | None = None,
    source_y: int | None = None,
    step: int = 0,
) -> list[float]:
    """推进一个时间步，并在配置发射点注入新的密度。"""
    if len(density) != width * height:
        raise ValueError("密度场尺寸与网格配置不一致")
    source_x = width // 2 if source_x is None else max(0, min(width - 1, source_x))
    source_y = height * 3 // 4 if source_y is None else max(0, min(height - 1, source_y))
    previous = density[:]
    for y in range(height):
        for x in range(width):
            if mode == "smoke":
                source = min(height - 1, y + 1) * width + x
                left = previous[y * width + max(0, x - 1)]
                right = previous[y * width + min(width - 1, x + 1)]
                value = 0.985 * (0.5 * previous[source] + 0.25 * (left + right))
            else:
                left = previous[y * width + max(0, x - 1)]
                right = previous[y * width + min(width - 1, x + 1)]
                above = previous[max(0, y - 1) * width + x]
                below = previous[min(height - 1, y + 1) * width + x]
                value = 0.97 * (0.4 * previous[y * width + x] + 0.15 *
                                (left + right + above + below))
            density[y * width + x] = max(0.0, min(1.0, value))
    radius_limit = 2 if mode == "smoke" else min(8, 2 + step // 3)
    for radius in range(-radius_limit, radius_limit + 1):
        x = source_x + radius if mode == "splash" else source_x
        y = source_y if mode == "splash" else source_y + radius
        if 0 <= x < width and 0 <= y < height:
            density[y * width + x] = min(
                1.0, density[y * width + x] + 0.18 *
                (1.0 - abs(radius) / (radius_limit + 1.0))
            )
    return density


def solve_fallback(
    width: int,
    height: int,
    steps: int,
    mode: str,
    source_x: int | None = None,
    source_y: int | None = None,
) -> list[float]:
    density = [0.0] * (width * height)
    for step in range(steps):
        density = advance_fallback(density, width, height, mode, source_x, source_y, step)
    return density


def solve_taichi(
    width: int,
    height: int,
    steps: int,
    mode: str,
    source_x: int | None = None,
    source_y: int | None = None,
) -> list[float]:
    import taichi as ti

    ti.init(arch=ti.cpu, offline_cache=False, log_level=ti.ERROR)
    density = ti.field(dtype=ti.f32, shape=(width, height))
    scratch = ti.field(dtype=ti.f32, shape=(width, height))
    velocity = ti.field(dtype=ti.f32, shape=(width, height))
    next_velocity = ti.field(dtype=ti.f32, shape=(width, height))

    @ti.kernel
    def smoke_step():
        for x, y in density:
            source_y = ti.min(height - 1, y + 1)
            left_x = ti.max(0, x - 1)
            right_x = ti.min(width - 1, x + 1)
            scratch[x, y] = ti.max(0.0, ti.min(
                1.0, 0.985 * (0.5 * density[x, source_y] +
                              0.25 * (density[left_x, y] + density[right_x, y]))
            ))

    @ti.kernel
    def splash_step():
        for x, y in density:
            left_x = ti.max(0, x - 1)
            right_x = ti.min(width - 1, x + 1)
            above_y = ti.max(0, y - 1)
            below_y = ti.min(height - 1, y + 1)
            laplacian = (density[left_x, y] + density[right_x, y] +
                         density[x, above_y] + density[x, below_y] -
                         4.0 * density[x, y])
            next_velocity[x, y] = (velocity[x, y] + 0.16 * laplacian) * 0.985
            scratch[x, y] = ti.max(0.0, ti.min(
                1.0, density[x, y] + next_velocity[x, y]
            ))

    @ti.kernel
    def commit(use_velocity: ti.i32):
        for x, y in density:
            density[x, y] = scratch[x, y]
            if use_velocity == 1:
                velocity[x, y] = next_velocity[x, y]

    @ti.kernel
    def inject_smoke(source_x: ti.i32, source_y: ti.i32):
        for radius in range(-2, 3):
            y = source_y + radius
            if 0 <= y < height:
                density[source_x, y] = ti.min(
                    1.0, density[source_x, y] + 0.18 * (1.0 - ti.abs(radius) / 3.0)
                )

    @ti.kernel
    def inject_splash(source_x: ti.i32, source_y: ti.i32, radius_limit: ti.i32):
        for radius in range(-8, 9):
            if ti.abs(radius) <= radius_limit:
                x = source_x + radius
                if 0 <= x < width:
                    density[x, source_y] = ti.min(
                        1.0, density[x, source_y] + 0.18 *
                        (1.0 - ti.abs(radius) / (radius_limit + 1.0))
                    )

    source_x = width // 2 if source_x is None else max(0, min(width - 1, source_x))
    source_y = height * 3 // 4 if source_y is None else max(0, min(height - 1, source_y))
    for step in range(steps):
        if mode == "smoke":
            smoke_step()
            commit(0)
            inject_smoke(source_x, source_y)
        else:
            splash_step()
            commit(1)
            inject_splash(source_x, source_y, min(8, 2 + step // 3))
    return density.to_numpy().transpose().reshape(-1).tolist()


def main() -> int:
    parser = argparse.ArgumentParser(description="PhysFX Taichi 烟雾桥")
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--steps", type=int, default=1)
    parser.add_argument("--width", type=int, default=96)
    parser.add_argument("--height", type=int, default=54)
    parser.add_argument("--mode", choices=("smoke", "splash"), default="smoke")
    parser.add_argument("--anchor-x", type=int)
    parser.add_argument("--anchor-y", type=int)
    parser.add_argument("--require-taichi", action="store_true")
    args = parser.parse_args()
    if args.steps <= 0 or args.width < 8 or args.height < 8:
        print("steps/width/height 参数无效")
        return 2
    started = time.perf_counter()
    try:
        density = solve_taichi(args.width, args.height, args.steps, args.mode,
                               args.anchor_x, args.anchor_y)
        backend = "taichi-cpu"
    except ImportError as exc:
        if args.require_taichi:
            print(f"Taichi 运行时不可用: {exc}")
            return 4
        density = solve_fallback(args.width, args.height, args.steps, args.mode,
                                 args.anchor_x, args.anchor_y)
        backend = "cpu-fallback"
    if any(not math.isfinite(value) or value < 0.0 or value > 1.0 for value in density):
        print("密度场数值不稳定")
        return 3
    elapsed_ms = (time.perf_counter() - started) * 1000.0
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps({"version": 3, "backend": backend, "mode": args.mode,
                                       "width": args.width, "height": args.height,
                                       "steps": args.steps, "elapsed_ms": elapsed_ms,
                                       "density": density}),
                           encoding="utf-8")
    print(f"TAICHI_BRIDGE={backend} mode={args.mode} elapsed_ms={elapsed_ms:.2f}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
