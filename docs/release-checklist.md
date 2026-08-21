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
5. Phase 8 起 Gitee/GitHub 双 Release 均已创建并写入完成报告，且
   `MATERIAL_STATUS=ready`；`pending` 只允许存在于开发过程记录，不能通过完成门禁。
6. CI 最新运行全绿；运行 URL 写入完成报告。
7. `roadmap.md` 已更新并包含下一阶段粗规划。
8. Release 正文、完成报告和 README 的降级项一致。

任一项失败都不得宣告阶段完成。远端不可访问时应保留失败证据，而不是删除检查项或以本地日志替代远端证据。

代码推送仍与发布解耦：物料或 Release 红灯不阻止已验证代码同步到主仓，但会阻止 RC/正式版
完成声明。Phase 8 的 RC 还必须有独立附注标签、双平台预发布记录和至少一周公示证据。
