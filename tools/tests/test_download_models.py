#!/usr/bin/env python3
# PhysFX Engine —— 视频世界编辑器（Video World Editor）
# Copyright (c) 2026 向伟典
#
# 本文件遵循 Apache License 2.0 开源协议，许可证全文见仓库根目录 LICENSE 文件。
# 按 Apache-2.0 要求，修改本文件的衍生版本须在文件中保留显著修改声明。
#
# SPDX-License-Identifier: Apache-2.0

from tools import download_models


def test_sha256_requires_exact_hex_digest():
    assert download_models.valid_sha256("a" * 64)
    assert download_models.valid_sha256("A1" * 32)
    assert not download_models.valid_sha256("not-downloaded")
    assert not download_models.valid_sha256("a" * 63)


def test_incomplete_lock_record_is_rejected():
    record = {
        "id": "fixture",
        "filename": "fixture.pth",
        "url": "https://example.invalid/fixture.pth",
        "revision": "fixture@deadbeef",
        "sha256": "a" * 64,
        "bytes": 1,
        "format": "PyTorch checkpoint",
        "license": "MIT",
    }
    assert download_models.validate_record(record) == "缺少 tensor_contract"
