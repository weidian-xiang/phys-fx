# PhysFX 节点式 GUI

`Project` 表示视频源、线性节点和命令序列，`ProjectCompiler` 将节点编译为 Phase 4
编辑脚本 JSON。Dear ImGui 1.90.9 + GLFW 3.4 + OpenGL 窗口绑定同一数据层，核心引擎
不反向依赖 GUI；默认构建使用 `-DPHYSFX_BUILD_GUI=OFF`，开启后构建真实 `physfx_gui`
应用。CI 可运行 `physfx_gui --self-test` 验证无窗口导出合同。

`.physfxproj` 是 JSON 集合，节点类型包括 `source`、`perception`、`semantics`、
`editing`、`physics`、`render`、`template` 和 `output`。本阶段只支持线性/浅层编排，
不引入循环、条件、子图或关键帧曲线。
