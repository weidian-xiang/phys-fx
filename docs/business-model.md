# 商业模式与预埋边界

PhysFX 采用 Open Core：Apache-2.0 引擎免费、完整、可自托管；商业收入来自增量服务，不来自阉割基础能力。

| 收费点 | 预留接口/文件 | 计划阶段 | 约束 |
| --- | --- | --- | --- |
| 高级特效/编辑模板包 | `plugins/IEffectPlugin.h`、`IEditTemplatePlugin.h`、`ILicenseVerifier.h` | Phase 5 | 模板包包含签名字段；开源模板不强制授权 |
| 云渲染 | `cloud/IRenderServiceClient.h` | Phase 6 | 本地渲染永远可用，云端提供批量/高分辨率/排队加速 |
| 模板市场 | `cloud/ITemplateMarketplaceClient.h` | Phase 6 | 第三方作者可发布、售卖和分成 |
| 企业版 | 文档与服务部署边界 | Phase 6 | 批量处理、优先支持、私有部署，不删减社区能力 |

## 模板包骨架

未来模板包应包含 `manifest`、资源列表、版本、发布者、签名算法和签名字段。Phase 2 只定义 `TemplatePackageManifest` 与 `ILicenseVerifier`，不实现加密、联网或授权强制逻辑。

## 红线

1. 开源版永不阉割已有功能。
2. 商业增值只增量，不做“付费解锁基础按钮”。
3. “PhysFX”名称、Logo 和品牌视觉保留商标权，代码版权按 Apache-2.0 授权。
4. 云服务失败不应阻塞本地管线；核心构建不得出现网络调用。
