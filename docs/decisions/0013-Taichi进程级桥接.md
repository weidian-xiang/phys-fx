# ADR-0013：Taichi 进程级桥接

## 状态

已接受（Phase 4）。

## 决策

Taichi 通过独立 Python 进程运行，不进入 C++ 构建链。C++ 侧保留
`TaichiFluidSimulator` 接口；无 Taichi 环境时返回可诊断的 `kNotImplemented`，其他
粒子路径继续可用。桥脚本位于 `tools/taichi_bridge/`，输入输出协议使用版本化 JSON。
