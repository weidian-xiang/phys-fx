<!--
 PhysFX Engine —— 视频世界编辑器（Video World Editor）
 Copyright (c) 2026 向伟典
 SPDX-License-Identifier: Apache-2.0
-->

# Gitee 到 GitHub 镜像同步规范

## 仓库角色

PhysFX 采用单向镜像模型：Gitee 是唯一主仓库，GitHub 是只读镜像。

| 角色 | 地址 | 约定 |
| --- | --- | --- |
| 主仓库 | [Gitee](https://gitee.com/xiang-weidian/phys-fx) | 代码、Issue、PR、发布和保护分支的权威来源 |
| 镜像仓库 | [GitHub](https://github.com/weidian-xiang/phys-fx) | 用于公开可见性、跨平台拉取和镜像后的 Actions 验证 |

GitHub 不承担开发主仓、发布主仓或问题追踪主仓的职责。贡献者应在 Gitee
提交 Issue/PR；GitHub 上的讨论和 PR 不作为合并依据。阶段门禁要求验证
`pull_request` 事件时，可以为已从 Gitee 单向镜像的同名分支创建临时 GitHub PR，
但它只用于触发 Actions：不得在 GitHub 合并、修改代码或创建发布，验证完成后关闭。
对应改动仍须通过 Gitee PR 或维护者直推进入 `master`。

## Gitee 镜像配置

在 Gitee 仓库管理页完成一次配置：

1. 打开 `仓库设置 → 仓库镜像管理`。
2. 新增推送镜像，目标地址填写
   `https://github.com/weidian-xiang/phys-fx.git`。
3. 认证使用 Gitee 页面提供的安全凭据配置，不把 Token、密码或私钥写入仓库。
4. Gitee 会同步代码提交、分支和标签；临时工作分支仍需按主仓工作流及时清理，
   不在 GitHub 单独维护同名分支。
5. 保存后执行一次手动更新，并在 GitHub 检查 `master`、分支和 `v*` 标签是否可见。

镜像只允许从 Gitee 推送到 GitHub，禁止配置 GitHub→Gitee 的反向镜像，避免分支、
标签和发布记录形成循环或冲突。

## 本地远程命名

本地克隆建议保持以下命名：

```powershell
git remote add github https://github.com/weidian-xiang/phys-fx.git
git remote set-url --push github DISABLED
git config --local remote.pushDefault origin
git config --local push.default simple
git config --local remote.origin.prune true
git config --local remote.github.prune true
git fetch --prune --tags origin
git fetch --prune --tags github
```

`origin` 必须指向 Gitee；`github` 只用于读取和比对。`DISABLED` push URL 会使误执行
`git push github` 立即失败，`remote.pushDefault=origin` 则保证未显式指定远程时仍推向
Gitee。外部贡献分支从最新 Gitee `master` 创建，PR 只在 Gitee 发起；仓库所有者/主维护者
的本人代码可以按 `docs/git-workflow.md` 直接推送 Gitee `master`，再由镜像任务同步。

## 同步后校验

镜像完成后，在本地执行：

```powershell
git fetch --prune --tags origin github
git rev-parse origin/master
git rev-parse github/master
git diff --exit-code origin/master github/master
git tag --list "v*" --sort=-version:refname | Select-Object -First 5
```

两个 `rev-parse` 应输出同一个提交；若存在差异，先暂停发布和后续镜像，确认 Gitee
是否仍有未完成的推送。标签以 Gitee 为准，不在 GitHub 单独创建或移动标签。

## 故障处理

- 镜像失败：在 Gitee 镜像管理页重试，记录失败时间和平台提示，不手工改写 GitHub 历史。
- GitHub 出现额外提交：立即停止同步，保留证据并删除未经 Gitee 审核的提交；不得用
  force push 覆盖历史。
- 凭据疑似泄露：在 Gitee/GitHub 立即撤销并重新生成凭据，检查仓库历史，禁止把新凭据
  写入脚本、工作流或文档。
- 需要暂停镜像：暂停 Gitee 推送镜像任务，完成修复后先在临时分支验证，再恢复
  `master` 与标签同步。

## 发布和 CI 边界

发布标签、Release 说明和 CHANGELOG 先在 Gitee 完成，再由镜像带到 GitHub。Gitee Go
是主流水线；`.github/workflows/ci.yml` 仅作为镜像仓可运行的等价质量检查，不执行推送、
反向同步或发布操作。
