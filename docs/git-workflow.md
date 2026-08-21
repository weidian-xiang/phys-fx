# Git 工程工作流

## 代码推送与版本发布永久解耦

| | 代码推送（PUSH） | 版本发布（RELEASE） |
| --- | --- | --- |
| 频率 | 每个工作日结束必做 | 阶段里程碑 |
| 门槛 | 无（本地测试绿即可） | 检查单全绿 |
| 性质 | 备份与协作基础 | 对外承诺 |
| 谁批准 | 不需要任何人 | 检查单脚本 + 维护者 |

- `master` 应始终反映最新已验证基线。未完成能力以功能开关或诚实的降级提示留在
  `master`，不构成延迟推送的理由。
- Release 可以分两步完成：`vX.Y.Z` 附注标签和基础正文先行，录屏、GIF、性能数据等
  物料后补编辑。物料缺失只阻塞完整版正文，永不阻塞代码推送或标签。
- 远端分支就是备份；WIP 分支也必须推送，本地独家代码存活不得超过一个工作日。
- 每个工作日结束前运行 `python tools/sync_check.py`。退出码非零表示工作尚未完成收尾，
  不得以“Release 尚未准备好”为由忽略。

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

`master` 是受保护的发布分支，采用维护者与外部贡献者双轨权限：

- 仓库所有者、主维护者向伟典可以把本人代码直接提交并推送到 `master`，不需要为自己的
  提交创建 PR 或执行形式化自审；推送前仍须完成与改动风险相匹配的本地测试、版权扫描、
  提交规范和 DCO 签署。
- 其他贡献者不得直接推送 `master`。功能从最新 `master` 切出
  `feature/<scope>-<简述>`，修复使用 `fix/<简述>`，文档使用 `docs/<简述>`，每个分支
  对应一个 Gitee PR，并完成审查与测试后合入。
- 任何人都不得 force push 或删除 `master`。

外部贡献 PR 必须通过构建、CTest、pytest、版权扫描、提交规范和 DCO 检查，并更新
`CHANGELOG.md` 的 `Unreleased` 段。合入使用 `git merge --no-ff`，不 squash，保留
逐条签署的提交历史；合入后删除远端工作分支。公开分支同步使用 `git merge master`，
不改写历史。维护者也可以主动选择分支和 PR 处理高风险或需要公开讨论的变更，但这不是
维护者本人提交的强制门槛。

Gitee 生成的合并提交可能使用平台固定的 `Merge pull request ...` 首行且不附 DCO。
`tools/check_commit_msg.py --range` 会根据父提交数量识别并跳过这类平台合并提交，
但仍逐条严格校验合并前的普通提交；贡献者提交本身仍必须符合 Conventional Commits
并包含 DCO。

Gitee 后台保护分支路径：仓库设置 → 分支管理 → 分支保护 → 添加 `master`。推送权限
只授予仓库所有者/主维护者向伟典，其他成员无直推权限；合并权限授予维护者，强制推送和
删除分支保持禁止。PR 的审查与测试门禁用于外部贡献，不要求维护者为本人代码自建 PR。

Phase 5 CI 结论见 [ADR-0015](decisions/0015-CI平台终局.md)：GitHub Actions 负责镜像
后的质量验证，Gitee 仍负责主仓发布、PR、标签和 Release。两边均禁止通过 CI 写回代码、
标签或远程配置。

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

Release 的完整物料仍需按 `docs/release-checklist.md` 核验，但物料暂缺时应在基础正文中
明确写明“物料补充中”，不得把它扩大解释为阻塞标签或代码推送。
