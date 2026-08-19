# Phase 3 示例

`particle_occlusion_demo.json` 演示真实视频读取、语义实体轨迹、CPU 火花粒子和掩码
遮挡合成。仓库不提交视频和模型权重；先用 FFmpeg 生成一个短测试输入：

```powershell
ffmpeg -f lavfi -i testsrc2=size=640x360:rate=30 -t 3 -pix_fmt yuv420p build/demo-input.mp4
build\windows\apps\cli\physfx.exe run --config examples\particle_occlusion_demo.json
```

输出为 `build/particle-occlusion-demo.mp4`。示例默认使用确定性语义桩生成中央目标掩码，
因此无需模型即可验证遮挡链路；准备好经许可审查的 SAM/XMem ONNX 权重后，将
`semantics.implementation` 改为 `onnx`。没有 FFmpeg 开发包时，构建会回退视频 IO
桩并给出中文错误；先运行 `tools/setup_deps.py` 并按 `third_party/versions.md` 放置
LGPL 动态开发包。

常见问题：本机只有启用 GPL 的静态 FFmpeg 命令行工具时，不得将其作为 PhysFX
发布依赖；它只可用于本地生成测试素材。模型缺失时 `needs_models` 测试自动跳过。
