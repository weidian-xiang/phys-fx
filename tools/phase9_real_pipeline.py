#!/usr/bin/env python3
"""
 * PhysFX Engine - Phase 9 real-weight video pipeline
 * Copyright (c) 2026 向伟典
 *
 * SPDX-License-Identifier: Apache-2.0
"""

from __future__ import annotations

"""Run the Phase 9 demonstrations with the locked neural weights only."""

import argparse
import dataclasses
import hashlib
import importlib
import json
import os
import sys
import time
import types
from pathlib import Path

import cv2
import numpy as np
import torch


ROOT = Path(__file__).resolve().parents[1]
MODELS = ROOT / "models"
FOOTAGE = ROOT / "assets" / "demo-footage"
BUILD = ROOT / "build" / "phase9-real"
XMEM_ROOT = ROOT / "build" / "phase9-third-party" / "XMem-ec776eac6feaf59b4860678c3363ce7634e19c5e"
MIDAS_ROOT = ROOT / "build" / "phase9-third-party" / "MiDaS-1645b7e1675301fdfac03640738fe5a6531e17d6"


class PipelineError(RuntimeError):
    """Raised when the real-weight pipeline cannot satisfy its contract."""


def sha256(path: Path) -> str:
    value = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            value.update(block)
    return value.hexdigest()


def load_locked_models() -> dict[str, Path]:
    lock = json.loads((ROOT / "docs" / "model-lock.json").read_text(encoding="utf-8"))
    result: dict[str, Path] = {}
    failures: list[str] = []
    for record in lock["models"]:
        path = MODELS / record["filename"]
        if not path.is_file():
            failures.append(f"{record['id']}: missing {path}")
            continue
        actual_size = path.stat().st_size
        actual_sha = sha256(path)
        if actual_size != record["bytes"]:
            failures.append(f"{record['id']}: bytes expected={record['bytes']} actual={actual_size}")
        if actual_sha.lower() != record["sha256"].lower():
            failures.append(f"{record['id']}: sha256 expected={record['sha256']} actual={actual_sha}")
        else:
            result[record["id"]] = path
    if failures:
        raise PipelineError("locked model verification failed: " + "; ".join(failures))
    return result


def require_cuda() -> torch.device:
    if not torch.cuda.is_available():
        raise PipelineError("CUDA is required for Phase 9 real-weight runs; CPU fallback is forbidden")
    return torch.device("cuda:0")


def read_video(path: Path) -> tuple[list[np.ndarray], float, dict[str, object]]:
    capture = cv2.VideoCapture(str(path))
    if not capture.isOpened():
        raise PipelineError(f"cannot open input video: {path}")
    fps = float(capture.get(cv2.CAP_PROP_FPS))
    frames: list[np.ndarray] = []
    while True:
        ok, frame = capture.read()
        if not ok:
            break
        frames.append(cv2.cvtColor(frame, cv2.COLOR_BGR2RGB))
    capture.release()
    if not frames or fps <= 0:
        raise PipelineError(f"invalid input video: {path}")
    height, width = frames[0].shape[:2]
    return frames, fps, {"width": width, "height": height, "frames": len(frames)}


def write_video(path: Path, frames: list[np.ndarray], fps: float) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    height, width = frames[0].shape[:2]
    writer = cv2.VideoWriter(
        str(path), cv2.VideoWriter_fourcc(*"avc1"), fps, (width, height)
    )
    if not writer.isOpened():
        writer = cv2.VideoWriter(
            str(path), cv2.VideoWriter_fourcc(*"mp4v"), fps, (width, height)
        )
    if not writer.isOpened():
        raise PipelineError(f"cannot open output encoder: {path}")
    for frame in frames:
        writer.write(cv2.cvtColor(frame, cv2.COLOR_RGB2BGR))
    writer.release()


def write_comparison_video(
    path: Path,
    before: list[np.ndarray],
    after: list[np.ndarray],
    fps: float,
) -> None:
    if len(before) != len(after) or not before:
        raise PipelineError("before/after video lengths do not match")
    comparison: list[np.ndarray] = []
    for source, result in zip(before, after):
        if source.shape != result.shape:
            raise PipelineError("before/after frame shapes do not match")
        separator = np.full((source.shape[0], 6, 3), 255, dtype=np.uint8)
        frame = np.concatenate([source, separator, result], axis=1).copy()
        cv2.putText(frame, "BEFORE", (18, 34), cv2.FONT_HERSHEY_SIMPLEX, 0.85, (255, 255, 255), 3, cv2.LINE_AA)
        cv2.putText(frame, "BEFORE", (18, 34), cv2.FONT_HERSHEY_SIMPLEX, 0.85, (20, 20, 20), 1, cv2.LINE_AA)
        right_x = source.shape[1] + separator.shape[1] + 18
        cv2.putText(frame, "AFTER", (right_x, 34), cv2.FONT_HERSHEY_SIMPLEX, 0.85, (255, 255, 255), 3, cv2.LINE_AA)
        cv2.putText(frame, "AFTER", (right_x, 34), cv2.FONT_HERSHEY_SIMPLEX, 0.85, (20, 20, 20), 1, cv2.LINE_AA)
        comparison.append(frame)
    write_video(path, comparison, fps)


