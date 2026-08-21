# PhysFX Engine v0.5.0

Phase 6/6.1 收口版：本地云渲染与模板市场基线、SQLite 服务、Docker Compose、真实
Dear ImGui/OpenGL 窗口、Taichi 1.7.4 CPU 烟雾/泼水验收，以及可执行远端同步门禁。

## 已知限制

- SAM/XMem、ProPainter、Depth Anything 缺少经许可锁定的固定 ONNX 导出与哈希，永久
  使用高对比/低遮挡素材白名单、逐帧提示分割 + IoU 关联、CPU 修复和启发式深度。
- 低对比、完全遮挡重现、快速形变、复杂纹理不作自动点选和修复质量承诺。
- 云渲染与模板市场用户入口默认关闭；无支付、对象存储托管或商业 SLA。
- 录屏/GIF 物料补充中，不影响本标签与基础 Release。

详细数据与商业影响见 `docs/perf-baseline.md`、`docs/model-cards.md` 和
`docs/business-model.md`。
