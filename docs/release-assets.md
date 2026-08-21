# v0.6.0 / v1.0 发布物料清单

状态：**pending**。本页是物料制作和上传台账，不是发布完成证据。

## 已验证的本地产物

| 用途 | 本地路径 | 媒体参数 | SHA-256 | 是否足以清零 |
| --- | --- | --- | --- | --- |
| 粒子遮挡 before | `build/demo-input.mp4` | 1 秒，320×240，30 fps，MPEG-4，82,814 bytes | `1302bd69352829ad971acaa623bed475b79010ca8968de109b637cd0fe3add8a` | 否 |
| 粒子遮挡 after | `build/particle-occlusion-demo.mp4` | 1 秒，320×240，30 fps，H.264，176,586 bytes | `f67e9dbd8c69db4af60a3d1ef8606abef065110c20764ef5d6e335d90cacc4ef` | 否 |

以上文件已由 `ffprobe` 验证可解码，但位于被忽略的构建目录，不作为仓库源码，也没有
外部下载链接。它们只覆盖早期粒子/遮挡链路，不能冒充 Phase 7 的 GPU provider、
半分辨率预览、关键帧曲线或支付沙箱演示。

## 清零所需物料

| 物料 | 最低内容 | 状态 | 完成证据 |
| --- | --- | --- | --- |
| Phase 7 产品演示 | GUI provider 提示、快速/全量预览切换、曲线编辑和导出逐帧可复现 | pending | 待上传 URL + SHA-256 |
| v1.0 主 demo | 冻结版安装、去物、特效、导出；不得演示 OUT 能力 | pending | RC 稳定后录制 |
| 模板 before/after | 每个 v1.0 新模板各一组 | pending | 待任务⑥冻结产物 |
| 中英字幕与封面 | B 站/YouTube/掘金/Reddit 尺寸和文案矩阵 | pending | 待传播平台 URL |
| 双平台 Release 资产 | Gitee/GitHub 文件名、字节数、SHA-256 一致 | pending | 两个平台下载链接 |

上传前必须从 RC 构建重新复现并记录命令、提交、媒体参数和校验值。源码 zip/tar.gz、CI
artifact 名称或本地文件存在都不等于演示物料已上传。
