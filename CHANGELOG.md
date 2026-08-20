# 变更日志

本文件遵循 Keep a Changelog，版本号遵循语义化版本规范。

## [Unreleased]

- 锁定并实现 SAM/XMem 类 ONNX 模型的真实张量推理契约。
- 规范 Gitee 到 GitHub 的单向镜像、只读远程和镜像仓贡献入口。

## [0.2.0] - 2026-08-20

### 新增

- 固化 Conventional Commits、DCO、分支模型、PR 自检和双平台 CI 配置。
- 新增可选 FFmpeg RGB24/H.264 视频读写器与 VideoIoFactory；无开发包时保留桩回退。
- 新增 ONNX Runtime SAM/XMem 适配器接口和模型下载/校验脚本。
- 新增纯 CPU 火花/烟尘粒子系统、精灵 alpha/加色合成和语义掩码遮挡。
- 新增提示点颜色连通分割和质心时序跟踪，作为无模型环境的真实计算降级路径。
- 新增 `particle_occlusion_demo.json` 示例和性能基线记录。
- 新增锁定的跨平台 FFmpeg LGPL shared 下载、哈希校验和真实视频冒烟制品。

### 兼容性

- `Frame`、`SimulationResult`、`Config` 仅追加带默认值字段，Phase 1/2 调用保持兼容。
- ONNX Runtime 1.18.1 已完成 SHA512 校验、MinGW ON 构建和无模型测试；修复 `_stdcall`
  头文件兼容问题。
- FFmpeg 与 ONNX Runtime 同时开启的 MinGW 构建通过；FFmpeg 往返测试和 30 帧 demo
  冒烟通过，默认 OFF 构建仍保持零依赖。
- SAM/XMem 类适配器尚未锁定具体模型张量契约；v0.2.0 的可演示语义能力明确采用
  传统降级路径，不宣称神经模型质量。
