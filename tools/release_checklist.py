#!/usr/bin/env python3
# PhysFX Engine —— 视频世界编辑器（Video World Editor）
# Copyright (c) 2026 向伟典
#
# 本文件遵循 Apache License 2.0 开源协议，许可证全文见仓库根目录 LICENSE 文件。
# 按 Apache-2.0 要求，修改本文件的衍生版本须在文件中保留显著修改声明。
#
# SPDX-License-Identifier: Apache-2.0

"""Phase release gate; reports evidence without mutating Git or remote services."""

from __future__ import annotations

import argparse
import json
import re
import subprocess
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]


def run(*args: str) -> str:
    result = subprocess.run(args, cwd=ROOT, capture_output=True, text=True, check=False)
    return result.stdout.strip()


def run_status(*args: str) -> bool:
    return subprocess.run(
        args, cwd=ROOT, capture_output=True, text=True, check=False
    ).returncode == 0


def check_sync() -> tuple[bool, str]:
    result = subprocess.run(
        [sys.executable, str(ROOT / "tools" / "sync_check.py"), "--json"],
        cwd=ROOT,
        capture_output=True,
        text=True,
        encoding="utf-8",
        errors="replace",
        check=False,
    )
    try:
        payload = json.loads(result.stdout)
        failures = [item["message"] for item in payload["checks"] if not item["ok"]]
    except (json.JSONDecodeError, KeyError, TypeError):
        return False, result.stderr.strip() or "sync_check.py 未返回有效 JSON"
    if result.returncode != 0 or not payload.get("all_green", False):
        return False, "；".join(failures) or "远端同步检查失败"
    return True, "工作区、Gitee、标签和 GitHub 镜像均已同步"


def check_versions() -> tuple[bool, str]:
    cmake = (ROOT / "CMakeLists.txt").read_text(encoding="utf-8")
    package = (ROOT / "bindings/python/physfx_py/__init__.py").read_text(encoding="utf-8")
    readme = (ROOT / "README.md").read_text(encoding="utf-8")
    values = [re.search(r"project\(PhysFX VERSION ([^) ]+)", cmake),
              re.search(r'__version__\s*=\s*["\']([^"\']+)', package),
              re.search(r"当前版本[：:]\s*`?v(0\.\d+\.\d+(?:-[\w.]+)?)", readme)]
    versions = [match.group(1) if match else "" for match in values]
    ok = len(set(versions)) == 1 and bool(versions[0])
    return ok, f"版本: {', '.join(versions)}"


def check_changelog() -> tuple[bool, str]:
    text = (ROOT / "CHANGELOG.md").read_text(encoding="utf-8")
    ok = "## [Unreleased]" not in text
    return ok, "CHANGELOG 已有明确版本段" if ok else "CHANGELOG 仍有 Unreleased 段"


def check_tag() -> tuple[bool, str]:
    cmake = (ROOT / "CMakeLists.txt").read_text(encoding="utf-8")
    match = re.search(r"project\(PhysFX VERSION ([^) ]+)", cmake)
    expected = f"v{match.group(1)}" if match else ""
    if not expected or expected not in run("git", "tag", "--list", expected).splitlines():
        return False, f"缺少版本标签 {expected or '未知'}"
    if run("git", "cat-file", "-t", expected) != "tag":
        return False, f"{expected} 不是附注标签"
    if not run_status("git", "merge-base", "--is-ancestor", expected, "HEAD"):
        return False, f"{expected} 不在当前 HEAD 的发布历史中"
    return True, f"附注标签 {expected} 位于当前发布历史中"


def check_ci() -> tuple[bool, str]:
    report = ROOT / "docs/phase6-completion-report.md"
    report_text = report.read_text(encoding="utf-8") if report.exists() else ""
    match = re.search(r"^CI_RUN_URL=(https://\S+)$", report_text, re.MULTILINE)
    ok = bool(match) and "placeholder" not in match.group(1).lower()
    return ok, f"CI URL: {match.group(1)}" if ok else "缺少真实 CI 运行 URL（仅有 workflow 不算证据）"


def check_release() -> tuple[bool, str]:
    report = ROOT / "docs/phase6-completion-report.md"
    text = report.read_text(encoding="utf-8") if report.exists() else ""
    release = re.search(r"^RELEASE_URL=(https://\S+)$", text, re.MULTILINE)
    material = re.search(r"^MATERIAL_STATUS=(ready|pending)$", text, re.MULTILINE)
    ok = bool(release and material)
    if not ok:
        return False, "缺少真实 Release URL 或 MATERIAL_STATUS=ready|pending"
    state = "物料齐全" if material.group(1) == "ready" else "物料补充中（允许基础 Release 先行）"
    return True, f"Release: {release.group(1)}；{state}"


def check_roadmap() -> tuple[bool, str]:
    text = (ROOT / "docs/roadmap.md").read_text(encoding="utf-8")
    ok = "Phase 7" in text and "Phase 6" in text
    return ok, "路线图包含 Phase 6 状态和 Phase 7 粗规划" if ok else "路线图缺少下一阶段规划"


def check_degradation() -> tuple[bool, str]:
    report = ROOT / "docs/phase6-completion-report.md"
    text = report.read_text(encoding="utf-8") if report.exists() else ""
    ok = "降级" in text and "真实" in text
    return ok, "完成报告包含真实能力与降级清单" if ok else "缺少完成报告中的降级清单"


CHECKS = [
    ("remote_sync", check_sync),
    ("versions", check_versions),
    ("changelog", check_changelog),
    ("annotated_tag", check_tag),
    ("release_evidence", check_release),
    ("ci_evidence", check_ci),
    ("roadmap", check_roadmap),
    ("degradation_report", check_degradation),
]


def main() -> int:
    if hasattr(sys.stdout, "reconfigure"):
        sys.stdout.reconfigure(encoding="utf-8")
    parser = argparse.ArgumentParser(description="PhysFX 阶段发布收尾检查单")
    parser.add_argument("--json", action="store_true", help="输出机器可读 JSON")
    args = parser.parse_args()
    results = []
    for name, function in CHECKS:
        try:
            ok, message = function()
        except (OSError, ValueError) as exc:
            ok, message = False, f"检查异常: {exc}"
        results.append({"name": name, "ok": ok, "message": message})
    if args.json:
        print(json.dumps({"phase": 6, "checks": results, "all_green": all(item["ok"] for item in results)}, ensure_ascii=False, indent=2))
    else:
        for item in results:
            print(f"{'✅' if item['ok'] else '❌'} {item['name']}: {item['message']}")
    return 0 if all(item["ok"] for item in results) else 1


if __name__ == "__main__":
    raise SystemExit(main())
