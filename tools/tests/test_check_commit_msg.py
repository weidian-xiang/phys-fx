"""
 * PhysFX Engine —— 视频世界编辑器（Video World Editor）
 * Copyright (c) 2026 向伟典
 *
 * 本文件遵循 Apache License 2.0 开源协议，许可证全文见仓库根目录 LICENSE 文件。
 * 按 Apache-2.0 要求，修改本文件的衍生版本须在文件中保留显著修改声明。
 *
 * SPDX-License-Identifier: Apache-2.0
"""

from tools.check_commit_msg import is_merge_commit, validate


def test_valid_commit_message() -> None:
    message = """feat(platform): 实现视频帧读取\n\n补充统一时序元数据。\n\nSigned-off-by: 向伟典 <xwd752438081@163.com>\n"""
    assert validate(message) == []


def test_invalid_type_and_missing_dco() -> None:
    errors = validate("bad(core): 错误提交")
    assert any("type" in error for error in errors)
    assert any("DCO" in error for error in errors)


def test_period_is_rejected() -> None:
    errors = validate("docs: 描述不能以句号结束。\nSigned-off-by: 向伟典 <xwd752438081@163.com>")
    assert errors


def test_subject_over_50_characters_is_rejected() -> None:
    message = "feat(core): " + "很长的描述" * 10 + "\nSigned-off-by: 向伟典 <xwd752438081@163.com>"
    assert any("50" in error for error in validate(message))


def test_platform_merge_commit_is_detected() -> None:
    assert is_merge_commit("1111111 2222222")
    assert not is_merge_commit("1111111")
