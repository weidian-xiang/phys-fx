# PhysFX Engine v0.3.0

编辑能力 CPU 基线版：命令栈、版本化脚本、删除/移动/复制/外观替换、CPU 视频修复、
分层合成、Taichi 进程桥接口与 `remove`/`move`/`appearance`/`smoke` CLI。

## 已知限制

- SAM/XMem 没有真实张量推理，使用传统提示分割和质心跟踪。
- 视频修复是 CPU 基线，不代表 ProPainter/E2FGVI 质量。
- Taichi 仅有进程桥接口；无外部运行时数据。
- 演示录屏和完整性能物料补充中。
