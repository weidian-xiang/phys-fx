# PhysFX Engine —— 视频世界编辑器（Video World Editor）
# Copyright (c) 2026 向伟典
#
# 本文件遵循 Apache License 2.0 开源协议，许可证全文见仓库根目录 LICENSE 文件。
# 按 Apache-2.0 要求，修改本文件的衍生版本须在文件中保留显著修改声明。
#
# SPDX-License-Identifier: Apache-2.0

import physfx_py


def test_version() -> None:
    """验证 Python 包骨架可以导入并返回版本号。"""
    assert physfx_py.version() == "0.2.0-dev"
    assert physfx_py.semantics.available()
    assert physfx_py.editing.available()
