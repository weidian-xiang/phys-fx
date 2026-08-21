# 模型卡片与终局限制

Phase 9 已将四个具体权重变体、官方下载地址、上游 revision、文件大小、SHA-256、格式、
张量合同和许可证写入 `docs/model-lock.json`。`python tools/download_models.py --lock`
只接受锁文件中的记录，并在使用前后同时校验文件大小与 SHA-256。GitHub Release 资产使用
官方 API 下载端点，以避免普通 Release 直链在部分网络环境中超时。

| 模型角色 | 锁定变体 | 许可证/格式 | 默认路径 |
| --- | --- | --- | --- |
| 首帧分割 | `facebookresearch/segment-anything` SAM ViT-B | Apache-2.0 / PyTorch checkpoint | 待真实推理适配完成；不存在时才允许传统回退 |
| 时序跟踪 | `hkchengrex/XMem` 固定 Release asset `70788989` | MIT / PyTorch checkpoint | 待真实推理适配完成；不存在时才允许传统回退 |
| 视频修复 | `sczhou/ProPainter` 固定 Release asset `124499582` | S-Lab License 1.0（非商业）/ PyTorch checkpoint | 待真实推理适配完成；不存在时才允许 CPU 回退 |
| 深度 | `isl-org/MiDaS` DPT Swin2 Tiny 256 | MIT / PyTorch state_dict checkpoint | 待真实推理适配完成；不存在时才允许启发式回退 |

四个权重已完成供应链锁定和本地文件校验，但这不等于推理链已验收。当前 C++
`OnnxSegmenter`/`OnnxTracker` 只完成运行时装载，仍会明确返回 `notImplemented`；在适配
输入输出合同并完成真实视频质量验收前，不得把模型加载或传统回退结果宣传为神经模型效果。
ProPainter 的许可证禁止商业使用，未经维护者和上游许可证审查不得作为商业默认能力。

## 白名单验收边界

逐帧提示分割 + IoU 关联仅支持主体与背景颜色差明显、遮挡较少、帧间位移连续的素材。
Phase 6.1 在三类高对比合成材质上运行 1280×720、30 fps、10 秒，共 900 帧：最小
IoU 1.0，最慢 6.90 ms/帧，轨迹未中断。低对比、完全遮挡后重现、快速形变和复杂纹理不在
质量承诺内；CPU 修复可能出现拖影，启发式深度只保证相对排序。

## 商业影响

云渲染和模板市场入口继续默认关闭，不能销售“任意素材自动点选”“神经视频修复”或
“真实单目深度”质量。可商业化范围仅限本地引擎支持、明确白名单素材和人工确认工作流；
定价、SLA 与宣传材料必须复用本页限制。
