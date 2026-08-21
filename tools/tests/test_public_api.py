#!/usr/bin/env python3
# PhysFX Engine —— 视频世界编辑器（Video World Editor）
# Copyright (c) 2026 向伟典
# SPDX-License-Identifier: Apache-2.0

from tools import check_public_api


def test_frozen_public_api_boundary_is_clean() -> None:
    assert check_public_api.audit(check_public_api.ROOT) == []
