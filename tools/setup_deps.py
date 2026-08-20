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

import argparse
import hashlib
import json
import shutil
import sys
import tarfile
import urllib.request
import zipfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
THIRD_PARTY = ROOT / "third_party"
CACHE = THIRD_PARTY / "cache" / "downloads"

MANIFEST = {
    "ffmpeg": {
        "version": "8.1.2-44-g7c533d0f86",
        "url": "https://ffmpeg.org/download.html",
        "license": "LGPL-2.1-or-later",
        "archives": {
            "windows": {
                "archive_url": "https://github.com/BtbN/FFmpeg-Builds/releases/download/autobuild-2026-08-18-15-03/ffmpeg-n8.1.2-44-g7c533d0f86-win64-lgpl-shared-8.1.zip",
                "sha256": "c057a7b631a8425e6247ee2a632370045662229564aa1a2ca7ecd42c00123121",
            },
            "linux": {
                "archive_url": "https://github.com/BtbN/FFmpeg-Builds/releases/download/autobuild-2026-08-18-15-03/ffmpeg-n8.1.2-44-g7c533d0f86-linux64-lgpl-shared-8.1.tar.xz",
                "sha256": "90f9471dbd7a1fedbeb1ccfff0e60edaff5d3f5b78db00858f472263f85de730",
            },
        },
        "strip_top_level": True,
        "destination": "ffmpeg",
    },
    "onnxruntime": {
        "version": "1.18.1",
        "url": "https://github.com/microsoft/onnxruntime/releases",
        "license": "MIT",
        "archive_url": "https://api.nuget.org/v3-flatcontainer/microsoft.ml.onnxruntime/1.18.1/microsoft.ml.onnxruntime.1.18.1.nupkg",
        "sha512": "c546c0c960b26078261b638168f1c03acaaf4085930094b9f4b774d71f258a2647203f000871050667325628061f146d06cd16566ec3f296d29f414c7920214d",
        "destination": "onnxruntime",
    },
}


def file_digest(path: Path, algorithm: str) -> str:
    digest = hashlib.new(algorithm)
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def archive_valid(path: Path, item: dict[str, object]) -> bool:
    return path.exists() and all(
        not item.get(algorithm)
        or file_digest(path, algorithm).lower() == str(item[algorithm]).lower()
        for algorithm in ("sha256", "sha512")
    )


def extract_zip(archive: Path, destination: Path, strip_top_level: bool) -> None:
    with zipfile.ZipFile(archive) as package:
        for member in package.infolist():
            parts = Path(member.filename).parts
            if strip_top_level and parts:
                parts = parts[1:]
            if not parts:
                continue
            target = destination.joinpath(*parts)
            if destination.resolve() not in target.resolve().parents:
                raise zipfile.BadZipFile("压缩包包含越界路径")
            if member.is_dir():
                target.mkdir(parents=True, exist_ok=True)
                continue
            target.parent.mkdir(parents=True, exist_ok=True)
            with package.open(member) as source, target.open("wb") as output:
                shutil.copyfileobj(source, output)


def extract_tar(archive: Path, destination: Path, strip_top_level: bool) -> None:
    with tarfile.open(archive, "r:*") as package:
        members = package.getmembers()
        top_levels = {Path(member.name).parts[0] for member in members if Path(member.name).parts}
        if strip_top_level and len(top_levels) != 1:
            raise tarfile.TarError("压缩包顶层目录数量不符合预期")
        prefix = next(iter(top_levels)) if strip_top_level else None
        for member in members:
            parts = Path(member.name).parts
            if prefix is not None and parts and parts[0] == prefix:
                parts = parts[1:]
            if not parts:
                continue
            member.name = str(Path(*parts))
            target = destination.joinpath(*parts).resolve()
            if destination.resolve() not in target.parents:
                raise tarfile.TarError("压缩包包含越界路径")
            if member.issym() or member.islnk():
                link_target = (target.parent / member.linkname).resolve()
                if destination.resolve() not in link_target.parents:
                    raise tarfile.TarError("压缩包包含越界链接")
        package.extractall(destination, members=members, filter="data")


