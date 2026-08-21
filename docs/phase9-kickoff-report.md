# Phase 9 开工与基线核对报告

核对时间：2026-08-21（Asia/Shanghai）
核对提交：`ca619ec18362afc210600e967ffc5c36154825a6`

本报告只记录可复核事实。外部编辑、素材授权、模型锁定、肉眼验收、公示和发布没有实际
证据时一律不写成完成。

## 最高指导原则

**问题一：什么时候能看到实际的效果？**

答：**本阶段结束时。** 之前看不到的根本原因不是工程量，而是排序错误——每个阶段都把
基建排在效果前面，效果被无限后移。本阶段把顺序彻底反转：真实效果是任务②，也是唯一
主线；所有适配器、管线、模型卡片、下载脚本在 Phase 3~8 早已就绪，剩下的是“锁定权重
→ 接入 → 真机验收 → 录制上传”这条窄路，无并行任务分摊带宽。

**问题二：什么时候真正基本核心完工？**

答：**满足 Phase 9“核心完工判据”五条之时，即本阶段验收之日。** 判据的核心是“维护者
本人肉眼验收真实素材演示”——效果的最终裁判是用户的眼睛，不是测试套件。之后的
Phase 10+ 全部是增长与演进，不再是还债。

## 证据链

| 项目 | 结果 | 可复核证据 |
| --- | --- | --- |
| 工作区与镜像 | 通过 | `python tools/sync_check.py --json` 全绿；本地 `HEAD`、Gitee `master`、GitHub `master` 均为 `ca619ec` |
| 当前主 CI | 通过 | [PhysFX CI run 32460733895](https://github.com/weidian-xiang/phys-fx/actions/runs/32460733895)，`success`，提交 `ca619ec` |
| 当前性能预算 CI | 通过 | [PhysFX Performance Budget run 32460733820](https://github.com/weidian-xiang/phys-fx/actions/runs/32460733820)，`success`，提交 `ca619ec` |
| `v0.6.0` 附注标签 | 通过 | 两个远端均存在标签对象 `691cbcf`，剥离后提交为 `d90e27f` |
| Gitee `v0.6.0` Release | 已修 | [Release](https://gitee.com/xiang-weidian/phys-fx/releases/tag/v0.6.0) 正文已更正并经公开 API 核验为剥离提交 `d90e27f` |
| GitHub `v0.6.0` Release | 缺失 | GitHub Releases API 对 `v0.6.0` 返回 404；标签存在不等于 Release 存在 |
| `v1.0.0` / RC | 未发布 | 两端无对应 Git 标签；GitHub Releases API 返回 404，Gitee Release API 返回 `null` |
| `release/1.x` | 未建立 | Gitee 与 GitHub 均无该远端分支 |
| 发布物料门禁 | 阻断 | `python tools/release_checklist.py --json` 因 `MATERIAL_STATUS=pending` 退出 1 |
| 支付审批 | 未通过 | 仓库没有可核验的经营主体或商户审批证据；`PHYSFX_BILLING_ENABLED=0` 必须保持默认关闭 |

## Phase 8 遗留清剿状态

Gitee `v0.6.0` 正文更正已完成并经公开 API 核验；GitHub `v0.6.0` Release 补建仍待认证的
GitHub 页面操作。物料逐项状态已刷新到 `docs/release-assets.md`，未上传项仍归入 Phase 9
任务③，且仍是 RC 前阻断项。

生产支付继续处于“技术就绪、审批待命”状态。没有维护者提供的书面审批证据，不执行开关
动作，也不把沙箱能力表述为生产收款。

## 任务②入口核对（已启动）

维护者已提供三段本地输入，任务②可以开始执行技术管线验证；它们的事实登记见
`assets/demo-footage/README.md`。三段文件实际均为 `1280x720`、`8.12 s`、`24 fps`、
H.264/AAC，且包含 `AIGC Label=1` 元数据，来源为 HappyHorse-1.1-T2V。因此它们不是实拍，
不能单独满足 Phase 9 的真实素材核心完工判据，也不能冒充发布证据。

任务②仍有两个验收边界：

1. `docs/model-lock.json` 已锁定 SAM ViT-B、XMem、ProPainter 和 MiDaS DPT Swin2 Tiny 256 的
   官方 URL、revision、文件大小、SHA-256、格式、许可证和张量合同；四个权重已在本地由
   `tools/download_models.py --lock` 下载并校验。锁定与可载入不等于推理链验收，真实视频
   仍必须通过任务②质量门槛。
2. 当前没有一组覆盖人物/车辆/宠物或运动物体的三条真实实拍素材。当前 AI 生成输入仅用于
   本地技术验证；其生成提示词未随素材提供，项目不自行推断。

若维护者决定执行终局白名单条款，也必须书面确认适用素材边界；白名单内仍需授权实拍素材、
真实命令产出、质量数据和肉眼验收，不能使用合成素材替代。

## 单线程结论

Phase 9 已进入任务②。GitHub `v0.6.0` Release 仍未补建，但不阻塞本地真实权重管线验证；
任务②退出前不制作发布演示，不创建 RC，不建立正式版 Release。智能化、实时协作、国际化、
新模板及其他新能力一律不启动，只记入 Phase 10 backlog。

## 同步门禁输出

```text
[PASS] 工作区：干净
[PASS] 主仓提交：HEAD=ca619ec18362 与 origin/master 一致
[PASS] 标签：本地 5 个标签均存在于 origin、github
[PASS] 镜像：github/master 与主仓同为 ca619ec18362
```
