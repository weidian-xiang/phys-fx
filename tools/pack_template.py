#!/usr/bin/env python3
# PhysFX Engine —— 视频世界编辑器（Video World Editor）
# Copyright (c) 2026 向伟典
#
# 本文件遵循 Apache License 2.0 开源协议，许可证全文见仓库根目录 LICENSE 文件。
# 按 Apache-2.0 要求，修改本文件的衍生版本须在文件中保留显著修改声明。
#
# SPDX-License-Identifier: Apache-2.0

"""Pack and verify PhysFX .pfxtpl template packages.

The package is a deterministic ZIP containing manifest.json, template files and
an optional SIGNATURE. Ed25519 keys are supplied by the caller and are never
written to the repository or CI logs. Unsigned packages remain loadable.
"""

from __future__ import annotations

import argparse
import base64
import hashlib
import json
import subprocess
import tempfile
import zipfile
from pathlib import Path


def _payload(manifest: bytes, files: list[tuple[str, bytes]]) -> bytes:
    payload = bytearray(manifest)
    for name, data in sorted(files):
        payload.extend(name.encode("utf-8"))
        payload.extend(b"\0")
        payload.extend(data)
    return bytes(payload)


def _openssl_sign(payload: bytes, key: Path) -> bytes:
    source = tempfile.NamedTemporaryFile(delete=False)
    target = tempfile.NamedTemporaryFile(delete=False)
    try:
        source.write(payload)
        source.close()
        target.close()
        subprocess.run(
            ["openssl", "pkeyutl", "-sign", "-rawin", "-inkey", str(key),
             "-in", source.name, "-out", target.name],
            check=True,
        )
        return Path(target.name).read_bytes()
    finally:
        Path(source.name).unlink(missing_ok=True)
        Path(target.name).unlink(missing_ok=True)


def _openssl_verify(payload: bytes, signature: bytes, public_key: Path) -> bool:
    source = tempfile.NamedTemporaryFile(delete=False)
    sig = tempfile.NamedTemporaryFile(delete=False)
    try:
        source.write(payload)
        source.close()
        sig.write(signature)
        sig.close()
        result = subprocess.run(
            ["openssl", "pkeyutl", "-verify", "-rawin", "-pubin", "-inkey", str(public_key),
             "-in", source.name, "-sigfile", sig.name],
            capture_output=True,
            text=True,
        )
        return result.returncode == 0
    finally:
        Path(source.name).unlink(missing_ok=True)
        Path(sig.name).unlink(missing_ok=True)


def pack(input_dir: Path, output: Path, key: Path | None = None, public_key: str = "") -> None:
    files: list[tuple[str, bytes]] = []
    for path in sorted(input_dir.rglob("*")):
        if path.is_file() and path.name not in {"manifest.json", "SIGNATURE"}:
            files.append((path.relative_to(input_dir).as_posix(), path.read_bytes()))
    manifest_path = input_dir / "manifest.json"
    manifest = json.loads(manifest_path.read_text(encoding="utf-8")) if manifest_path.exists() else {
        "packageId": input_dir.name, "version": "0.5.0", "publisher": "unknown",
    }
    manifest.setdefault("signatureAlgorithm", "Ed25519" if key else "")
    manifest.setdefault("contentDigest", hashlib.sha256(_payload(b"", files)).hexdigest())
    if public_key:
        manifest["publicKey"] = public_key
    manifest_bytes = (json.dumps(manifest, ensure_ascii=False, sort_keys=True, separators=(",", ":")) + "\n").encode()
    signature = _openssl_sign(_payload(manifest_bytes, files), key) if key else b""
    with zipfile.ZipFile(output, "w", compression=zipfile.ZIP_DEFLATED) as archive:
        archive.writestr("manifest.json", manifest_bytes)
        for name, data in files:
            if name != "manifest.json":
                archive.writestr(name, data)
        if signature:
            archive.writestr("SIGNATURE", base64.b64encode(signature).decode("ascii"))


def verify(package: Path, public_key: Path | None = None) -> int:
    with zipfile.ZipFile(package) as archive:
        manifest_bytes = archive.read("manifest.json")
        manifest = json.loads(manifest_bytes)
        files = [(name, archive.read(name)) for name in archive.namelist()
                 if name not in {"manifest.json", "SIGNATURE"}]
        if "SIGNATURE" not in archive.namelist():
            print("未认证来源：模板未签名（开源版允许加载）")
            return 0
        signature = base64.b64decode(archive.read("SIGNATURE"))
        key = public_key
        if key is None:
            print("验签失败：缺少公钥")
            return 1
        if not _openssl_verify(_payload(manifest_bytes, files), signature, key):
            print("验签失败：模板内容可能已被篡改")
            return 1
        print(f"已认证模板：{manifest.get('packageId', 'unknown')} Ed25519")
        return 0


def main() -> int:
    parser = argparse.ArgumentParser(description="PhysFX .pfxtpl 打包与 Ed25519 验签工具")
    sub = parser.add_subparsers(dest="command", required=True)
    packer = sub.add_parser("pack")
    packer.add_argument("input", type=Path)
    packer.add_argument("output", type=Path)
    packer.add_argument("--key", type=Path)
    packer.add_argument("--public-key", default="")
    checker = sub.add_parser("verify")
    checker.add_argument("package", type=Path)
    checker.add_argument("--public-key", type=Path)
    args = parser.parse_args()
    if args.command == "pack":
        pack(args.input, args.output, args.key, args.public_key)
        return 0
    return verify(args.package, args.public_key)


if __name__ == "__main__":
    raise SystemExit(main())
