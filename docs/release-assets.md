# v0.6.0 / v1.0.0 发布物料清单

状态：**pending**。本页是物料制作和上传台账，不是发布完成证据。

## Phase 9 本地 AI 测试输入登记

以下原始输入均位于被 `.gitignore` 排除的 `assets/demo-footage/*.mp4`，不纳入 Git，
也不得在 Release、公告或演示中表述为实拍。参数由 `ffprobe`、校验值由
`Get-FileHash -Algorithm SHA256` 于 2026-08-21 本地核验。

| 文件 | 设计用途 | 实测媒体参数 | 字节数 | SHA-256 | 来源与提示词 | 规格结论 |
| --- | --- | --- | ---: | --- | --- | --- |
| `人物.mp4` | 去物 | 1280x720、8.12 s、24 fps、194 帧、H.264/AAC | 2,504,649 | `fd70015f09dc7f4206e6b29c0a21d1ceec612ff40d5c4bf47cde8c0afbc9cf6d` | HappyHorse-1.1-T2V AI 生成；维护者未提供生成提示词，项目不得推断 | 720p、5~10 s 达标；非实拍 |
| `车辆.mp4` | 移动 | 1280x720、8.12 s、24 fps、194 帧、H.264/AAC | 2,536,206 | `e8ff5dfbcc1ae858e0dd71be8a9780d8526a7dcdef7cd50477c0b6e8e2daa32e` | HappyHorse-1.1-T2V AI 生成；维护者未提供生成提示词，项目不得推断 | 720p、5~10 s 达标；非实拍 |
| `宠物.mp4` | 烟雾遮挡 | 1280x720、8.12 s、24 fps、194 帧、H.264/AAC | 2,688,159 | `2b569902e84cb87a4573ff892f0d67ca4a3b35687b34679759d4c61602833f9d` | HappyHorse-1.1-T2V AI 生成；维护者未提供生成提示词，项目不得推断 | 720p、5~10 s 达标；非实拍 |

容器还带有 `AIGC Label=1` 元数据。三段输入只能用于本地技术验证，不能满足
Phase 9 的实拍核心完工判据，也没有形成对外传播授权。

## Phase 9 真实权重本地输出

以下为 `tools/phase9_real_pipeline.py` 在锁定权重上的本地技术输出。文件均在被忽略的
`build/phase9-real/`，没有上传，也不是 Release 资产。`before/after` 同屏文件的宽度为
2566 像素（两张 1280 像素画面加 6 像素分隔）；均为 H.264、720 像素高、24 fps、194 帧、
8.083292 s。完整质量数据与失败结论见 `docs/perf-baseline.md`。

| 用途 | 本地输出 | 字节数 | SHA-256 | 当前质量结论 |
| --- | --- | ---: | --- | --- |
| 人物去物 | `build/phase9-real/person-before-after.mp4` | 44,455,310 | `9014a08729a1d7058fcbf6862bb074ff0c0f34bbec4230c9f29a9e0c86e594f2` | 不通过：残影与修复纹理伪影 |
| 车辆移动 | `build/phase9-real/vehicle-before-after.mp4` | 41,791,661 | `4085a2bef6ecb0da80f27ab093b6b33a8c3dcc1f8b44257ec4ccdaadc6ab9fba` | 不通过：车体掩码/修复伪影导致贴图感 |
| 宠物烟雾遮挡 | `build/phase9-real/pet-before-after.mp4` | 41,552,501 | `607572b919649677e0de1ce9030a860331a9a7634a8aea9a8bdd45b05edd4652` | 自动跟踪门槛通过；遮挡仍待维护者肉眼验收，不能发布 |

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
| 人物去物 demo | 5~10 秒 720p 实拍素材，before/after 同屏 | Phase 9 任务②/③ | blocked | 当前 AI 测试输入的真实权重输出视觉失败；仍缺授权实拍输入 |
| 车辆移动 demo | 5~10 秒 720p 实拍素材，原位修复和新位置合成可见 | Phase 9 任务②/③ | blocked | 当前 AI 测试输入的真实权重输出视觉失败；仍缺授权实拍输入 |
| 宠物/运动物体特效 demo | 5~10 秒 720p 实拍素材，泼水或烟雾遮挡关系可见 | Phase 9 任务②/③ | blocked | 当前 AI 测试输入仅完成跟踪数值门槛；遮挡待维护者验收，仍缺授权实拍输入 |
| 中英字幕与封面 | 三支核心 demo 均有中英字幕和封面，不剪辑美化效果 | Phase 9 任务③ | pending | 待三条真实效果通过肉眼验收 |
| 双平台 Release 资产 | Gitee/GitHub 文件名、字节数、SHA-256 一致 | Phase 9 任务③/④ | pending | 待两个平台的可访问下载链接 |
| 传播平台链接 | 至少 B 站或 YouTube 可访问链接，并与 Release 资产对应 | Phase 9 任务③ | pending | 待真实上传 URL |

每支视频上传前必须从候选构建重新复现并登记文件名、字节数、SHA-256、原始素材来源与
授权说明、处理命令、构建提交、媒体参数和校验值。当前三段输入明确为 HappyHorse-1.1-T2V
AI 生成测试素材，不能替代实拍发布证据。源码 zip/tar.gz、CI artifact 名称或本地文件存在
都不等于演示物料已上传。