def resize_frames(frames: list[np.ndarray], height: int) -> tuple[list[np.ndarray], float]:
    source_height, source_width = frames[0].shape[:2]
    if source_height <= height:
        return frames, 1.0
    ratio = height / source_height
    width = int(round(source_width * ratio))
    width -= width % 16
    target_height = int(round(source_height * ratio))
    target_height -= target_height % 16
    return [cv2.resize(frame, (width, target_height), interpolation=cv2.INTER_AREA) for frame in frames], ratio


def sam_first_mask(frame: np.ndarray, checkpoint: Path, point: tuple[int, int], device: torch.device) -> np.ndarray:
    from segment_anything import SamPredictor, sam_model_registry

    model = sam_model_registry["vit_b"](checkpoint=str(checkpoint))
    model.to(device=device)
    model.eval()
    predictor = SamPredictor(model)
    predictor.set_image(frame)
    point_coords = np.asarray([point], dtype=np.float32)
    point_labels = np.asarray([1], dtype=np.int32)
    masks, scores, _ = predictor.predict(
        point_coords=point_coords,
        point_labels=point_labels,
        multimask_output=True,
    )
    selected = int(np.argmax(scores))
    if float(scores[selected]) < 0.5 or masks[selected].sum() == 0:
        raise PipelineError(f"SAM failed to produce a usable first-frame mask; scores={scores.tolist()}")
    del predictor, model
    torch.cuda.empty_cache()
    return masks[selected].astype(np.float32)


def xmem_masks(
    frames: list[np.ndarray],
    seed_mask: np.ndarray,
    seed_index: int,
    checkpoint: Path,
    device: torch.device,
) -> list[np.ndarray]:
    if not XMEM_ROOT.is_dir():
        raise PipelineError(f"XMem source is missing: {XMEM_ROOT}")
    sys.path.insert(0, str(XMEM_ROOT))
    from model.network import XMem
    from inference.inference_core import InferenceCore

    config = {
        "single_object": False,
        "max_mid_term_frames": 10,
        "min_mid_term_frames": 5,
        "max_long_term_elements": 10000,
        "num_prototypes": 128,
        "top_k": 30,
        "mem_every": 5,
        "deep_update_every": -1,
        "enable_long_term": True,
        "enable_long_term_count_usage": True,
    }
    if not 0 <= seed_index < len(frames):
        raise PipelineError(f"XMem seed frame is outside the video: {seed_index}")

    network = XMem(config, str(checkpoint)).to(device).eval()
    processor = InferenceCore(network, config=config)
    processor.set_all_labels([1])
    # No target exists before the manually verified seed frame. XMem is started
    # at that frame so its recurrent state is not poisoned by background masks.
    output: list[np.ndarray] = [np.zeros(frames[0].shape[:2], dtype=np.float32) for _ in range(seed_index)]
    for index, frame in enumerate(frames[seed_index:], start=seed_index):
        image = torch.from_numpy(frame).to(device=device).permute(2, 0, 1).float().div(255.0).unsqueeze(0)
        image = image.contiguous()
        mask = torch.from_numpy(seed_mask).to(device=device).unsqueeze(0) if index == seed_index else None
        with torch.inference_mode(), torch.autocast(device_type="cuda", dtype=torch.float16):
            predicted = processor.step(image, mask=mask, end=index == len(frames) - 1)
        output.append((predicted[0].float().cpu().numpy() > 0.5).astype(np.float32))
        if index % 20 == 0:
            print(f"XMem frame {index + 1}/{len(frames)}")
    del processor, network
    torch.cuda.empty_cache()
    return output


