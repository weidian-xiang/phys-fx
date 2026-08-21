# PhysFX v1.0 功能冻结清单

状态：**待维护者确认**
候选基线：`6cdc081` 之后的 Phase 8 冻结提交
确认记录：维护者尚未确认；在本行改为“已确认”并记录日期/提交前，任务③、④、⑤不得动工。

本清单是 v1.0 的唯一范围来源。源码位置表示承诺的实现或门禁所有者，不代表尚未完成的
外部发布、公示或审批已经发生。

## IN：v1.0 承诺面

| 能力 | v1.0 承诺 | 源码/合同位置 |
| --- | --- | --- |
| CLI | `run`、`remove`、`move`、`appearance`、`smoke`、`splash`、`explode`、`season`、`cloud render`、`market search/install` 保持可用；`appearance` 是 0.x 已有入口，因此纳入冻结而不是静默删除 | `apps/cli/main.cpp`、`pipeline/`、`editing/` |
| GUI | 预览、点选、右键编辑、撤销/重做命令栈、线性节点图、linear/smooth 曲线编辑、中文/英文核心界面 | `apps/gui/src/`、`docs/gui.md` |
| 编辑脚本 JSON | `version=1`、`commands` 和 1.0 已有命令；1.x 只新增可选字段或新命令类型 | `editing/src/EditScript.cpp`、`docs/edit-script-format.md` |
| `.physfxproj` | `version=1` 的输入/输出、节点、命令和可选曲线；0.4-0.6 工程可读 | `apps/gui/src/Project.cpp`、`apps/gui/src/Project.h` |
| `.pfxtpl` | ZIP 容器、manifest、可选 Ed25519 签名；未签名包可加载但必须标识未认证来源 | `tools/pack_template.py`、`plugins/src/TemplatePackageVerifier.cpp` |
| 公共 C++ API | 各模块 `include/physfx/` 下的头文件、`PHYSFX_API` 可见性入口和 `physfx::v1` 稳定命名入口遵循 SemVer | 各模块 `include/`、`core/include/physfx/core/Export.h`、`docs/api-audit.md` |
| 插件 SDK | effect/edit template 接口、注册表、模板打包/签名/验签和未签名包完整可用原则 | `plugins/include/`、`docs/plugin-sdk.md` |
| 本地降级 | 无模型、无 FFmpeg、无 ONNX、无 Taichi、无 GPU、云服务关闭时的既有显式回退；限制必须在 README、Release、完成报告三处一致 | `docs/model-cards.md`、`docs/business-model.md`、各工厂实现 |
| 云渲染与市场 | 账号/API Key、队列、配额、模板索引和本地参考服务；用户入口继续受明确功能开关控制 | `cloud/`、`docs/pricing.md` |
| 工程同步与只读分享 | 私有 `.physfxproj` 上传、最近 N 个快照、可过期只读链接、另一环境拉取；不含实时协同 | `cloud/server/workspace/`（冻结后任务⑤实现） |
| 团队档 | 席位模型和共享配额逻辑入库；仅在生产支付审批通过且开关开启时生效 | `cloud/server/`（冻结后任务⑤实现）、`docs/pricing.md` |
| Web 只读查看器 | 成片播放和工程元信息/结构树预览，页面固定声明“不支持浏览器内编辑” | `apps/web-viewer/`（冻结后任务⑤实现） |
| v1.0 运营内容 | 2-3 个冻结范围内模板、案例库、三篇教程、作者申请/审核/签发/结算 SLA 和 opt-in 计数 | `plugins/templates/`、`docs/showcase.md`、`docs/tutorials/` |

## OUT：v1.0 不承诺

| 能力 | v1.0 结论 | 归档位置 |
| --- | --- | --- |
| 扩散式生成编辑/视频重渲染 | 不实现；Phase 8 只交 ADR-0024 研究结论和成本估算，spike 不进 `master` | `docs/research/`、`docs/decisions/0024-*` |
| 实时多人协同 | 不实现 OT/CRDT、在线光标、冲突合并或浏览器内共同编辑 | Phase 9 backlog |
| 移动端原生应用 | 不实现 iOS/Android 原生应用或移动端编辑 | Phase 9 backlog |
| 3DGS 自由视角 | 终局不做；继续使用 2.5D/分层路径，不重开评估 | `docs/decisions/0020-3DGS方向终局.md` |
| 换材质完整版 | 不承诺真实深度、重光照或神经材质替换；保留现有颜色/外观降级并永久标注 | `docs/model-cards.md`、`README.md` |
| 浏览器内编辑 | 查看器只读，不提供命令修改、保存或伪装成编辑器的控件/文案 | `apps/web-viewer/` footer 合同 |
| 生产支付 | 审批前不启用，不把沙箱称为生产收款 | `docs/pricing.md`、`docs/decisions/0021-支付渠道选型.md` |
| 2.0 时间表 | 不规划、不承诺 | 无 |

## 变更控制

1. 确认后，IN 只能做缺陷修复、兼容实现和完成表内闭环；新增用户能力进入 Phase 9 backlog。
2. RC 期间 `master` 只接受缺陷修复、测试、文档与发布物料变更。
3. RC 最多三轮；第三轮仍有 P1 时，依次裁剪 Web 查看器、团队档、新模板数量。冻结清单、P1=0 和至少一周公示不可裁剪。
4. 永久已知限制必须有维护者书面确认，并在 README、Release、完成报告三处一致。
5. 对本清单的确认只确认范围，不等于确认尚未发生的 RC 公示、用户反馈、支付审批或外部发布。
