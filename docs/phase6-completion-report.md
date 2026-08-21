# Phase 6/6.1 完成报告

CI_RUN_URL=NOT_YET_AVAILABLE
RELEASE_URL=NOT_YET_AVAILABLE
MATERIAL_STATUS=pending

## 机制落地

- `docs/git-workflow.md` 顶部永久区分日常 PUSH 与里程碑 RELEASE；Release 物料不再连坐
  代码推送或标签。
- `tools/sync_check.py` 以退出码核对工作区、Gitee `master`、全部本地标签和 GitHub 镜像；
  `release_checklist.py` 第 0 步直接调用，GitHub/Gitee CI 也执行。
- CONTRIBUTING 和 `pre-push` 占位钩子明确要求每个工作日结束前运行同步检查。

## 真实化验收

- GUI：GLFW 3.4 + Dear ImGui 1.90.9 + OpenGL 3.0 真实窗口，包含 1280×720 预览、
  点选、右键编辑、撤销/重做、时间轴、节点图和脚本导出；基准 75.17 fps，可见窗口
  验收最低观察值 39.6 fps，中文显示正常。
- Taichi：Python 3.12.6 + Taichi 1.7.4 `arch=x64`，烟雾/泼水各 30 步通过，输出全部
  有限且位于 [0,1]；耗时 903.69 ms / 1216.76 ms（含初始化）。
- 模型终局路径：三类 1280×720/30fps/10 秒高对比素材，共 900 帧，提示分割 + IoU
  关联最小 IoU 1.0、最慢 6.90 ms/帧、轨迹 900/900 连续。

## 永久已知限制与商业影响

SAM/XMem、ProPainter、Depth Anything 的冻结目标没有合格的固定 ONNX 导出、张量合同、
64 位 SHA256 和许可结论，`download_models.py --lock` 会拒绝下载。`v0.5.0` 永久采用
高对比/低遮挡素材白名单、CPU 修复和启发式深度；低对比、完全遮挡重现、快速形变及
复杂纹理不作质量承诺。云渲染和模板市场入口保持默认关闭，不销售神经点选、神经修复或
真实深度 SLA。Release 正文、`docs/model-cards.md`、`docs/business-model.md` 与 roadmap
使用同一限制。

## 验证结果

- GUI ON：MinGW 构建成功，窗口合同自检通过；最终 CTest 结果在发布提交后记录。
- GUI OFF：最终双构建结果在发布提交后记录。
- Python、版权、格式、DCO 与远端证据在发布提交后记录。

## 远端证据

远端提交、三标签、三版 Release 与 CI URL 仅在实际创建后写入本节；不以预期链接替代
平台记录。
