#!/usr/bin/env python3
# PhysFX Engine —— 视频世界编辑器（Video World Editor）
# Copyright (c) 2026 向伟典
#
# 本文件遵循 Apache License 2.0 开源协议，许可证全文见仓库根目录 LICENSE 文件。
# 按 Apache-2.0 要求，修改本文件的衍生版本须在文件中保留显著修改声明。
#
# SPDX-License-Identifier: Apache-2.0

"""检查工作区、主仓、标签和只读镜像是否完成同步。"""

from __future__ import annotations

import argparse
import json
import subprocess
import sys
from dataclasses import asdict, dataclass
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]


@dataclass(frozen=True)
class CheckResult:
    name: str
    ok: bool
    message: str


def git(*args: str, timeout: int = 30) -> subprocess.CompletedProcess[str]:
    return subprocess.run(
        ["git", *args],
        cwd=ROOT,
        capture_output=True,
        text=True,
        encoding="utf-8",
        errors="replace",
        check=False,
        timeout=timeout,
    )


def output(result: subprocess.CompletedProcess[str]) -> str:
    return result.stdout.strip()


def failure(result: subprocess.CompletedProcess[str]) -> str:
    return result.stderr.strip() or result.stdout.strip() or f"git 退出码 {result.returncode}"


def remote_ref(remote: str, ref: str) -> tuple[str, str]:
    result = git("ls-remote", remote, ref)
    if result.returncode != 0:
        return "", failure(result)
    for line in output(result).splitlines():
        fields = line.split()
        if len(fields) == 2 and fields[1] == ref:
            return fields[0], ""
    return "", f"远端 {remote} 不存在 {ref}"


def check_worktree() -> CheckResult:
    result = git("status", "--porcelain", "--untracked-files=all")
    if result.returncode != 0:
        return CheckResult("工作区", False, failure(result))
    changes = output(result).splitlines()
    if changes:
        return CheckResult("工作区", False, f"存在 {len(changes)} 项未提交改动")
    return CheckResult("工作区", True, "干净")


def check_head(origin: str, branch: str) -> tuple[CheckResult, str]:
    ref = f"refs/heads/{branch}"
    remote_head, error = remote_ref(origin, ref)
    if error:
        return CheckResult("主仓提交", False, error), ""
    head_result = git("rev-parse", "HEAD")
    if head_result.returncode != 0:
        return CheckResult("主仓提交", False, failure(head_result)), remote_head
    head = output(head_result)
    if head == remote_head:
        return CheckResult("主仓提交", True, f"HEAD={head[:12]} 与 {origin}/{branch} 一致"), remote_head

    # ls-remote only exposes the tip. Fetch the object without updating a local
    # branch so ancestry can still be proven when the remote is ahead.
    fetch = git("fetch", "--quiet", "--no-tags", origin, ref, timeout=120)
    if fetch.returncode != 0:
        return CheckResult("主仓提交", False, f"无法取回远端提交: {failure(fetch)}"), remote_head
    ancestor = git("merge-base", "--is-ancestor", head, remote_head)
    if ancestor.returncode == 0:
        return CheckResult(
            "主仓提交", True, f"HEAD={head[:12]} 已包含于 {origin}/{branch}={remote_head[:12]}"
        ), remote_head
    return CheckResult(
        "主仓提交", False, f"HEAD={head[:12]} 尚未推送到 {origin}/{branch}={remote_head[:12]}"
    ), remote_head


def local_tags() -> tuple[dict[str, str], str]:
    result = git("for-each-ref", "--format=%(refname:strip=2) %(objectname)", "refs/tags")
    if result.returncode != 0:
        return {}, failure(result)
    tags: dict[str, str] = {}
    for line in output(result).splitlines():
        name, separator, object_id = line.partition(" ")
        if separator:
            tags[name] = object_id
    return tags, ""


def remote_tags(remote: str) -> tuple[dict[str, str], str]:
    result = git("ls-remote", "--tags", remote, "refs/tags/*")
    if result.returncode != 0:
        return {}, failure(result)
    tags: dict[str, str] = {}
    for line in output(result).splitlines():
        fields = line.split()
        if len(fields) != 2 or fields[1].endswith("^{}"):
            continue
        tags[fields[1].removeprefix("refs/tags/")] = fields[0]
    return tags, ""


def check_tags(origin: str) -> CheckResult:
    local, error = local_tags()
    if error:
        return CheckResult("标签", False, error)
    remote, error = remote_tags(origin)
    if error:
        return CheckResult("标签", False, f"读取 {origin} 标签失败: {error}")
    missing = sorted(name for name, object_id in local.items() if remote.get(name) != object_id)
    if missing:
        return CheckResult("标签", False, f"{origin} 缺少或不一致: {', '.join(missing)}")
    return CheckResult("标签", True, f"本地 {len(local)} 个标签均存在于 {origin}")


def check_mirror(mirror: str, branch: str, origin_head: str) -> CheckResult:
    if not origin_head:
        return CheckResult("镜像", False, "主仓提交未知，无法比较镜像")
    mirror_head, error = remote_ref(mirror, f"refs/heads/{branch}")
    if error:
        return CheckResult("镜像", False, error)
    if mirror_head != origin_head:
        return CheckResult(
            "镜像", False, f"{mirror}/{branch}={mirror_head[:12]}，主仓={origin_head[:12]}"
        )
    return CheckResult("镜像", True, f"{mirror}/{branch} 与主仓同为 {origin_head[:12]}")


def evaluate(origin: str, mirror: str, branch: str) -> list[CheckResult]:
    worktree = check_worktree()
    head, origin_head = check_head(origin, branch)
    return [worktree, head, check_tags(origin), check_mirror(mirror, branch, origin_head)]


def main() -> int:
    if hasattr(sys.stdout, "reconfigure"):
        sys.stdout.reconfigure(encoding="utf-8")
    parser = argparse.ArgumentParser(description="PhysFX 远端同步检查")
    parser.add_argument("--origin", default="origin", help="Gitee 主仓 remote 名")
    parser.add_argument("--mirror", default="github", help="GitHub 镜像 remote 名")
    parser.add_argument("--branch", default="master", help="需要核对的主分支")
    parser.add_argument("--json", action="store_true", help="输出机器可读 JSON")
    args = parser.parse_args()
    try:
        results = evaluate(args.origin, args.mirror, args.branch)
    except (OSError, subprocess.TimeoutExpired) as exc:
        results = [CheckResult("执行", False, f"检查异常: {exc}")]

    all_green = all(item.ok for item in results)
    if args.json:
        print(json.dumps(
            {"checks": [asdict(item) for item in results], "all_green": all_green},
            ensure_ascii=False,
            indent=2,
        ))
    else:
        for item in results:
            print(f"{'✅' if item.ok else '❌'} {item.name}: {item.message}")
    return 0 if all_green else 1


if __name__ == "__main__":
    raise SystemExit(main())
