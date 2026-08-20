# 第三方依赖说明

Phase 3 的实体依赖由 `tools/setup_deps.py` 按 `versions.md` 的锁定版本、平台和哈希
下载。FFmpeg 只接受 BtbN 的 LGPL shared 包，ONNX Runtime 使用 Microsoft 官方 NuGet
包；下载缓存和解包目录均被 Git 忽略，不得入库。

默认构建仍为零依赖。OpenCV、Taichi、OpenGL/Vulkan 等尚未引入；新增或升级依赖必须
先完成许可证审查、版本锁定、哈希校验和 NOTICE 更新。
