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
import sys
from pathlib import Path

try:
    from tools import sync_check
except ModuleNotFoundError:
    import sync_check


ROOT = Path(__file__).resolve().parents[1]


def completion_report() -> Path:
    """Return the current phase report, retaining Phase 6 compatibility."""
    phase8 = ROOT / "docs/phase8-completion-report.md"
    phase7 = ROOT / "docs/phase7-completion-report.md"
    if phase8.exists():
        return phase8
    return phase7 if phase7.exists() else ROOT / "docs/phase6-completion-report.md"


def current_phase() -> int:
    if (ROOT / "docs/phase8-completion-report.md").exists():
        return 8
    return 7 if (ROOT / "docs/phase7-completion-report.md").exists() else 6


def run(*args: str) -> str:
    if not args or args[0] != "git":
        raise ValueError("release_checklist.run 只允许调用 git")
    result = sync_check.git(*args[1:])
    return result.stdout.strip()


def run_status(*args: str) -> bool:
    if not args or args[0] != "git":
        raise ValueError("release_checklist.run_status 只允许调用 git")
    return sync_check.git(*args[1:]).returncode == 0


def check_sync() -> tuple[bool, str]:
    try:
        results = sync_check.evaluate("origin", "github", "master")
    except (OSError, sync_check.subprocess.TimeoutExpired) as exc:
        return False, f"同步检查异常: {exc}"
    failures = [item.message for item in results if not item.ok]
    if failures:
        return False, "；".join(failures) or "远端同步检查失败"
    return True, "工作区、Gitee、标签和 GitHub 镜像均已同步"


def check_versions() -> tuple[bool, str]:
    cmake = (ROOT / "CMakeLists.txt").read_text(encoding="utf-8")
    package = (ROOT / "bindings/python/physfx_py/__init__.py").read_text(encoding="utf-8")
    readme = (ROOT / "README.md").read_text(encoding="utf-8")
    values = [re.search(r"project\(PhysFX VERSION ([^) ]+)", cmake),
              re.search(r'__version__\s*=\s*["\']([^"\']+)', package),
              re.search(r"当前版本[：:]\s*`?v(\d+\.\d+\.\d+(?:-[\w.]+)?)", readme)]
    versions = [match.group(1) if match else "" for match in values]
    ok = len(set(versions)) == 1 and bool(versions[0])
    return ok, f"版本: {', '.join(versions)}"


def check_changelog() -> tuple[bool, str]:
    text = (ROOT / "CHANGELOG.md").read_text(encoding="utf-8")
    ok = "## [Unreleased]" not in text
    return ok, "CHANGELOG 已有明确版本段" if ok else "CHANGELOG 仍有 Unreleased 段"


def expected_tag() -> str:
    cmake = (ROOT / "CMakeLists.txt").read_text(encoding="utf-8")
    match = re.search(r"project\(PhysFX VERSION ([^) ]+)", cmake)
    return f"v{match.group(1)}" if match else ""


def check_tag() -> tuple[bool, str]:
    expected = expected_tag()
    if not expected or expected not in run("git", "tag", "--list", expected).splitlines():
        return False, f"缺少版本标签 {expected or '未知'}"
    if run("git", "cat-file", "-t", expected) != "tag":
        return False, f"{expected} 不是附注标签"
    if not run_status("git", "merge-base", "--is-ancestor", expected, "HEAD"):
        return False, f"{expected} 不在当前 HEAD 的发布历史中"
    return True, f"附注标签 {expected} 位于当前发布历史中"


def check_ci() -> tuple[bool, str]:
    report = completion_report()
    report_text = report.read_text(encoding="utf-8") if report.exists() else ""
    match = re.search(r"^CI_RUN_URL=(https://\S+)$", report_text, re.MULTILINE)
    ok = bool(match) and "placeholder" not in match.group(1).lower()
    return ok, f"CI URL: {match.group(1)}" if ok else "缺少真实 CI 运行 URL（仅有 workflow 不算证据）"


def check_release() -> tuple[bool, str]:
    report = completion_report()
    text = report.read_text(encoding="utf-8") if report.exists() else ""
    material = re.search(r"^MATERIAL_STATUS=(ready|pending)$", text, re.MULTILINE)
    if material is None:
        return False, "缺少 MATERIAL_STATUS=ready|pending"
    if material.group(1) != "ready":
        return False, "演示物料仍为 pending；完成门禁不允许带入发布"
    if current_phase() >= 8:
        gitee = re.search(r"^GITEE_RELEASE_URL=(https://\S+)$", text, re.MULTILINE)
        github = re.search(r"^GITHUB_RELEASE_URL=(https://\S+)$", text, re.MULTILINE)
        if not gitee or not github:
            return False, "Phase 8 缺少 Gitee/GitHub 双平台 Release URL"
        return True, f"双平台 Release 可查；物料齐全：{gitee.group(1)}；{github.group(1)}"
    release = re.search(r"^RELEASE_URL=(https://\S+)$", text, re.MULTILINE)
    if not release:
        return False, "缺少真实 Release URL"
    return True, f"Release: {release.group(1)}；物料齐全"


def check_roadmap() -> tuple[bool, str]:
    text = (ROOT / "docs/roadmap.md").read_text(encoding="utf-8")
    required = ("Phase 8", "Phase 9") if current_phase() >= 8 else ("Phase 7", "Phase 6")
    ok = all(item in text for item in required)
    return ok, f"路线图包含 {required[0]} 与 {required[1]}" if ok else "路线图缺少下一阶段规划"


def check_degradation() -> tuple[bool, str]:
    report = completion_report()
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
        print(json.dumps({"phase": current_phase(), "checks": results, "all_green": all(item["ok"] for item in results)}, ensure_ascii=False, indent=2))
    else:
        for item in results:
            print(f"{'✅' if item['ok'] else '❌'} {item['name']}: {item['message']}")
    return 0 if all(item["ok"] for item in results) else 1


if __name__ == "__main__":
    raise SystemExit(main())
