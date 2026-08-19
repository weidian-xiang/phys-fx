# Taichi 适配器占位

Phase 1 只保留目录边界，不引入 Taichi 运行时。Phase 2 将在此处实现
`IPhysicsSimulator` 适配器，并通过依赖注入替换 `MockSimulator`。
