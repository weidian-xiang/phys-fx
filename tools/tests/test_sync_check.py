#!/usr/bin/env python3
# PhysFX Engine —— 视频世界编辑器（Video World Editor）
# Copyright (c) 2026 向伟典
#
# 本文件遵循 Apache License 2.0 开源协议，许可证全文见仓库根目录 LICENSE 文件。
# 按 Apache-2.0 要求，修改本文件的衍生版本须在文件中保留显著修改声明。
#
# SPDX-License-Identifier: Apache-2.0

import subprocess

from tools import sync_check


def completed(stdout: str = "", returncode: int = 0, stderr: str = ""):
    return subprocess.CompletedProcess(["git"], returncode, stdout, stderr)


def test_remote_tags_ignore_peeled_annotated_refs(monkeypatch):
    monkeypatch.setattr(
        sync_check,
        "git",
        lambda *args, **kwargs: completed(
            "abc refs/tags/v0.5.0\ndef refs/tags/v0.5.0^{}\n123 refs/tags/v0.4.0\n"
        ),
    )
    tags, error = sync_check.remote_tags("origin")
    assert not error
    assert tags == {"v0.5.0": "abc", "v0.4.0": "123"}


def test_dirty_worktree_fails(monkeypatch):
    monkeypatch.setattr(sync_check, "git", lambda *args, **kwargs: completed(" M README.md\n"))
    result = sync_check.check_worktree()
    assert not result.ok
    assert "1 项" in result.message


def test_mirror_requires_same_commit(monkeypatch):
    monkeypatch.setattr(sync_check, "remote_ref", lambda remote, ref: ("b" * 40, ""))
    result = sync_check.check_mirror("github", "master", "a" * 40)
    assert not result.ok
    assert "github/master" in result.message


def test_tags_must_exist_on_origin_and_mirror(monkeypatch):
    monkeypatch.setattr(sync_check, "local_tags", lambda: ({"v1.0.0": "abc"}, ""))
    monkeypatch.setattr(
        sync_check,
        "remote_tags",
        lambda remote: ({"v1.0.0": "abc"}, "") if remote == "origin" else ({}, ""),
    )

    result = sync_check.check_tags("origin", "github")

    assert not result.ok
    assert "github" in result.message