def extract_archive(archive: Path, destination: Path, strip_top_level: bool) -> None:
    if zipfile.is_zipfile(archive):
        extract_zip(archive, destination, strip_top_level)
        return
    if tarfile.is_tarfile(archive):
        extract_tar(archive, destination, strip_top_level)
        return
    raise ValueError("不支持的依赖压缩格式")


def platform_name() -> str:
    if sys.platform == "win32":
        return "windows"
    if sys.platform.startswith("linux"):
        return "linux"
    return sys.platform


def resolve_platform_item(item: dict[str, object]) -> dict[str, object]:
    resolved = dict(item)
    archives = resolved.pop("archives", None)
    if archives:
        platform_archive = archives.get(platform_name())
        if platform_archive is None:
            raise ValueError(f"当前平台没有锁定的依赖包: {platform_name()}")
        resolved.update(platform_archive)
    return resolved


def main() -> int:
    parser = argparse.ArgumentParser(description="准备 PhysFX 可选第三方依赖")
    parser.add_argument("--component", choices=["ffmpeg", "onnxruntime", "all"], default="all")
    parser.add_argument("--manifest", type=Path, help="下载清单 JSON（可选）")
    parser.add_argument("--check", action="store_true", help="仅检查，不下载或解包")
    args = parser.parse_args()
    selected = MANIFEST if args.component == "all" else {args.component: MANIFEST[args.component]}
    CACHE.mkdir(parents=True, exist_ok=True)
    if args.manifest:
        try:
            selected = json.loads(args.manifest.read_text(encoding="utf-8"))
        except (OSError, json.JSONDecodeError) as exc:
            print(f"依赖清单读取失败: {exc}", file=sys.stderr)
            return 2
    for name, item in selected.items():
        try:
            item = resolve_platform_item(item)
        except ValueError as exc:
            print(f"{name}: {exc}", file=sys.stderr)
            return 1
        destination = THIRD_PARTY / item.get("destination", name)
        destination.mkdir(parents=True, exist_ok=True)
        print(f"{name}: 版本 {item.get('version', '未指定')}，许可 {item.get('license', '未指定')}")
        if not args.check and item.get("archive_url"):
            archive = CACHE / Path(item["archive_url"]).name
            try:
                if not archive_valid(archive, item):
                    archive.unlink(missing_ok=True)
                    print(f"正在下载 {name}: {item['archive_url']}")
                    partial = archive.with_name(archive.name + ".partial")
                    partial.unlink(missing_ok=True)
                    with urllib.request.urlopen(item["archive_url"], timeout=60) as response, partial.open(
                        "wb"
                    ) as output:
                        shutil.copyfileobj(response, output)
                    partial.replace(archive)
            except (OSError, ValueError) as exc:
                print(f"下载失败: {exc}; 请手动下载并放入 {destination}", file=sys.stderr)
                return 1
            for algorithm in ("sha256", "sha512"):
                expected = item.get(algorithm)
                if expected and file_digest(archive, algorithm).lower() != expected.lower():
                    print(f"{algorithm.upper()} 校验失败: {archive}", file=sys.stderr)
                    return 1
            try:
                extract_archive(archive, destination, bool(item.get("strip_top_level", False)))
            except (OSError, ValueError, zipfile.BadZipFile, tarfile.TarError) as exc:
                print(f"依赖解包失败: {exc}", file=sys.stderr)
                return 1
            print(f"已下载并校验: {archive}")
        elif not any(destination.iterdir()):
            print(f"未发现实体包。请按 third_party/versions.md 放入: {destination}")
    print("依赖准备检查完成；未启用的实体依赖不会影响默认构建。")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
