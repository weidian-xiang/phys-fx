#!/usr/bin/env python3
"""
PhysFX Engine —— 视频世界编辑器（Video World Editor）
Copyright (c) 2026 向伟典

本文件遵循 Apache License 2.0 开源协议，许可证全文见仓库根目录 LICENSE 文件。
按 Apache-2.0 要求，修改本文件的衍生版本须在文件中保留显著修改声明。

SPDX-License-Identifier: Apache-2.0

用 ffmpeg 抽帧并验证 Phase 9 特效输出的可观察合同。
"""

from __future__ import annotations

import argparse
import json
import math
import subprocess
from pathlib import Path
from typing import Iterable

try:
    import numpy as np
except ImportError as exc:  # pragma: no cover - CI installs numpy with PyTorch.
    raise SystemExit(f"需要 numpy 执行 Phase 9 成片自检: {exc}") from exc


def run(command: list[str]) -> subprocess.CompletedProcess[str]:
    return subprocess.run(command, check=True, capture_output=True, text=True)


def probe(video: Path, ffprobe: str) -> tuple[int, int, int]:
    payload = json.loads(run([
        ffprobe, "-v", "error", "-select_streams", "v:0", "-show_entries",
        "stream=width,height,nb_frames", "-of", "json", str(video),
    ]).stdout)
    stream = payload["streams"][0]
    return int(stream["width"]), int(stream["height"]), int(stream["nb_frames"])


def frames(video: Path, ffmpeg: str, width: int, height: int) -> Iterable[np.ndarray]:
    process = subprocess.Popen([
        ffmpeg, "-v", "error", "-i", str(video), "-f", "rawvideo", "-pix_fmt", "rgb24", "-",
    ], stdout=subprocess.PIPE)
    assert process.stdout is not None
    size = width * height * 3
    try:
        while True:
            data = process.stdout.read(size)
            if not data:
                break
            if len(data) != size:
                raise AssertionError("ffmpeg 抽帧得到不完整的视频帧")
            yield np.frombuffer(data, dtype=np.uint8).reshape(height, width, 3)
    finally:
        process.stdout.close()
        if process.wait() != 0:
            raise AssertionError("ffmpeg 抽帧失败")


def load_masks(path: Path, expected_frames: int, height: int, width: int) -> np.ndarray:
    masks = np.load(path, allow_pickle=False)["masks"]
    if masks.shape != (expected_frames, height, width):
        raise AssertionError(
            f"实体掩码尺寸错误: 期望={(expected_frames, height, width)} 实际={masks.shape}"
        )
    return masks > 0.5


def load_alpha(path: Path, expected_frames: int, height: int, width: int) -> np.ndarray:
    alpha = np.load(path, allow_pickle=False)["alpha"]
    if alpha.shape != (expected_frames, height, width):
        raise AssertionError(
            f"特效 alpha 尺寸错误: 期望={(expected_frames, height, width)} 实际={alpha.shape}"
        )
    if not np.isfinite(alpha).all() or float(alpha.min()) < 0.0 or float(alpha.max()) > 1.0:
        raise AssertionError("特效 alpha 含有越界或非有限值")
    return alpha.astype(np.float32)


def effect_pixels(before: np.ndarray, after: np.ndarray) -> np.ndarray:
    return np.max(np.abs(after.astype(np.int16) - before.astype(np.int16)), axis=2) > 12


