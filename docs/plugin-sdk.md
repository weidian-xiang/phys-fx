# PhysFX 插件 SDK（v1 冻结候选）

本页描述第三方如何从零开发、打包、验签和安装一个模板插件。SDK 不删除社区版任何能力，
签名只表达来源信任，不是功能解锁。插件不得上传视频、读取 API key 或绕过本地隐私设置。

## 1. 开发

实现 `IEffectPlugin` 或 `IEditTemplatePlugin`，固定 `PluginMetadata` 的名称、版本和类型。
编辑模板推荐返回版本化命令描述，例如：

```json
{"type":"move_entity","entity_id":1,"x":520,"y":300}
```

只使用公开的 `Frame`、`SceneContext` 和编辑命令接口；不要依赖 GUI 私有头文件。完整最小
示例见 `plugins/examples/empty_effect/`。

v1 插件推荐使用 `physfx::v1::plugins`、`physfx::v1::core` 等稳定入口；0.x 的
`physfx::plugins` 拼写在整个 1.x 保持源码兼容。弃用至少保留一个 minor，公共接口和
虚函数布局的破坏性变更不进入 1.x。跨编译器/运行库的二进制插件必须从源码重编译，详见
`docs/compatibility.md`。

## 2. 打包与验签

创建目录 `my-template/manifest.json`，至少写入 `packageId`、`version`、`publisher`，
再执行：

```powershell
python tools/pack_template.py pack my-template my-template.pfxtpl
python tools/pack_template.py verify my-template.pfxtpl
```

官方包由离线 Ed25519 私钥签名：

```powershell
python tools/pack_template.py pack my-template my-template.pfxtpl --key signer.pem --public-key signer.pub
python tools/pack_template.py verify my-template.pfxtpl --public-key signer.pub
```

私钥只能保存在维护者控制的密钥库，绝不提交 Git、CI 日志或模板包。未签名包在开源版仍
可安装；市场入口可按审核策略拒绝未签名包，但核心能力不被阉割。

## 3. 安装与发布检查

先在干净目录执行安装，再运行 `physfx --version`、一个示例编辑脚本和模板对应的 CTest。
发布检查应记录 package digest、签名状态、引擎最低版本、许可证和已知限制。作者收益在
支付主体获批前只记账待结算，不承诺提现到账；支付开关关闭时不产生任何资金流水。

## 4. 安全清单

- 不执行 manifest 中的 shell、动态下载或路径穿越。
- 资源限制：manifest <= 1 MiB，命令 <= 10,000 条，路径必须位于安装目录。
- 只声明真实能力；不得把永久模型降级路径宣传成神经推理。
- 问题提交请附最小 `.pfxtpl` 和脱敏日志，不上传原始视频。
