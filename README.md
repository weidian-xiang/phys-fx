# PhysFX Engine

[![PhysFX CI](https://github.com/weidian-xiang/phys-fx/actions/workflows/ci.yml/badge.svg?branch=master)](https://github.com/weidian-xiang/phys-fx/actions/workflows/ci.yml)

当前版本：`v0.6.0`

PhysFX Engine 是面向短视频创作者和独立开发者的开源视频世界编辑器：把 AI 场景理解、语义编辑、物理特效和未来神经渲染统一到一条易扩展的管线中。
核心引擎采用 Apache-2.0，复杂能力通过插件和模板封装，普通用户面对极简操作，开发者保留完整接口栈。

## Phase 6.1 快速开始

环境要求：CMake 3.26+、C++20 编译器、Python 3.11+。Windows MinGW 可直接运行：

```text
cmake --workflow --preset windows
build\windows\apps\cli\physfx.exe --version
build\windows\apps\cli\physfx.exe run --config examples\empty_pipeline.json
```

一键工作流会完成配置、构建和 CTest。默认关闭实体依赖，保持零依赖构建；可选的
FFmpeg（LGPL 动态库）和 ONNX Runtime 由 `tools/setup_deps.py` 按 `third_party/versions.md`
准备，模型权重由 `tools/download_models.py` 下载到被忽略的 `models/`。

Python 门面：

```text
python -m pip install -e bindings/python
python -c "import physfx_py; print(physfx_py.version())"
python -m pytest bindings/python/tests
```

## 五阶段管线与粒子演示

`perception → semantics → editing → physics → render`。默认配置会使用所有桩实现，输出带阶段名、状态和耗时的日志；`semantic_edit_pipeline.json` 展示神经渲染直通路径，`edit_script_pipeline.json` 展示版本化命令脚本。
`examples/particle_occlusion_demo.json` 选择 `simple_particles + sprite`，在有 FFmpeg
动态开发包时逐帧读写视频；没有实体包时会明确回退或报中文状态，不会静默伪造输出。

模板化命令：

```text
physfx remove input.mp4 --at 320,180 -o clean.mp4
physfx move input.mp4 --at 320,180 --to 520,300 -o moved.mp4
physfx appearance input.mp4 --at 320,180 --material red -o recolor.mp4
physfx smoke input.mp4 --anchor 400,350 -o smoked.mp4
```

无模型环境使用传统分割与 CPU 修复/粒子基线；`smoke`/`splash` 使用稳定的半拉格朗日
密度场桥接实现。真实 SAM/XMem、ProPainter 和外部 Taichi 权重仍按 `needs_models`/
可选依赖策略记录，不会把回退路径伪装成神经推理。

## 目录与边界

- `core/`：Frame、SemanticScene、SceneContext、Status/Result 等跨模块契约。
- `perception/`：光照、深度、相机、地面和遮挡接口。
- `semantics/`：分割、跟踪、实体属性接口与假实体桩。
- `editing/`：可序列化命令、撤销/重做栈和空命令。
- `physics/`：物理模拟抽象、MockSimulator 和纯 CPU 火花/烟尘粒子系统。
- `neural_render/`、`compositing/`：神经重渲染与物理合成的统一渲染出口。
- `plugins/`、`cloud/`：模板授权、默认关闭的云渲染和市场客户端/单机服务。
- `docs/edit-script-format.md`：版本化编辑脚本格式，供 CLI 与 GUI 共用。

详细架构见 [docs/architecture.md](docs/architecture.md)，开源治理见 [docs/open-source-plan.md](docs/open-source-plan.md)，商业边界见 [docs/business-model.md](docs/business-model.md)。
仓库协作与 Gitee→GitHub 镜像规范见 [docs/git-workflow.md](docs/git-workflow.md) 和
[docs/mirror-sync.md](docs/mirror-sync.md)。Gitee 是主仓，GitHub
（https://github.com/weidian-xiang/phys-fx）是只读镜像。

## 当前限制

`v0.5.0` 已落地命令栈、编辑脚本、CPU 删除/移动/修复、模板 CLI、真实 ImGui/OpenGL
窗口、外部 Taichi CPU 求解器和默认关闭的本地云/市场基线。SAM/XMem、ProPainter 与
Depth Anything 因供应链未形成固定 ONNX 导出、哈希和许可结论，永久采用提示分割 +
IoU 关联、CPU 修复和启发式深度限制；具体素材白名单与商业影响见
[`docs/model-cards.md`](docs/model-cards.md)。这些路径不代表神经推理质量。

GUI 可用以下开关构建；关闭时不获取窗口依赖：

```powershell
cmake -S . -B build/gui -DPHYSFX_BUILD_GUI=ON
cmake --build build/gui --target physfx_gui
build\gui\physfx_gui.exe
```

模板命令：`physfx splash`、`physfx explode`、`physfx season --to winter`。`.pfxtpl`
可用 `python tools/pack_template.py` 离线打包和 Ed25519 验签；未签名模板仍可加载。
