# PhysFX Engine

PhysFX Engine 是面向短视频创作者和独立开发者的开源视频世界编辑器：把 AI 场景理解、语义编辑、物理特效和未来神经渲染统一到一条易扩展的管线中。
核心引擎采用 Apache-2.0，复杂能力通过插件和模板封装，普通用户面对极简操作，开发者保留完整接口栈。

## Phase 2 快速开始

环境要求：CMake 3.26+、C++20 编译器、Python 3.11+。Windows MinGW 可直接运行：

```text
cmake --workflow --preset windows
build\windows\apps\cli\physfx.exe --version
build\windows\apps\cli\physfx.exe run --config examples\empty_pipeline.json
```

一键工作流会完成配置、构建和 CTest。当前验证不接入 FFmpeg、OpenCV、Taichi 或网络服务，默认零实体依赖；vcpkg manifest 仅声明未来可选的 GoogleTest/pybind11。

Python 门面：

```text
python -m pip install -e bindings/python
python -c "import physfx_py; print(physfx_py.version())"
python -m pytest bindings/python/tests
```

## 五阶段空管线

`perception → semantics → editing → physics → render`。默认配置会使用所有桩实现，输出带阶段名、状态和耗时的日志；`semantic_edit_pipeline.json` 展示神经渲染直通路径。

## 目录与边界

- `core/`：Frame、SemanticScene、SceneContext、Status/Result 等跨模块契约。
- `perception/`：光照、深度、相机、地面和遮挡接口。
- `semantics/`：分割、跟踪、实体属性接口与假实体桩。
- `editing/`：可序列化命令、撤销/重做栈和空命令。
- `physics/`：物理模拟抽象与 MockSimulator。
- `neural_render/`、`compositing/`：神经重渲染与物理合成的统一渲染出口。
- `plugins/`、`cloud/`：模板授权、云渲染和市场的商业扩展边界，仅接口预留。

详细架构见 [docs/architecture.md](docs/architecture.md)，开源治理见 [docs/open-source-plan.md](docs/open-source-plan.md)，商业边界见 [docs/business-model.md](docs/business-model.md)。

## 当前限制

Phase 2 不实现真实模型、视频 IO、物理求解、GPU 渲染、网络访问或 GUI。开源版不阉割已有能力，未来商业服务只提供增量模板、云算力和企业支持。
