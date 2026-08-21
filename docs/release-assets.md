# v0.6.0 / v1.0.0 发布物料清单

状态：**pending**。本页是物料制作和上传台账，不是发布完成证据。

## 已验证的历史本地产物

| 用途 | 本地路径 | 媒体参数 | SHA-256 | 是否足以清零 |
| --- | --- | --- | --- | --- |
| 粒子遮挡 before | `build/demo-input.mp4` | 1 秒，320×240，30 fps，MPEG-4，82,814 bytes | `1302bd69352829ad971acaa623bed475b79010ca8968de109b637cd0fe3add8a` | 否 |
| 粒子遮挡 after | `build/particle-occlusion-demo.mp4` | 1 秒，320×240，30 fps，H.264，176,586 bytes | `f67e9dbd8c69db4af60a3d1ef8606abef065110c20764ef5d6e335d90cacc4ef` | 否 |

以上文件已由 `ffprobe` 验证可解码，但位于被忽略的构建目录，不作为仓库源码，也没有
外部下载链接。它们是 320×240 合成输入，只覆盖早期粒子/遮挡链路，不能冒充真实实拍
效果，也不能用于 Phase 9 核心完工验收。

## 清零所需物料

| 物料 | 最低内容 | 归属 | 状态 | 完成证据 |
| --- | --- | --- | --- | --- |
| Phase 7 产品演示 | GUI provider 提示、快速/全量预览切换、曲线编辑和导出逐帧可复现 | Phase 9 任务③ | pending | 待真实效果稳定后录制；URL + SHA-256 |
| 人物去物 demo | 5~10 秒 720p AI 生成测试素材，before/after 同屏；不得标为实拍 | Phase 9 任务②/③ | available-for-pipeline | 原始文件本地登记见 `assets/demo-footage/README.md`；待真实素材补齐后才能成为发布证据 |
| 车辆移动 demo | 5~10 秒 720p AI 生成测试素材，原位修复和新位置合成可见；不得标为实拍 | Phase 9 任务②/③ | available-for-pipeline | 原始文件本地登记见 `assets/demo-footage/README.md`；待真实素材补齐后才能成为发布证据 |
| 宠物/运动物体特效 demo | 5~10 秒 720p AI 生成测试素材，泼水或烟雾遮挡关系可见；不得标为实拍 | Phase 9 任务②/③ | available-for-pipeline | 原始文件本地登记见 `assets/demo-footage/README.md`；待真实素材补齐后才能成为发布证据 |
| 中英字幕与封面 | 三支核心 demo 均有中英字幕和封面，不剪辑美化效果 | Phase 9 任务③ | pending | 待三条真实效果通过肉眼验收 |
| 双平台 Release 资产 | Gitee/GitHub 文件名、字节数、SHA-256 一致 | Phase 9 任务③/④ | pending | 待两个平台的可访问下载链接 |
| 传播平台链接 | 至少 B 站或 YouTube 可访问链接，并与 Release 资产对应 | Phase 9 任务③ | pending | 待真实上传 URL |

每支视频上传前必须从候选构建重新复现并登记文件名、字节数、SHA-256、原始素材来源与
授权说明、处理命令、构建提交、媒体参数和校验值。当前三段输入明确为 HappyHorse-1.1-T2V
AI 生成测试素材，不能替代实拍发布证据。源码 zip/tar.gz、CI artifact 名称或本地文件存在
都不等于演示物料已上传。
