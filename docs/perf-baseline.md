# Phase 4 性能基线

基线目标：720p、10 秒视频、CPU 掩码传播单帧低于 500 ms，全流程峰值内存低于
4 GB。2026-08-20 在 Windows、MinGW Debug 配置下完成传统语义降级管线实测：

- CPU：Intel Core i7-14650HX（16 核 24 线程）；内存：31.73 GiB；GCC 14.2.0。
- 输入：FFmpeg `testsrc2`，1280×720、30 fps、10 秒、300 帧。
- 输出：H.264/MP4，1280×720、30 fps、300/300 帧可解码。
- 全流程墙钟 49.53 秒，平均 165.10 ms/帧；峰值工作集 827.04 MiB。

| 阶段 | P50 | P95 | 最大值 |
| --- | ---: | ---: | ---: |
| perception 桩 | 0.01 ms | 0.02 ms | 0.18 ms |
| traditional semantics | 157.94 ms | 245.20 ms | 406.26 ms |
| editing 空命令 | 0.01 ms | 0.01 ms | 0.05 ms |
| CPU particles | 0.02 ms | 0.03 ms | 0.17 ms |
| sprite compositing | 5.93 ms | 9.78 ms | 10.97 ms |

因此传统降级路径满足 `< 500 ms/帧` 与 `< 4 GB` 门槛。以上数据来自真实解码、
连通分割、质心跟踪、粒子遮挡合成和编码，不是桩耗时。

ONNX Runtime 1.18.1 已通过 MinGW ON 构建，但四个冻结目标没有合格的固定 ONNX 导出、
哈希和许可结论；上述数据不代表神经模型性能，具体终局限制见 `docs/model-cards.md`。

Phase 4 编辑基线：命令栈与 CPU 掩码修复在 1280×720 合成帧上的状态/像素测试已纳入
CTest。

## Phase 5 增量基线

默认无权重路径新增确定性的深度、天空和光照估计器，以及低分辨率烟雾密度场（默认
96×54）。它们用于 GUI 预览和模板交互，不宣称替代 Depth-Anything/SAM/XMem 或
ProPainter 权重。

## Phase 6.1 终局基线

- 模型降级验收：Intel Core i7-14650HX、MinGW Debug，三类高对比材质，每类
  1280×720/30 fps/10 秒，共 900 帧；逐帧提示分割 + IoU 关联最小 IoU 1.0，最慢
  6.90 ms/帧，900/900 帧轨迹连续。
- GUI：GLFW 3.4、Dear ImGui 1.90.9、OpenGL 3.0，1280×720 预览连续更新 120 帧，
  自动基准 75.17 fps；可见窗口右键菜单验收时 39.6 fps，均高于 15 fps。
- Taichi：Python 3.12.6、Taichi 1.7.4、`arch=x64` CPU，96×54/30 步；烟雾
  903.69 ms、泼水 1216.76 ms（均含运行时初始化），5,184 个输出全部有限且在 [0,1]。
- 无 Taichi 环境仍运行同尺寸确定性 CPU 回退，并在 JSON 中写入
  `backend=cpu-fallback`，不会伪装成 Taichi。

## Phase 7 性能预算与 GPU 路径

CI CPU 门禁保持既有数值断言：点选掩码 P95 < 500 ms/帧、内存 < 4 GiB、GUI 预览 >= 15
fps、参数调整后的半分辨率预览目标 < 300 ms、云端冷启动 < 30 s。单项相对基线回归超过 20% 发出警告，超过 50% 失败。GPU runner
不存在时不伪造 GPU 数字，必须记录 `gpu=unavailable` 并执行同一 CPU 门禁。

ONNX Runtime 的提供者探测顺序为 CUDA → DirectML → CPU；当前锁定的模型仍按
`docs/model-cards.md` 永久限制，不会因为探测到 GPU 就宣称 SAM/XMem 已验收。预览采用
半分辨率快速路径，确认后再排队全量渲染；GUI 的 `--benchmark` 结果是可重复的窗口合同，
不是神经模型吞吐承诺。

## Phase 9 锁定权重本地技术运行

2026-08-22 在 Windows、NVIDIA GeForce RTX 4060 Laptop GPU（8 GiB、驱动 572.83）、
Python 3.13、PyTorch 2.7.1+cu118 上运行 `tools/phase9_real_pipeline.py`。本节是一次
失败/待验收的技术记录，**不是**性能承诺或发布质量结论。输入均为
HappyHorse-1.1-T2V AI 生成测试素材，非实拍，生成提示词未提供；其 SHA-256、媒体参数和
使用边界见 `docs/release-assets.md` 与 `assets/demo-footage/README.md`。

管线实际载入并校验了锁定的 SAM ViT-B、XMem、MiDaS DPT Swin2 Tiny 256，以及去物/移动
使用的 ProPainter、RAFT-Things、ProPainter-RFC。推理分辨率为 640x352，输出再合成为
1280x720 的 before/after 同屏 H.264 视频。没有调用 C++ 传统分割、质心跟踪或 CPU 修复
降级路径。

| 输入与操作 | 帧数 | 相邻掩码 IoU（min / P50） | 总墙钟 / 平均每帧 | 自动门槛 | 视觉质量结论 |
| --- | ---: | --- | --- | --- | --- |
| 人物 / 去物 | 194 | 0.9268 / 0.9897 | 95.93 s / 494.46 ms | IoU 通过（> 0.7） | **失败**：人物残影、背景修复纹理与棋盘伪影明显，未达到无拖影/无闪烁 |
| 车辆 / 移动 | 194 | 0.9974 / 0.9999 | 109.02 s / 561.94 ms | IoU 通过（> 0.7） | **失败**：车体掩码/修复区域有明显纹理伪影，移动后存在贴图感，原位修复不可签收 |
| 宠物 / 树干烟雾 | 194 | 0.9621 / 0.9919 | 28.17 s / 145.21 ms | IoU 通过（> 0.7；从第 72 帧目标首次完整可见处计） | **待维护者验收**：自动指标不证明遮挡正确；烟雾与狗轮廓关系不得标为通过或发布 |

`needs_models` 在本机 `build/phase9-models` 实跑：

```text
100% tests passed, 0 tests failed out of 2
Label Time Summary:
needs_models = 11.25 sec*proc (2 tests)
```

其中 `model_checkpoint_smoke_test` 对六个锁定文件执行了 SHA-256、字节数和 PyTorch 载入
校验。该 CTest 通过只证明权重供应链和可载入性；上述真实视频的视觉失败不因它而解除。
在人物与车辆达到视觉门槛、宠物获得维护者肉眼确认，并以授权实拍素材复测之前，任务③④、
RC 与任何 Release 上传均保持阻断。