def load_propainter(checkpoint: Path, device: torch.device) -> torch.nn.Module:
    from pytorchcv.models.propainter import propainter

    source = torch.load(checkpoint, map_location="cpu", weights_only=True)
    model = propainter()
    target = model.state_dict()
    mapped: dict[str, torch.Tensor] = {}
    for key in target:
        source_key = key
        if key.startswith("encoder.layers.") and ".conv." in key:
            index, suffix = key[len("encoder.layers."):].split(".conv.", 1)
            source_key = f"encoder.layers.{int(index) * 2}.{suffix}"
        elif key.startswith("decoder.unit"):
            unit, suffix = key.split(".", 2)[1:]
            unit_index = int(unit[-1])
            conv_index = int(suffix.split(".", 1)[0][-1])
            parameter = suffix.split(".conv.", 1)[1]
            source_index = (unit_index - 1) * 4 + (conv_index - 1) * 2
            source_key = f"decoder.{source_index}.{'conv.' if conv_index == 1 else ''}{parameter}"
        elif key.startswith("feat_prop_module.deform_align."):
            if ".deform_conv." in key:
                source_key = key.replace(".deform_conv.", ".")
            else:
                parts = key.split(".")
                conv_number = int(parts[4].replace("conv", ""))
                source_key = ".".join(parts[:4] + [str((conv_number - 1) * 2), parts[-1]])
        elif key.startswith("feat_prop_module.backbone.") or key.startswith("feat_prop_module.fuse."):
            parts = key.split(".")
            if key.startswith("feat_prop_module.backbone."):
                source_key = ".".join(parts[:3] + [str((int(parts[3].replace("conv", "")) - 1) * 2), parts[-1]])
            else:
                source_key = ".".join(parts[:2] + [str((int(parts[2].replace("conv", "")) - 1) * 2), parts[-1]])
        if source_key not in source:
            raise PipelineError(f"ProPainter checkpoint mapping missing source key for {key}: {source_key}")
        if tuple(source[source_key].shape) != tuple(target[key].shape):
            raise PipelineError(f"ProPainter shape mismatch {key}: {tuple(target[key].shape)} vs {source_key}: {tuple(source[source_key].shape)}")
        mapped[key] = source[source_key]
    model.load_state_dict(mapped, strict=True)
    model.to(device=device).eval()
    for parameter in model.parameters():
        parameter.requires_grad_(False)
    return model


def propainter_inpaint(
    frames: list[np.ndarray],
    masks: list[np.ndarray],
    checkpoint: Path,
    raft_checkpoint: Path,
    rfc_checkpoint: Path,
    device: torch.device,
) -> list[np.ndarray]:
    from pytorchcv.models.propainter_stream import ProPainterIterator, TensorSequencer

    model = load_propainter(checkpoint, device)
    scale = 0.5
    raw_frames = np.asarray(frames, dtype=np.uint8)
    raw_masks = np.asarray([(mask > 0.5).astype(np.uint8) for mask in masks])
    frame_seq = TensorSequencer(
        data=torch.from_numpy(raw_frames).permute(0, 3, 1, 2).float().div(127.5).sub(1.0).to(device)
    )
    mask_seq = TensorSequencer(data=torch.from_numpy(raw_masks)[:, None].float().to(device))
    iterator = ProPainterIterator(
        frames=frame_seq,
        masks=mask_seq,
        pp_model=model,
        use_cuda=True,
        raft_model=str(raft_checkpoint),
        pprfc_model=str(rfc_checkpoint),
        pp_window_size=min(20, len(frames) if len(frames) % 2 == 0 else len(frames) - 1),
        pp_stride=5,
        step=10,
    )
    restored_chunks: list[torch.Tensor] = []
    for index, chunk in enumerate(iterator):
        if chunk is None:
            raise PipelineError(f"ProPainter returned no output at iterator chunk {index}")
        restored_chunks.append(chunk.detach().float().cpu())
        print(f"ProPainter chunk {index + 1}: {len(chunk)} frames")
    if not restored_chunks:
        raise PipelineError("ProPainter returned no restored frames")
    restored = torch.cat(restored_chunks, dim=0)
    restored = restored.add(1.0).mul(127.5).clamp(0, 255).byte()
    output = [frame.permute(1, 2, 0).numpy() for frame in restored]
    del iterator, model, restored_chunks, restored
    torch.cuda.empty_cache()
    return output


