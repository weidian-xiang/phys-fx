# 贡献指南

欢迎提交问题、设计建议和代码贡献。当前项目进入 Phase 3，完整的提交、分支、PR
和发布规则见 [`docs/git-workflow.md`](docs/git-workflow.md)。

1. 使用 C++20，类型采用 PascalCase，函数和变量采用 camelCase，命名空间使用 `physfx::`。
2. 新增接口必须包含中文 Doxygen 注释，并注明 `@brief`、参数、返回值和对应后续阶段 TODO。
3. 模块之间通过 `core` 数据结构和抽象接口通信，避免跨层直接依赖具体实现。
4. 代码格式遵循 `.clang-format`，文本文件遵循 `.editorconfig`。
5. 在本地运行 CMake 构建、`ctest` 和 Python 测试后再提交变更。

## DCO 签署

每个提交必须包含 `Signed-off-by` 行，例如：

```text
Signed-off-by: 向伟典 <xwd752438081@163.com>
```

签署表示你有权提交该内容，并同意按 Apache-2.0 许可授予项目。可以使用
`git commit -s` 自动添加签署行。

提交前请运行本地构建、CTest、pytest、版权扫描和提交信息校验；每个 PR 必须更新
`CHANGELOG.md` 的 `Unreleased` 段。外部贡献者不得直接推送 `master`，必须通过 Gitee
PR 接受审查和测试。仓库所有者/主维护者向伟典可将本人代码直接推送到 `master`，无需
自建 PR，但仍须遵守测试、提交规范和 DCO 要求。

每个工作日结束前还必须运行 `python tools/sync_check.py`，确认工作区、Gitee `master`、
本地标签和 GitHub 镜像全部同步。代码推送是日常备份动作，不受 Release 物料进度影响。
