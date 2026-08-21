# 阶段收尾检查单

每个 Phase 的完成报告必须附 `python tools/release_checklist.py --json` 输出。脚本只检查
本地 Git、远端和文档状态，不会推送、打标签或创建 Release。

门禁项目：

0. `tools/sync_check.py` 全绿：工作区、Gitee 主仓、全部本地标签和 GitHub 镜像同步。
1. 工作区干净，`origin/master` 包含当前 HEAD。
2. CMake、Python 包和 README 版本一致。
3. `CHANGELOG.md` 没有 `Unreleased` 残留。
4. 附注标签已推送且位于当前发布历史中（Phase 6 为 `v0.5.0`）。Release 创建后允许
   追加一笔只记录实际 URL 的证据提交，标签无需也不得为此移动。
5. Gitee Release 已创建并写入完成报告；演示物料允许标记“物料补充中”后续编辑，
   不阻塞基础 Release、标签和代码推送。
6. CI 最新运行全绿；运行 URL 写入完成报告。
7. `roadmap.md` 已更新并包含下一阶段粗规划。
8. Release 正文、完成报告和 README 的降级项一致。

任一项失败都不得宣告阶段完成。远端不可访问时应保留失败证据，而不是删除检查项或以本地日志替代远端证据。
