# 贡献指南（草案）

欢迎提交问题、设计建议和代码贡献。当前项目处于 Phase 2 架构整合期，提交前请注意：

1. 使用 C++20，类型采用 PascalCase，函数和变量采用 camelCase，命名空间使用 `physfx::`。
2. 新增接口必须包含中文 Doxygen 注释，并注明 `@brief`、参数、返回值和对应后续阶段 TODO。
3. 模块之间通过 `core` 数据结构和抽象接口通信，避免跨层直接依赖具体实现。
4. 代码格式遵循 `.clang-format`，文本文件遵循 `.editorconfig`。
5. 在本地运行 CMake 构建、`ctest` 和 Python 测试后再提交变更。

## DCO 签署

每个提交必须包含 `Signed-off-by` 行，例如：

```text
Signed-off-by: 向伟典 <你的 Gitee 绑定邮箱>
```

签署表示你有权提交该内容，并同意按 Apache-2.0 许可授予项目。可以使用
`git commit -s` 自动添加签署行。

CI/CD 留待 Phase 3 启用；提交前至少运行本地一键工作流和版权检查。
