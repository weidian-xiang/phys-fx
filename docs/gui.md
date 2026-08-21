# GUI 使用手册与架构

启用 `PHYSFX_BUILD_GUI=ON` 后会获取锁定的 GLFW 3.4 与 Dear ImGui 1.90.9，构建真实
OpenGL 窗口 `physfx_gui`。GUI 工程由 `Project` 表示，包含输入/输出
视频和线性节点列表；节点的 `commandJson` 直接保存 Phase 4 命令 JSON。调用
`ProjectCompiler::compileEditScript` 可得到 CLI 同构的 `{version:1,commands:[...]}` 编辑脚本，
因此 GUI 不会形成第二套编辑引擎。

`PreviewWindow`、`Timeline`、`CommandStackPanel` 和 `NodeGraph` 提供可测试的交互状态层：
点击会被限制在当前帧尺寸内，时间轴支持播放/暂停、逐帧和 scrub，命令面板维护撤销/重做
游标，节点图只负责收集命令并交给同一编译器。窗口已经绑定这些状态，提供 1280×720
预览、左键点选、右键编辑菜单、撤销/重做、时间轴、节点图和编辑脚本导出。中文字体按
微软雅黑、黑体、Noto CJK、文泉驿和苹方顺序探测；均不存在时才使用 ImGui 默认字体。

`physfx_gui --self-test` 不创建窗口，用于 CI 校验 GUI/CLI 脚本等价合同。Linux CI 需要
OpenGL 与 X11 开发包；`PHYSFX_BUILD_GUI=OFF` 不下载或编译任何窗口依赖。

节点类型为 `source`、`perception`、`semantics`、`editing`、`physics`、`render`、`template`
和 `output`。窗口实现线性/浅层节点编排；循环、条件和子图仍不在范围内。Phase 7 增加
关键帧曲线：移动轨迹与强度参数支持 `linear`/`smooth` 数值关键帧，参数改变先进入半
分辨率快速预览，确认后切换完整渲染。曲线最终写入 `animate_parameter`，CLI 与 GUI 使用
同一编辑脚本合同。
