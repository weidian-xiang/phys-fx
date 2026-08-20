# Git 工程工作流

PhysFX 从 Phase 3 起采用 Conventional Commits、DCO 和轻量 Git Flow。历史提交
`ee6761e` 及此前的 Phase 2 提交保持原样；新提交从本阶段首个提交起执行本规范。

## 提交信息

格式为 `<type>(<scope>): <中文描述>`，首行不超过 50 个字符且不以句号结束。
`type` 只能是 `feat`、`fix`、`docs`、`refactor`、`test`、`perf`、`build`、`ci` 或
`chore`；scope 使用小写模块名。每条提交必须包含：

```text
Signed-off-by: 向伟典 <xwd752438081@163.com>
```

仓库提供 `.gitmessage` 模板和 `.githooks/commit-msg` 校验钩子。首次配置：

```powershell
git config commit.template .gitmessage
git config core.hooksPath .githooks
```

也可以使用 `git commit -s` 自动写入 DCO。校验器为无第三方依赖的
`tools/check_commit_msg.py`，CI 会对 PR 中每一条提交逐条运行。

## 分支与合入

`master` 是受保护的发布分支，禁止直接 push、force push 和删除；功能从最新
`master` 切出 `feature/<scope>-<简述>`，修复使用 `fix/<简述>`，文档使用
`docs/<简述>`，发布准备使用 `release/v<X.Y.Z>`。一个分支只对应一个 PR。

PR 必须通过构建、CTest、pytest、版权扫描、提交规范和 DCO 检查，并更新
`CHANGELOG.md` 的 `Unreleased` 段。合入使用 `git merge --no-ff`，不 squash，保留
逐条签署的提交历史；合入后删除远端工作分支。公开分支同步使用 `git merge master`，
不改写历史。

Gitee 生成的合并提交可能使用平台固定的 `Merge pull request ...` 首行且不附 DCO。
`tools/check_commit_msg.py --range` 会根据父提交数量识别并跳过这类平台合并提交，
但仍逐条严格校验合并前的普通提交；贡献者提交本身仍必须符合 Conventional Commits
并包含 DCO。

Gitee 后台保护分支路径：仓库设置 → 分支管理 → 分支保护 → 添加 `master`，限制
推送和强制推送权限，仅允许 PR 合入。当前单人维护期允许作者自审，但仍须完整勾选
PR 清单。

## Gitee 主仓与 GitHub 镜像

仓库采用单向镜像：Gitee（`https://gitee.com/xiang-weidian/phys-fx`）是唯一主仓，
GitHub（`https://github.com/weidian-xiang/phys-fx`）是只读镜像。Issue、PR、发布和
标签均在 Gitee 完成，禁止从 GitHub 反向开发、推送或创建标签。

本地远程约定为 `origin`=Gitee、`github`=GitHub 镜像；`github` 配置不可用的 push
URL，防止误推。镜像配置和同步后校验命令见
[`docs/mirror-sync.md`](mirror-sync.md)。

## 发布

版本遵循 SemVer。Phase 3 使用 `release/v0.2.0` 准备发布，合入后创建附注标签
`v0.2.0`，标签信息引用 CHANGELOG 摘要。`origin` 指向 Gitee，`github` 指向 GitHub
只读镜像；发布完成后由 Gitee 镜像任务同步标签。
