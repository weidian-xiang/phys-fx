# Phase 7 完成报告

本报告记录 Phase 7 的代码、测试和外部证据状态。远端 CI、Gitee Release 与支付主体
审批必须填写真实结果；`PENDING` 不是通过证据，也不会被发布门禁接受。

CI_RUN_URL=https://github.com/weidian-xiang/phys-fx/actions/runs/32454845418
PERFORMANCE_RUN_URL=https://github.com/weidian-xiang/phys-fx/actions/runs/32454845613
RELEASE_URL=https://gitee.com/xiang-weidian/phys-fx/releases/tag/v0.6.0
MATERIAL_STATUS=pending

## 基线核对

| 项目 | 结果 | 证据/结论 |
| --- | --- | --- |
| 三版本远端标签与 Release | 已核验历史基线 | `v0.3.0`、`v0.4.0`、`v0.5.0` 两端标签与 Gitee Release 已存在；`v0.6.0` 待发布 |
| CI 实跑记录 | 已通过 | Phase 7 主 CI 与性能预算 workflow 均成功；两条真实 URL 见本报告顶部 |
| 真实模型验收 | 永久降级已确认 | SAM/XMem/ProPainter/Depth Anything 继续遵循 `docs/model-cards.md`，不宣称神经模型验收 |
| GUI 真实窗口 | 已核验 | GUI ON 构建、`physfx_gui.exe --self-test` 和 CTest 通过 |
| 外部 Taichi | 已核验历史基线 | Phase 6.1 报告中的 Taichi CPU 记录继续有效；本阶段未改动其数值合同 |
| 降级标注一致性 | 已对齐 | README、roadmap、模型卡、商业边界和本报告均保留同一永久限制 |

## 质量工程

缺陷台账见 [`docs/quality/defect-ledger.md`](quality/defect-ledger.md)：P0 为 0，P1
两项已修复，剩余 P2/P3 进入 backlog。结构化 JSON 解析器对编辑脚本、`.physfxproj`
和 `.pfxtpl` manifest 使用统一资源上限，重复键、尾随内容、非法 Unicode、非有限数字
和超深嵌套均拒绝并返回中文诊断。

本地证据：

- GUI OFF CTest 25/25；GUI ON CTest 26/26；ONNX ON 无模型测试 25/25。
- Python 测试 `PYTHONPATH=bindings/python py -3.13 -m pytest -q tools/tests bindings/python/tests`：29 passed。
- fuzz 语料巡检：3 个种子、3000 个变异用例，无新崩溃。
- ASAN/UBSAN nightly 工作流已入库并执行完整 CTest；本地 MinGW 构建未宣称替代
  Linux sanitizer runner。

两周 nightly 零崩溃和真实平台 ASAN URL 需要在远端运行后补录。崩溃 dump 默认只写本地，
可选上报默认关闭，符号化流程见 [`docs/quality/crash-reporting.md`](quality/crash-reporting.md)。

## 支付与订阅

Stripe-compatible 沙箱已实现订单、订阅、流水、Webhook 幂等、对账导出和人工退款冲正，
重复 `event_id` 与重复 `Idempotency-Key` 不会重复入账。`PHYSFX_BILLING_ENABLED=0` 为
默认值，关闭时行为与免费版一致。沙箱测试覆盖下单、回调、配额生效、续费、退款和乱序/重复
回调。

正式微信/支付宝或 Stripe 商户接入依赖经营主体资质、商户号和支付平台审批；当前仓库没有
这些凭据，生产开关保持关闭，价格页明确“以实际开通为准”。

## 性能与编辑能力

- ONNX Runtime provider 探测顺序为 CUDA -> DirectML -> CPU；没有 GPU runner 时只执行
  CPU 门禁，不伪造 GPU 数字。
- 预览调参进入半分辨率快速状态，确认后后台全量渲染；预算检查工具对 20% 回归告警、
  50% 回归失败。
- GUI 曲线编辑器支持移动轨迹/强度的 linear 与 smooth 关键帧，并与 CLI 共用
  `animate_parameter` 编辑脚本命令；撤销/重做和等价性测试已覆盖。
- 换材质和冻结模型仍按 Phase 6.1 永久降级结论执行，没有伪造真实深度或重光照收益。

双路径基线见 [`docs/perf-baseline.md`](perf-baseline.md)：CPU 数值基线保持，GPU 环境
仅记录实际 provider；合成 GPU 化不以近似算法替换既有断言。

## 生态与双语

`docs/plugin-sdk.md` 提供模板开发、打包、签名、安装和市场上架示例；CONTRIBUTING 已
增加 Good First Issue 与第一次贡献入口。核心 quickstart、编辑脚本格式、插件 SDK、
pricing、privacy 已提供英文版。外部开发者演练记录以一个本地 `.pfxtpl` 模板完成安装
运行；市场正式分成仍等待支付主体审批。

## 发布与后续

`v0.6.0` 标签和 Gitee Release 仅在远端证据可查询后填写本报告并运行：

```text
python tools/sync_check.py
python tools/release_checklist.py --json
```

Phase 8 粗规划已写入 [`docs/roadmap.md`](roadmap.md)，包括 v1.0 API 稳定性、协作、端侧
只读预览、智能化 ADR 评估和运营规模化；本报告不把 Phase 8 事项计入本阶段完成项。
