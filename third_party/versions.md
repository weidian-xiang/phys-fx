# 第三方依赖版本锁定

本目录只保存版本、来源和校验规则，不保存二进制包。下载产物必须放在被 Git
忽略的缓存目录中。

| 组件 | 建议版本 | 来源 | 许可 | 备注 |
| --- | --- | --- | --- | --- |
| FFmpeg shared | 8.1.2-44-g7c533d0f86 | BtbN autobuild-2026-08-18-15-03 | LGPL-2.1+ | Windows SHA256 `c057…3121`；Linux SHA256 `90f9…e730` |
| ONNX Runtime | 1.18.1 | Microsoft.ML.OnnxRuntime NuGet | MIT | SHA512 `c546…214d`；含多平台原生库 |

精确构建由 `tools/setup_deps.py` 的平台清单驱动，Windows 与 Linux 均下载同一版本的
LGPL shared 包。升级版本时必须同步更新 ADR、NOTICE 和本文件，并记录完整 SHA256。
