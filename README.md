# PhysFX Engine

PhysFX Engine 是面向短视频创作者和独立开发者的开源视频世界编辑器：把 AI 场景理解、语义编辑、物理特效和未来神经渲染统一到一条易扩展的管线中。
核心引擎采用 Apache-2.0，复杂能力通过插件和模板封装，普通用户面对极简操作，开发者保留完整接口栈。

## Phase 3 快速开始

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

`perception → semantics → editing → physics → render`。默认配置会使用所有桩实现，输出带阶段名、状态和耗时的日志；`semantic_edit_pipeline.json` 展示神经渲染直通路径。
`examples/particle_occlusion_demo.json` 选择 `simple_particles + sprite`，在有 FFmpeg
动态开发包时逐帧读写视频；没有实体包时会明确回退或报中文状态，不会静默伪造输出。

## 目录与边界

- `core/`：Frame、SemanticScene、SceneContext、Status/Result 等跨模块契约。
- `perception/`：光照、深度、相机、地面和遮挡接口。
- `semantics/`：分割、跟踪、实体属性接口与假实体桩。
- `editing/`：可序列化命令、撤销/重做栈和空命令。
- `physics/`：物理模拟抽象、MockSimulator 和纯 CPU 火花/烟尘粒子系统。
- `neural_render/`、`compositing/`：神经重渲染与物理合成的统一渲染出口。
- `plugins/`、`cloud/`：模板授权、云渲染和市场的商业扩展边界，仅接口预留。

详细架构见 [docs/architecture.md](docs/architecture.md)，开源治理见 [docs/open-source-plan.md](docs/open-source-plan.md)，商业边界见 [docs/business-model.md](docs/business-model.md)。

## 当前限制

Phase 3 的 ONNX 模型输入输出仍按具体模型卡片装载，真实权重不入库；FFmpeg ON 路径
要求 LGPL 动态开发包。开源版不阉割已有能力，未来商业服务只提供增量模板、云算力和企业支持。