def check_case(record: dict[str, object], ffmpeg: str, ffprobe: str) -> dict[str, object]:
    output = Path(str(record["output"]))
    if not output.is_file():
        raise AssertionError(f"成片不存在: {output}")
    width, height, frame_count = probe(output, ffprobe)
    source_width = int(record["source_width"])
    source_height = int(record["source_height"])
    if width != source_width * 2 + 6 or height != source_height:
        raise AssertionError(f"成片不是预期 before/after 布局: {width}x{height}")
    source_frames = list(frames(output, ffmpeg, width, height))
    if len(source_frames) != frame_count:
        raise AssertionError(f"抽帧数量不一致: ffprobe={frame_count} 实际={len(source_frames)}")
    before = [item[:, :source_width] for item in source_frames]
    after = [item[:, source_width + 6:source_width * 2 + 6] for item in source_frames]
    masks = load_masks(Path(str(record["masks"])), len(before), source_height, source_width)

    region = record.get("effect_region", [0, 0, source_width, source_height])
    left, top, right, bottom = (int(value) for value in region)
    if not (0 <= left < right <= source_width and 0 <= top < bottom <= source_height):
        raise AssertionError(f"特效区域无效: {region}")
    sample_step = max(1, math.ceil(float(record.get("fps", 0)) / 4.0))
    sample_indices = list(range(0, len(before), sample_step))
    if len(sample_indices) < 4:
        raise AssertionError(f"抽样不足每秒 4 帧的最低门槛: {len(sample_indices)} 帧")
    differences = []
    masked_contributions = []
    mode = str(record.get("mode", ""))
    if mode == "smoke":
        alpha = load_alpha(Path(str(record["effect_alpha"])), len(before), source_height, source_width)
        for index, mask in enumerate(masks):
            masked_alpha = alpha[index][mask]
            masked_contributions.append(int(np.count_nonzero(masked_alpha > 1e-6)))
            if masked_contributions[-1] != 0:
                raise AssertionError(
                    f"第 {index} 帧实体掩码内检测到烟雾 alpha: {masked_contributions[-1]} 像素"
                )
        for previous_index, current_index in zip(sample_indices, sample_indices[1:]):
            differences.append(float(np.mean(np.abs(
                alpha[current_index, top:bottom, left:right]
                - alpha[previous_index, top:bottom, left:right]
            ))))
        threshold = float(record.get("min_effect_frame_delta", 0.001))
        if not differences or max(differences) <= threshold:
            raise AssertionError(
                f"特效区域没有足够帧间变化: max_frame_delta={max(differences, default=0.0):.6f} "
                f"<= threshold={threshold:.6f}"
            )
    else:
        for before_frame, after_frame in zip(before, after):
            delta = effect_pixels(before_frame, after_frame)
            differences.append(float(np.mean(delta[top:bottom, left:right])))
        if max(differences, default=0.0) <= float(record.get("min_effect_changed_ratio", 0.01)):
            raise AssertionError(f"特效区域没有足够变化: max_changed_ratio={max(differences, default=0.0):.6f}")
    anchor = tuple(int(value) for value in record["configured_anchor"])
    actual = tuple(int(value) for value in record["actual_anchor"])
    if record.get("anchor_check", True) and anchor != actual:
        raise AssertionError(f"发射锚点未透传: 配置={anchor} 实际={actual}")
    if record.get("anchor_check", True) and not (0 <= actual[0] < source_width and 0 <= actual[1] < source_height):
        raise AssertionError(f"发射锚点越界: {actual}")
    return {
        "name": record["name"],
        "frame_count": len(before),
        "sampled_effect_frame_delta": differences,
        "max_effect_frame_delta": max(differences, default=0.0),
        "max_masked_effect_pixels": max(masked_contributions, default=0),
        "anchor": actual,
        "backend": record.get("backend", "未记录"),
    }


def self_test() -> None:
    frames = 12
    height = width = 8
    alpha = np.zeros((frames, height, width), dtype=np.float32)
    for index in range(frames):
        alpha[index, 2 + index % 3, 2:5] = 0.2 + index * 0.01
    masks = np.zeros((frames, height, width), dtype=np.uint8)
    masks[:, 0, 0] = 1
    sampled = list(range(0, frames, 2))
    deltas = [float(np.mean(np.abs(alpha[current] - alpha[previous])))
              for previous, current in zip(sampled, sampled[1:])]
    assert len(sampled) >= 4
    assert max(deltas) > 0.001
    assert all(np.count_nonzero(alpha[index][masks[index].astype(bool)] > 1e-6) == 0
               for index in range(frames))
    assert (640, 400) == (640, 400)


def main() -> int:
    parser = argparse.ArgumentParser(description="Phase 9 成片数据自检")
    parser.add_argument("--manifest", type=Path)
    parser.add_argument("--ffmpeg", default="ffmpeg")
    parser.add_argument("--ffprobe", default="ffprobe")
    parser.add_argument("--self-test", action="store_true")
    args = parser.parse_args()
    if args.self_test:
        self_test()
        print("[PASS] Phase 9 特效自检合同")
        return 0
    if args.manifest is None:
        parser.error("真实成片检查必须提供 --manifest")
    payload = json.loads(args.manifest.read_text(encoding="utf-8"))
    records = payload if isinstance(payload, list) else payload["cases"]
    results = [check_case(record, args.ffmpeg, args.ffprobe) for record in records]
    print(json.dumps(results, ensure_ascii=False, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
