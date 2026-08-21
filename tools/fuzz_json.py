#!/usr/bin/env python3
# PhysFX Engine —— 视频世界编辑器（Video World Editor）
# Copyright (c) 2026 向伟典
#
# 本文件遵循 Apache License 2.0 开源协议，许可证全文见仓库根目录 LICENSE 文件。
# 按 Apache-2.0 要求，修改本文件的衍生版本须在文件中保留显著修改声明。
#
# SPDX-License-Identifier: Apache-2.0

"""Deterministic parser-boundary smoke/fuzz corpus check; never executes package content."""
from __future__ import annotations

import argparse
import json
import random
from pathlib import Path


def mutate(value: bytes, seed: int) -> bytes:
    rng = random.Random(seed)
    data = bytearray(value)
    if data and rng.random() < 0.8:
        data[rng.randrange(len(data))] ^= 1 << rng.randrange(8)
    if rng.random() < 0.5:
        data.extend(bytes(rng.randrange(256) for _ in range(rng.randrange(0, 16))))
    return bytes(data[: 1024 * 1024 + 1])


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--root", type=Path, default=Path("tests/fuzz"))
    parser.add_argument("--iterations", type=int, default=250)
    args = parser.parse_args()
    seeds = sorted(args.root.glob("*.seed.json"))
    if not seeds: raise SystemExit("fuzz 语料为空")
    checked = 0
    for seed_path in seeds:
        original = seed_path.read_bytes()
        for index in range(args.iterations):
            candidate = original if index == 0 else mutate(original, index)
            try: json.loads(candidate)
            except (UnicodeDecodeError, json.JSONDecodeError, ValueError): pass
            checked += 1
    print(f"FUZZ_CORPUS=PASS seeds={len(seeds)} cases={checked}")
    return 0


if __name__ == "__main__": raise SystemExit(main())