def run_depth(frame: np.ndarray, checkpoint: Path, device: torch.device) -> np.ndarray:
    if not MIDAS_ROOT.is_dir():
        raise PipelineError(f"MiDaS source is missing: {MIDAS_ROOT}")
    sys.path.insert(0, str(MIDAS_ROOT))
    for module_name in list(sys.modules):
        if module_name == "midas" or module_name.startswith("midas."):
            sys.modules.pop(module_name, None)
    package = types.ModuleType("midas")
    package.__path__ = [str(MIDAS_ROOT / "midas")]
    package.__package__ = "midas"
    sys.modules["midas"] = package
    # MiDaS pins timm 0.6.12; Python 3.13 tightened dataclass mutable-default checks
    # in an otherwise unused timm model declaration.
    original_get_field = dataclasses._get_field
    def compat_get_field(cls, field_name, default, kw_only):
        try:
            return original_get_field(cls, field_name, default, kw_only)
        except ValueError as exc:
            if "mutable default" not in str(exc):
                raise
            current = getattr(cls, field_name)
            setattr(cls, field_name, dataclasses.field(default_factory=lambda value=current: value))
            try:
                return original_get_field(cls, field_name, default, kw_only)
            finally:
                setattr(cls, field_name, current)
    dataclasses._get_field = compat_get_field
    hubconf = importlib.util.spec_from_file_location("physfx_midas_hubconf", MIDAS_ROOT / "hubconf.py")
    if hubconf is None or hubconf.loader is None:
        raise PipelineError("cannot load locked MiDaS hubconf")
    module = importlib.util.module_from_spec(hubconf)
    hubconf.loader.exec_module(module)
    dataclasses._get_field = original_get_field
    model = module.DPT_SwinV2_T_256(pretrained=False).to(device).eval()
    model.load_state_dict(torch.load(checkpoint, map_location="cpu", weights_only=True), strict=True)
    transform = module.transforms().swin256_transform
    tensor = transform(frame).to(device=device)
    with torch.inference_mode(), torch.autocast(device_type="cuda", dtype=torch.float16):
        depth = model(tensor)
    depth = torch.nn.functional.interpolate(depth[:, None], size=frame.shape[:2], mode="bicubic", align_corners=False)[0, 0]
    depth = depth.float().cpu().numpy()
    depth -= float(depth.min())
    depth /= max(float(depth.max()), 1e-6)
    del model
    torch.cuda.empty_cache()
    return depth


def feather(mask: np.ndarray, radius: int = 7) -> np.ndarray:
    blurred = cv2.GaussianBlur(mask.astype(np.float32), (0, 0), radius)
    return np.clip(blurred, 0.0, 1.0)


def make_move(frames: list[np.ndarray], masks: list[np.ndarray], repaired: list[np.ndarray], depth: np.ndarray) -> list[np.ndarray]:
    shift_x, shift_y = -90, -20
    output: list[np.ndarray] = []
    for frame, mask, background in zip(frames, masks, repaired):
        alpha = feather(mask, 5)
        matrix = np.float32([[1, 0, shift_x], [0, 1, shift_y]])
        moved = cv2.warpAffine(frame, matrix, (frame.shape[1], frame.shape[0]), borderMode=cv2.BORDER_CONSTANT)
        moved_alpha = cv2.warpAffine(alpha, matrix, (frame.shape[1], frame.shape[0]), borderMode=cv2.BORDER_CONSTANT)
        result = background.astype(np.float32)
        result = result * (1 - moved_alpha[..., None]) + moved.astype(np.float32) * moved_alpha[..., None]
        output.append(np.clip(result, 0, 255).astype(np.uint8))
    return output


def make_smoke(frames: list[np.ndarray], masks: list[np.ndarray], depth: np.ndarray) -> list[np.ndarray]:
    height, width = frames[0].shape[:2]
    yy, xx = np.mgrid[0:height, 0:width]
    # The plume is attached to the central tree trunk; the tracked dog masks it
    # when crossing in front, rather than treating the animal as part of smoke.
    center_x, center_y = int(width * 0.50), int(height * 0.55)
    base = np.exp(-(((xx - center_x) / (width * 0.08)) ** 2 + ((yy - center_y) / (height * 0.22)) ** 2))
    outputs: list[np.ndarray] = []
    for index, (frame, mask) in enumerate(zip(frames, masks)):
        wobble = np.sin(index * 0.22) * width * 0.018
        plume = np.exp(-(((xx - center_x - wobble) / (width * 0.09)) ** 2 + ((yy - center_y + index * height * 0.0008) / (height * 0.25)) ** 2))
        opacity = np.clip((0.30 * base + 0.42 * plume) * (0.75 + 0.25 * depth), 0, 0.62)
        opacity *= (1.0 - feather(mask, 3))
        smoke = np.zeros_like(frame, dtype=np.float32)
        smoke[..., :] = np.asarray([226, 232, 236], dtype=np.float32)
        result = frame.astype(np.float32) * (1 - opacity[..., None]) + smoke * opacity[..., None]
        outputs.append(np.clip(result, 0, 255).astype(np.uint8))
    return outputs


