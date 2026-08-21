# `.pfxtpl` 模板包格式

`.pfxtpl` 是 ZIP 容器，包含 `manifest.json`、模板文件和可选 `SIGNATURE`。清单至少包含
`packageId`、`version`、`publisher`；官方包增加 `signatureAlgorithm: "Ed25519"`、
`publicKey` 和 `contentDigest`。签名覆盖规范化清单以及按文件名排序的
`文件名 + NUL + 文件字节` 序列。

使用 `python tools/pack_template.py pack <目录> <包.pfxtpl> --key <离线私钥>` 打包，
使用 `verify` 验签。私钥不得入库或写入 CI；详见 [SECURITY.md](../SECURITY.md)。

开源版永远允许加载未签名模板。验签失败只显示“未认证来源”警告，不阻断模板功能。
