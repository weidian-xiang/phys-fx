# Phase 8 开工与基线核对报告

核对时间：2026-08-21（Asia/Shanghai）  
核对提交：`6cdc0812cb133d6d298bd10dd9b84de4aefd3e3c`

本报告只记录可复核事实。外部上传、审批、公示和反馈没有实际证据时一律不写成完成。

## 证据链

| 项目 | 结果 | 可复核证据 |
| --- | --- | --- |
| 工作区与主仓 | 通过 | 开工时工作区干净；`HEAD`、Gitee `master`、GitHub `master` 均为 `6cdc081` |
| `v0.6.0` 附注标签 | 通过 | 两个远端均存在标签对象 `691cbcf`，剥离后提交为 `d90e27f` |
| GitHub 主 CI | 通过 | [run 32456492522](https://github.com/weidian-xiang/phys-fx/actions/runs/32456492522)，`success`，提交 `6cdc081` |
| GitHub 性能预算 | 通过 | [run 32456492513](https://github.com/weidian-xiang/phys-fx/actions/runs/32456492513)，`success`，提交 `6cdc081` |
| Gitee `v0.6.0` Release | 已存在但正文待修 | [PhysFX Engine v0.6.0](https://gitee.com/xiang-weidian/phys-fx/releases/tag/v0.6.0)，只有源码 zip/tar.gz；正文写“标签提交 `864ad61`”，实际标签剥离后为 `d90e27f` |
| GitHub `v0.6.0` Release | 缺失 | GitHub Releases API 对 `v0.6.0` 返回 404；Git 标签存在不等于 Release 存在 |
| Phase 7 演示物料 | 未清零 | `docs/phase7-completion-report.md` 仍为 `MATERIAL_STATUS=pending`，Gitee Release 未发现独立 demo 资产 |
| 降级标注 | 通过 | README、roadmap、model cards、business model、Phase 7 报告均保留真实模型永久降级和支付默认关闭说明 |
| 同步门禁 | 通过 | `python tools/sync_check.py`：工作区、Gitee 主仓、5 个本地标签、GitHub 镜像均为绿色 |

## 遗留清剿状态

演示物料不能仅凭源码归档认定完成。制作台账见 `docs/release-assets.md`。Phase 7 和 v1.0 合并物料至少需要 before/after
演示、操作脚本、字幕/封面及校验值，并分别附可访问的 Gitee/GitHub Release 或传播平台
链接。当前没有这些上传证据，因此保持 `pending`，并作为 v1.0 RC 前的阻断项。

生产支付状态保持“技术就绪、审批待命”：仓库未包含经营主体、商户号或生产凭据，
`PHYSFX_BILLING_ENABLED=0` 继续是默认值。维护者提供书面审批证据前，不得开启生产支付，
且该外部依赖不阻塞本地开源能力或其他 Phase 8 工程任务。

## 冻结前结论

基线代码、标签、主 CI、性能 CI 和镜像提交可复核；双平台 Release 与演示物料尚未满足
v1.0 标准。任务②的冻结清单和兼容政策入库后，必须由维护者明确确认，才能开始任务③至⑤。