def process_case(
    name: str,
    input_path: Path,
    point: tuple[int, int],
    seed_frame: int,
    mode: str,
    models: dict[str, Path],
    device: torch.device,
) -> dict[str, object]:
    started = time.perf_counter()
    frames, fps, input_info = read_video(input_path)
    inference_frames, ratio = resize_frames(frames, 360)
    inference_point = (int(point[0] * ratio), int(point[1] * ratio))
    first_mask = sam_first_mask(inference_frames[seed_frame], models["sam-vit-base"], inference_point, device)
    masks_small = xmem_masks(inference_frames, first_mask, seed_frame, models["xmem"], device)
    masks = [cv2.resize(mask, (frames[0].shape[1], frames[0].shape[0]), interpolation=cv2.INTER_LINEAR) for mask in masks_small]
    depth_small = run_depth(inference_frames[0], models["midas-dpt-swin2-tiny"], device)
    depth = cv2.resize(depth_small, (frames[0].shape[1], frames[0].shape[0]), interpolation=cv2.INTER_CUBIC)
    if mode in {"remove", "move"}:
        repaired_small = propainter_inpaint(
            inference_frames,
            masks_small,
            models["propainter"],
            models["propainter-raft-things"],
            models["propainter-rfc"],
            device,
        )
        repaired = [cv2.resize(frame, (frames[0].shape[1], frames[0].shape[0]), interpolation=cv2.INTER_CUBIC) for frame in repaired_small]
    else:
        repaired = frames
    if mode == "remove":
        output = repaired
    elif mode == "move":
        output = make_move(frames, masks, repaired, depth)
    elif mode == "smoke":
        output = make_smoke(frames, masks, depth)
    else:
        raise PipelineError(f"unknown mode: {mode}")
    output_path = BUILD / f"{name}-before-after.mp4"
    write_comparison_video(output_path, frames, output, fps)
    ious = []
    for previous, current in zip(masks, masks[1:]):
        previous_binary = previous > 0.5
        current_binary = current > 0.5
        union = np.logical_or(previous_binary, current_binary).sum()
        ious.append(float(np.logical_and(previous_binary, current_binary).sum() / union) if union else 1.0)
    if not ious:
        raise PipelineError(f"{name}: no adjacent mask pairs were produced")
    min_iou = min(ious)
    if min_iou <= 0.7:
        raise PipelineError(f"{name}: adjacent mask IoU gate failed: min={min_iou:.4f} <= 0.7")
    return {
        "name": name,
        "mode": mode,
        "input_path": str(input_path),
        "output": str(output_path),
        "fps": fps,
        "input_media": input_info,
        "inference_height": inference_frames[0].shape[0],
        "inference_width": inference_frames[0].shape[1],
        "xmem_seed_frame": seed_frame,
        "mask_iou_adjacent_min": min_iou,
        "mask_iou_adjacent_p50": float(np.percentile(ious, 50)),
        "mask_iou_gate": "passed (min > 0.7)",
        "repair_quality": "requires maintainer visual review for trails/flicker",
        "occlusion_quality": "requires maintainer visual review",
        "wall_seconds": time.perf_counter() - started,
        "weights": {key: str(value) for key, value in models.items()},
    }


def main() -> int:
    parser = argparse.ArgumentParser(description="Run Phase 9 real-weight demonstrations")
    parser.add_argument("--case", choices=("person", "vehicle", "pet", "all"), default="all")
    args = parser.parse_args()
    try:
        device = require_cuda()
        models = load_locked_models()
        cases = {
            "person": ("人物", (470, 300), 0, "remove"),
            "vehicle": ("车辆", (500, 350), 0, "move"),
            "pet": ("宠物", (790, 450), 72, "smoke"),
        }
        selected = list(cases) if args.case == "all" else [args.case]
        records = []
        for case in selected:
            name, point, seed_frame, mode = cases[case]
            records.append(process_case(case, FOOTAGE / f"{name}.mp4", point, seed_frame, mode, models, device))
        BUILD.mkdir(parents=True, exist_ok=True)
        (BUILD / "metrics.json").write_text(json.dumps(records, ensure_ascii=False, indent=2), encoding="utf-8")
        print(json.dumps(records, ensure_ascii=False, indent=2))
        return 0
    except Exception as exc:
        print(f"[FAIL] {type(exc).__name__}: {exc}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
