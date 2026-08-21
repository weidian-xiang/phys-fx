# v1.0 公共 API 审计

审计基线：`6cdc081`；状态：冻结候选，等待 `docs/v1-freeze.md` 的维护者确认。

## 公共边界

公共 C++ 头文件只来自以下目录：`core`、`perception`、`semantics`、`editing`、`physics`、
`compositing`、`neural_render`、`plugins`、`platform`、`cloud`、`pipeline` 各自的
`include/physfx/<module>/`。`apps/gui/src/`、所有 `src/`、`tests/`、`plugins/examples/`
和研究 spike 均为内部实现，不安装、不进入兼容承诺。

逐目录检查未发现公共 include path 指向 `src/` 或应用私有头。现有 Stub、传统实现、
FFmpeg/ONNX/Taichi 适配器已经位于公开 include 树并被 0.x 使用，因此 v1.0 将其作为受支持
公共类型冻结；不能在 1.x 中借“内部实现”名义移动或删除。

## 命名与可见性

`core/include/physfx/core/Export.h` 定义唯一的 `PHYSFX_API`。静态构建为空；共享库构建
由 `PHYSFX_SHARED` 和 `PHYSFX_BUILDING_LIBRARY` 选择导出/导入属性。公共的非内联自由
函数和含非内联/虚函数的类使用该宏，模板与纯数据类型不导出实例。

`core/include/physfx/core/Version.h` 提供 `physfx::v1::<module>` 命名别名。为避免在
1.0 冻结时破坏所有 0.x 源码和符号名，现有 `physfx::<module>` 是实际命名空间，
`physfx::v1` 是稳定、推荐的新入口。该兼容结构在 1.x 中不得反转或删除。

`tests/public_api_contract_test.cpp` 编译所有公共模块的代表头，并验证旧入口与 `v1` 入口
指向同一类型；`tools/check_public_api.py` 检查公开路径、内部 include 泄漏和导出宏使用，
两者均作为 CTest/CI 常驻门禁。

## 审计结论

- 公开头所有权明确，GUI 私有模型未误纳入 SDK；
- 新增版本化入口不破坏 0.x 源码；
- 静态库当前没有平台导出需求，共享库可见性合同已预留并由门禁约束；
- C++ ABI 只在相同工具链/运行库/架构下承诺，跨工具链插件要求重编译；
- 后续公共头新增、移动或删除必须同步冻结清单、兼容政策和 API 合同测试。
