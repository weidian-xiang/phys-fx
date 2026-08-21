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
