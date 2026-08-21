# PhysFX Engine —— 视频世界编辑器（Video World Editor）
# Copyright (c) 2026 向伟典
#
# 本文件遵循 Apache License 2.0 开源协议，许可证全文见仓库根目录 LICENSE 文件。
# 按 Apache-2.0 要求，修改本文件的衍生版本须在文件中保留显著修改声明。
#
# SPDX-License-Identifier: Apache-2.0

"""PhysFX Engine 的 Python API Phase 2 门面。

pybind11 扩展可选；未编译原生扩展时，纯 Python 门面保证工具链和示例可运行。
"""

__version__ = "0.5.0"

try:
    from ._physfx import version as _native_version
except ImportError:
    _native_version = None


def version() -> str:
    """返回 PhysFX Engine 版本号。"""
    if _native_version is not None:
        return _native_version()
    return __version__


from . import editing, semantics  # noqa: E402  保持门面模块可发现
