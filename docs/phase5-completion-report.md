# Phase 5 完成报告

## 已交付

- `PHYSFX_BUILD_GUI` 可选构建 `physfx_gui`；`ProjectCompiler` 将线性节点编译为 Phase 4
  编辑脚本 JSON，并提供 `.physfxproj` 保存/加载模型。
- 新增 `SetSeason`、泼水/爆炸/换季节模板注册、密度场合成和季节色调合成；CLI 提供
  `splash`、`explode`、`season`。
- 新增确定性的深度、天空二分类和启发式光照估计；新增 96×54 半拉格朗日密度场回退，
  无外部运行时也能完成稳定性测试。
- `.pfxtpl` ZIP 格式和 OpenSSL Ed25519 离线打包/验签工具可用；未签名模板保持可加载。
- ADR-0015–0017、GUI 手册、素材入库标准、性能增量基线和安全密钥纪律已补齐。

## 诚实的降级项

本提交没有伪称完成真实 SAM/XMem、Depth-Anything、ProPainter/E2FGVI 或 Taichi 外部
运行时张量契约。ONNX 适配器仍要求具体模型卡片，未安装权重时使用传统/启发式路径；
密度场是 C++ 零依赖的桥接兼容实现。3DGS 探索、真实 ImGui 窗口、云渲染和完整 BRDF
材质均留在 Phase 6。动态视频素材的神经修复质量门槛不能由本地无权重测试替代。

这意味着产品路线当前仍是“可演示编辑器 + 明确降级提示”：无权重素材默认走传统分割、
启发式感知和 CPU 修复；动态场景的点选精度、修复质量和 15 fps 预览不能作为 Phase 6
云渲染/模板市场的承诺。真实模型卡片锁定后，需重新验收 §4.1/§4.2 门槛，再扩大默认
适用素材范围。

## 验证证据

在本地 CMake 3.24 验证副本（工程声明仍为 3.26+）中，`PHYSFX_BUILD_GUI=ON` 和
`PHYSFX_BUILD_GUI=OFF` 均构建成功，各自 CTest `21/21` 通过，包含
`phase5_contract_test`；版权扫描和 `git diff --check` 通过。
Python 工具测试在工作区临时目录权限修正后 `9/9` 通过。

## 发布状态

代码与文档已准备 `0.4.0-dev`；本地未执行 Gitee 推送、标签或 Release 创建，也没有虚构
v0.3.0/v0.4.0 的远端链接。发布前应由维护者补充真实模型性能、GUI 录屏、天空素材许可
清单和 CI Actions 运行 URL。
