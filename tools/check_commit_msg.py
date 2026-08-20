#!/usr/bin/env python3
"""
 * PhysFX Engine —— 视频世界编辑器（Video World Editor）
 * Copyright (c) 2026 向伟典
 *
 * 本文件遵循 Apache License 2.0 开源协议，许可证全文见仓库根目录 LICENSE 文件。
 * 按 Apache-2.0 要求，修改本文件的衍生版本须在文件中保留显著修改声明。
 *
 * SPDX-License-Identifier: Apache-2.0
"""

from __future__ import annotations

import re
import subprocess
import sys
from pathlib import Path

TYPES = {"feat", "fix", "docs", "refactor", "test", "perf", "build", "ci", "chore"}
OWNER = "Signed-off-by: 向伟典 <xwd752438081@163.com>"
HEADER = re.compile(r"^(?P<type>[a-z]+)(?:\((?P<scope>[a-z0-9][a-z0-9_-]*)\))?: (?P<subject>[^.。\r\n].*)$")


def validate(message: str) -> list[str]:
    lines = message.splitlines()
    if not lines or not lines[0].strip():
        return ["提交信息首行不能为空"]
    first = lines[0].strip()
    match = HEADER.match(first)
    errors: list[str] = []
    if not match:
        errors.append("首行必须符合 <type>(<scope>): <中文描述> 格式，描述末尾不能有句号")
    else:
        if match.group("type") not in TYPES:
            errors.append(f"type 不在允许列表中: {match.group('type')}")
        if len(first) > 50:
            errors.append("提交首行不得超过 50 个字符")
        subject = match.group("subject").strip()
        if not subject:
            errors.append("提交描述不能为空")
        elif subject.endswith((".", "。")):
            errors.append("提交描述末尾不能有句号")
    if OWNER not in message:
        errors.append(f"必须包含 DCO 签署行: {OWNER}")
    return errors


def main() -> int:
    if len(sys.argv) == 3 and sys.argv[1] == "--range":
        try:
            output = subprocess.check_output(
                ["git", "log", "--format=%B%x00", sys.argv[2]], text=True, encoding="utf-8"
            )
        except (OSError, subprocess.CalledProcessError) as exc:
            print(f"无法读取提交范围: {exc}", file=sys.stderr)
            return 2
        failed = False
        for index, message in enumerate(output.split("\x00"), start=1):
            if not message.strip():
                continue
            errors = validate(message.strip())
            if errors:
                failed = True
                print(f"范围内第 {index} 条提交不合规:", file=sys.stderr)
                for error in errors:
                    print(f"- {error}", file=sys.stderr)
        if failed:
            return 1
        print("提交范围校验通过")
        return 0
    if len(sys.argv) != 2:
        print("用法: check_commit_msg.py <提交信息文件> | --range <base..head>", file=sys.stderr)
        return 2
    path = Path(sys.argv[1])
    try:
        message = path.read_text(encoding="utf-8")
    except OSError as exc:
        print(f"无法读取提交信息: {exc}", file=sys.stderr)
        return 2
    errors = validate(message)
    if errors:
        print("提交信息校验失败:", file=sys.stderr)
        for error in errors:
            print(f"- {error}", file=sys.stderr)
        return 1
    print("提交信息校验通过")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
