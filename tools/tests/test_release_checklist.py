#!/usr/bin/env python3
# PhysFX Engine —— 视频世界编辑器（Video World Editor）
# Copyright (c) 2026 向伟典
#
# 本文件遵循 Apache License 2.0 开源协议，许可证全文见仓库根目录 LICENSE 文件。
# 按 Apache-2.0 要求，修改本文件的衍生版本须在文件中保留显著修改声明。
#
# SPDX-License-Identifier: Apache-2.0

import subprocess
import sys

from tools import release_checklist
from tools.sync_check import CheckResult


def tag_output(kind: str):
    def fake_run(*args: str) -> str:
        if args[1] == "tag":
            return "v0.5.0"
        if args[1] == "cat-file":
            return kind
        raise AssertionError(f"unexpected git command: {args}")

    return fake_run


def use_version(monkeypatch, tag: str = "v-test"):
    monkeypatch.setattr(release_checklist, "expected_tag", lambda: tag)


def test_annotated_tag_may_precede_release_evidence_commit(monkeypatch):
    use_version(monkeypatch, "v0.5.0")
    monkeypatch.setattr(release_checklist, "run", tag_output("tag"))
    monkeypatch.setattr(release_checklist, "run_status", lambda *args: True)

    ok, message = release_checklist.check_tag()

    assert ok
    assert "发布历史" in message


def test_lightweight_release_tag_is_rejected(monkeypatch):
    use_version(monkeypatch, "v0.5.0")
    monkeypatch.setattr(release_checklist, "run", tag_output("commit"))

    ok, message = release_checklist.check_tag()

    assert not ok
    assert "不是附注标签" in message


def test_unrelated_release_tag_is_rejected(monkeypatch):
    use_version(monkeypatch, "v0.5.0")
    monkeypatch.setattr(release_checklist, "run", tag_output("tag"))
    monkeypatch.setattr(release_checklist, "run_status", lambda *args: False)

    ok, message = release_checklist.check_tag()

    assert not ok
    assert "不在当前 HEAD" in message


def test_missing_expected_tag_is_rejected(monkeypatch):
    use_version(monkeypatch)
    monkeypatch.setattr(release_checklist, "run", lambda *args: "")

    ok, message = release_checklist.check_tag()

    assert not ok
    assert "缺少版本标签 v-test" in message


def test_unknown_project_version_is_rejected(monkeypatch):
    use_version(monkeypatch, "")

    ok, message = release_checklist.check_tag()

    assert not ok
    assert "缺少版本标签 未知" in message


def test_sync_check_reuses_sync_module_api(monkeypatch):
    monkeypatch.setattr(
        release_checklist.sync_check,
        "evaluate",
        lambda *args: [
            CheckResult("工作区", True, "干净"),
            CheckResult("镜像", True, "已同步"),
        ],
    )

    ok, message = release_checklist.check_sync()

    assert ok
    assert "均已同步" in message


def test_sync_check_aggregates_failures(monkeypatch):
    monkeypatch.setattr(
        release_checklist.sync_check,
        "evaluate",
        lambda *args: [CheckResult("镜像", False, "提交不一致")],
    )

    ok, message = release_checklist.check_sync()

    assert not ok
    assert message == "提交不一致"


def test_sync_check_reports_execution_error(monkeypatch):
    def fail(*args):
        raise OSError("network unavailable")

    monkeypatch.setattr(release_checklist.sync_check, "evaluate", fail)

    ok, message = release_checklist.check_sync()

    assert not ok
    assert "network unavailable" in message


def test_script_help_entrypoint_runs_from_repository_root():
    result = subprocess.run(
        [sys.executable, "tools/release_checklist.py", "--help"],
        cwd=release_checklist.ROOT,
        capture_output=True,
        text=True,
        encoding="utf-8",
        check=False,
    )

    assert result.returncode == 0
    assert "PhysFX 阶段发布收尾检查单" in result.stdout
