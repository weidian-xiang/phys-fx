# 变更日志

本文件遵循 Keep a Changelog，版本号遵循语义化版本规范。

## [0.5.0] - 2026-08-21

### 新增

- 新增本地优先的云渲染任务客户端、模板市场客户端、SQLite 单机服务和 Docker Compose 部署文件。
- 新增 `cloud render`、`market search`、`market install` CLI；云与市场用户入口默认关闭。
- 新增 GLFW 3.4 + Dear ImGui 1.90.9 + OpenGL 真实窗口，覆盖预览、点选、右键编辑、
  命令栈、时间轴、节点图和脚本导出。
- 新增 `tools/sync_check.py`、远端同步 CI 门禁和推送/发布永久解耦规范。
- 新增真实 Taichi 1.7.4 CPU 烟雾/泼水运行时路径与 96×54 明确回退。

### 修复与限制

- 修复模板市场 `describe()` 遍历临时结果导致的悬空引用风险和合同测试的运行目录依赖。
- 终局模型路径固定为逐帧提示分割 + IoU 关联、CPU 修复和启发式深度；SAM/XMem、
  ProPainter、Depth Anything 因缺少合格的固定 ONNX 导出、哈希及许可结论，永久列为
  已知限制，不进入默认云服务质量承诺。

## [0.4.0] - 2026-08-21

### 新增

- 新增可裁剪 GUI 工程模型、节点脚本编译器和 `.physfxproj` 文档契约。
- 新增 `splash`、`explode`、`season` 模板入口，`SetSeason` 命令和密度场/季节合成基线。
- 新增确定性深度、天空分割、启发式光照估计器和 `.pfxtpl` Ed25519 打包/验签工具。
- 新增 ADR-0015–0017、GUI 手册和天空素材入库标准。

## [0.3.0] - 2026-08-21

### 新增

- 新增可执行编辑命令栈、版本化编辑脚本、删除/移动/复制/外观替换 CPU 基线。
- 新增 `remove`、`move`、`appearance`、`smoke` 模板化 CLI 和编辑模板注册表。
- 新增视频修复 CPU 适配器、分层移动合成、Taichi 进程桥接口与 ADR-0011–0014。
- 规范 Gitee 到 GitHub 单向镜像、提交/DCO 检查和维护者直推规则。

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
