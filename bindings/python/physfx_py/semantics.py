# PhysFX Engine —— 视频世界编辑器（Video World Editor）
# Copyright (c) 2026 向伟典
#
# 本文件遵循 Apache License 2.0 开源协议，许可证全文见仓库根目录 LICENSE 文件。
# 按 Apache-2.0 要求，修改本文件的衍生版本须在文件中保留显著修改声明。
#
# SPDX-License-Identifier: Apache-2.0

"""语义理解 Python 门面占位；真实绑定留待 Phase 3。"""

AVAILABLE = True


def available() -> bool:
    """返回语义模块是否已在 Python API 中注册。"""
    return AVAILABLE
