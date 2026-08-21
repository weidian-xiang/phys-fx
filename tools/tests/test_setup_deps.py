"""
 * PhysFX Engine —— 视频世界编辑器（Video World Editor）
 * Copyright (c) 2026 向伟典
 *
 * 本文件遵循 Apache License 2.0 开源协议，许可证全文见仓库根目录 LICENSE 文件。
 * 按 Apache-2.0 要求，修改本文件的衍生版本须在文件中保留显著修改声明。
 *
 * SPDX-License-Identifier: Apache-2.0
"""

import hashlib
import io
import tarfile
import zipfile

from tools.setup_deps import (
    archive_valid,
    extract_archive,
    extract_zip,
    prepare_runtime_aliases,
    resolve_platform_item,
)


def test_archive_hash_and_strip_top_level(tmp_path) -> None:
    archive = tmp_path / "dependency.zip"
    with zipfile.ZipFile(archive, "w") as package:
        package.writestr("dependency/include/example.h", "header")
    expected = hashlib.sha256(archive.read_bytes()).hexdigest()
    assert archive_valid(archive, {"sha256": expected})

    destination = tmp_path / "installed"
    destination.mkdir()
    extract_zip(archive, destination, True)
    assert (destination / "include" / "example.h").read_text() == "header"


def test_invalid_archive_hash_is_rejected(tmp_path) -> None:
    archive = tmp_path / "dependency.zip"
    archive.write_bytes(b"bad")
    assert not archive_valid(archive, {"sha256": "0" * 64})


def test_tar_archive_hash_and_strip_top_level(tmp_path) -> None:
    archive = tmp_path / "dependency.tar.xz"
    data = b"header"
    with tarfile.open(archive, "w:xz") as package:
        member = tarfile.TarInfo("dependency/include/example.h")
        member.size = len(data)
        package.addfile(member, io.BytesIO(data))
    destination = tmp_path / "installed"
    destination.mkdir()
    extract_archive(archive, destination, True)
    assert (destination / "include" / "example.h").read_bytes() == data


def test_platform_archive_overrides_common_fields() -> None:
    resolved = resolve_platform_item(
        {"version": "1", "archives": {"windows": {"archive_url": "win.zip"}, "linux": {"archive_url": "linux.tar.xz"}}}
    )
    assert resolved["archive_url"].endswith(("win.zip", "linux.tar.xz"))


def test_linux_onnxruntime_soname_alias(tmp_path) -> None:
    runtime = tmp_path / "runtimes" / "linux-x64" / "native"
    runtime.mkdir(parents=True)
    source = runtime / "libonnxruntime.so"
    source.write_bytes(b"runtime")
    prepare_runtime_aliases("onnxruntime", tmp_path, "1.18.1", "linux")
    alias = runtime / "libonnxruntime.so.1.18.1"
    assert alias.exists()
    assert alias.read_bytes() == b"runtime"
