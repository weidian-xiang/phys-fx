# Phase 9 烟雾静止诊断

诊断时间：2026-08-22（Asia/Shanghai）

## 结论

宠物成片的烟雾不是 `physfx smoke`、C++ `TaichiFluidSimulator`、外部 Taichi，
也不是 `tools/taichi_bridge/smoke.py` 的输出。Phase 9 首轮脚本
`tools/phase9_real_pipeline.py` 直接在 `make_smoke()` 中用 NumPy 高斯透明层合成：

- 发射点写死为原始画面的 `width * 0.50`、`height * 0.55`，没有参数入口；
- 密度场没有逐帧状态、速度场或外部模拟器返回值；
- 只有极小的正弦横向偏移和线性纵向偏移，不能证明流体演化；
- 遮挡使用 XMem 掩码做像素透明度相乘，但没有逐帧像素断言或回归门禁。

因此维护者观察到的“烟雾静止、没有扩散、没有与狗交互”是实现缺陷，不是素材或
肉眼误判。

## 参数链核验

`apps/cli/main.cpp` 会解析 `--anchor x,y` 写入 `Config.editTarget`，
`pipeline/src/StageFactory.cpp` 再把它写入 `SimConfig.emitterPosition`，
`TaichiFluidSimulator::simulate()` 会按帧读取该配置。但这条 C++ 管线没有被
`phase9_real_pipeline.py` 调用，故该参数链没有进入首轮成片。

`tools/taichi_bridge/smoke.py` 的旧实现也把 `source_x` 固定为网格中心，未接受外部
锚点；当前 Python 3.13 环境没有安装 Taichi，所以真实调用该桥时应如实标记
`backend=cpu-fallback`。首轮成片既没有 `taichi-cpu`，也没有 `cpu-fallback` 标记，
实际路径应记录为 `phase9-python-gaussian`，并视为不合格路径。

## 修复边界

修复只覆盖 Phase 9 任务②所需的烟雾模拟、锚点传递、逐帧遮挡和回归验证：

1. 模拟器逐帧返回密度场，并记录 `backend`、配置锚点和实际发射锚点；
2. 锚点从 Phase 9 管线配置传入模拟器，不再写死；
3. 用 ffmpeg 抽帧检查特效差异、锚点和实体掩码遮挡；
4. 将同一检查接入 CTest，缺少输出或掩码时失败，不默认跳过。

人物去物和车辆移动的残影、棋盘伪影、贴图感仍保持失败基线，另行按五轮上限继续排查，
不会用烟雾修复结果覆盖它们的失败结论。

## 二次诊断：目标掩码通道读取错误

在接通逐帧密度场与 alpha 断言后，宠物成片仍没有可见烟雾。检查保存的真实 XMem
掩码发现：第 72 帧的实体覆盖率为 87.30%，第 180 帧为 100.00%，合成器因此正确地将
整个烟雾 alpha 归零。此前的相邻帧 IoU 只证明这个错误的背景掩码稳定，不能证明跟踪到狗。

根因在 `tools/phase9_real_pipeline.py` 的 `xmem_masks()`：XMem
`InferenceCore.step()` 返回的第 0 通道是背景概率、第 1 通道才是单对象概率，首轮代码
错误读取了 `predicted[0]`。SAM 在相同种子点上给出的候选对象面积为 9.41%~14.01%，并非
整幅画面，故问题不是种子点或素材本身。

后续修复必须读取对象通道，并在真实管线中记录每帧覆盖率；种子或时序掩码超过画面
60% 时必须失败，不能再以高 IoU 放行。修复后需重新跑三段真实权重案例，再执行烟雾
alpha、锚点和遮挡断言。此前所有三段的视觉不合格结论仍然有效。
