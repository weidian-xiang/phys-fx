# PhysFX Engine —— 视频世界编辑器（Video World Editor）
# Copyright (c) 2026 向伟典
#
# 本文件遵循 Apache License 2.0 开源协议，许可证全文见仓库根目录 LICENSE 文件。
# 按 Apache-2.0 要求，修改本文件的衍生版本须在文件中保留显著修改声明。
#
# SPDX-License-Identifier: Apache-2.0

from __future__ import annotations

import json
import threading
import urllib.error
import urllib.request

from cloud.server.app import build_server


def request(server, method: str, path: str, body: dict | None = None, key: str = "") -> tuple[int, dict]:
    data = None if body is None else json.dumps(body).encode()
    request = urllib.request.Request(
        f"http://127.0.0.1:{server.server_port}{path}", data=data, method=method,
        headers={"Content-Type": "application/json", "X-API-Key": key},
    )
    try:
        with urllib.request.urlopen(request) as response:
            return response.status, json.loads(response.read())
    except urllib.error.HTTPError as error:
        return error.code, json.loads(error.read())


def test_cloud_privacy_and_quota(tmp_path) -> None:
    server = build_server("127.0.0.1", 0, tmp_path)
    thread = threading.Thread(target=server.serve_forever, daemon=True)
    thread.start()
    try:
        status, registered = request(server, "POST", "/v1/register", {"email": "demo@example.com"})
        assert status == 201 and registered["telemetry"] == "off"
        key = registered["api_key"]
        assert request(server, "POST", "/v1/render/jobs", {"input": "clip.mp4"}, key)[0] == 202
        status, jobs = request(server, "GET", "/v1/templates")
        assert status == 200 and jobs["templates"] == []
        assert request(server, "POST", "/v1/render/jobs", {}, "bad-key")[0] == 401
    finally:
        server.shutdown()
        server.server_close()
