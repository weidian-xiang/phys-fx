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

from __future__ import annotations

import argparse
import json
import subprocess
import time
from pathlib import Path


def run(command: list[str], working_directory: Path) -> None:
    subprocess.run(command, cwd=working_directory, check=True)


def main() -> int:
    parser = argparse.ArgumentParser(description="运行真实视频粒子遮挡演示冒烟测试")
    parser.add_argument("--ffmpeg", type=Path, required=True)
    parser.add_argument("--ffprobe", type=Path, required=True)
    parser.add_argument("--physfx", type=Path, required=True)
    parser.add_argument("--config", type=Path, required=True)
    parser.add_argument("--root", type=Path, required=True)
    args = parser.parse_args()

    root = args.root.resolve()
    build = root / "build"
    build.mkdir(exist_ok=True)
    input_path = build / "demo-input.mp4"
    output_path = build / "particle-occlusion-demo.mp4"
    run(
        [
            str(args.ffmpeg.resolve()),
            "-y",
            "-f",
            "lavfi",
            "-i",
            "testsrc2=size=320x240:rate=30",
            "-t",
            "1",
            "-pix_fmt",
            "yuv420p",
            str(input_path),
        ],
        root,
    )
    started = time.perf_counter()
    run(
        [str(args.physfx.resolve()), "run", "--config", str(args.config.resolve())],
        root,
    )
    elapsed = time.perf_counter() - started
    probe = subprocess.run(
        [
            str(args.ffprobe.resolve()),
            "-v",
            "error",
            "-count_frames",
            "-select_streams",
            "v:0",
            "-show_entries",
            "stream=nb_read_frames,width,height",
            "-of",
            "json",
            str(output_path),
        ],
        cwd=root,
        check=True,
        capture_output=True,
        text=True,
        encoding="utf-8",
    )
    stream = json.loads(probe.stdout)["streams"][0]
    assert stream["width"] == 320 and stream["height"] == 240
    assert int(stream["nb_read_frames"]) == 30
    print(f"演示冒烟通过: 320x240, 30 帧, 管线耗时 {elapsed:.3f} 秒, 输出 {output_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
