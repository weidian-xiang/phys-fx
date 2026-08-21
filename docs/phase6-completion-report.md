# Phase 6/6.1 完成报告

CI_RUN_URL=https://github.com/weidian-xiang/phys-fx/actions/runs/32444480369
CI_PUSH_RUN_URL=https://github.com/weidian-xiang/phys-fx/actions/runs/32444480369
CI_PR_RUN_URL=NOT_YET_AVAILABLE
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

- GUI ON：MinGW 构建零项目警告，窗口合同自检通过，CTest 25/25 通过。
- GUI OFF + Taichi 1.7.4：构建零项目警告，CTest 26/26 通过。
- Python：pytest 17/17 通过；版权、clang-format、提交规范、DCO 和
  `git diff --check` 均通过。
- GitHub Actions push 运行 #6 全绿：`quality` 与 `optional-switches` 两个作业均成功。

## 远端证据

- Gitee 与 GitHub `master` 均已同步到 `dc2702480db3a5d748900efd214e8010bc533a97`。
- Gitee/GitHub 已存在 `v0.3.0`、`v0.4.0` 标签；`v0.5.0` 与三版 Release 在发布门禁
  完成后补录，不以预期链接替代平台记录。
- Push CI：<https://github.com/weidian-xiang/phys-fx/actions/runs/32444480369>。
