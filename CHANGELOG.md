# 变更日志

本文件遵循 Keep a Changelog，版本号遵循语义化版本规范。

## [Unreleased]

- 继续验证真实 FFmpeg/ONNX 实体依赖和端到端视频往返。

## [0.2.0] - 2026-08-19

### 新增

- 固化 Conventional Commits、DCO、分支模型、PR 自检和双平台 CI 配置。
- 新增可选 FFmpeg RGB24/H.264 视频读写器与 VideoIoFactory；无开发包时保留桩回退。
- 新增 ONNX Runtime SAM/XMem 适配器接口和模型下载/校验脚本。
- 新增纯 CPU 火花/烟尘粒子系统、精灵 alpha/加色合成和语义掩码遮挡。
- 新增 `particle_occlusion_demo.json` 示例和性能基线记录。

### 兼容性

- `Frame`、`SimulationResult`、`Config` 仅追加带默认值字段，Phase 1/2 调用保持兼容。
- 发布验收默认采用 `PHYSFX_WITH_FFMPEG=OFF`、`PHYSFX_WITH_ONNX=OFF`；实体依赖的 ON 构建
  需在具备合规开发包的环境中单独验证。
