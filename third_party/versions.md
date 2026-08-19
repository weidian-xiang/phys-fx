# 第三方依赖版本锁定

本目录只保存版本、来源和校验规则，不保存二进制包。下载产物必须放在被 Git
忽略的缓存目录中。

| 组件 | 建议版本 | 来源 | 许可 | 备注 |
| --- | --- | --- | --- | --- |
| FFmpeg shared | 7.x（LGPL 构建） | https://ffmpeg.org/download.html | LGPL-2.1+ | 必须动态链接，禁止 GPL 组件；本机 GPL 命令行不可替代 |
| ONNX Runtime | 1.18.x | https://github.com/microsoft/onnxruntime/releases | MIT | 仅启用 ONNX 选项时获取 |

精确构建由 `tools/setup_deps.py` 的清单驱动。升级版本时必须同步更新 ADR、NOTICE
和本文件，并记录 SHA256。
