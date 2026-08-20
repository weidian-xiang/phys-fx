# PhysFX 开源计划

## 对外口径

PhysFX 采用 Apache-2.0，核心引擎、管线、接口、基础桩和示例永远开源且功能完整。项目由向伟典发起，现阶段采用作者主导（BDFL）治理，逐步吸收社区维护者。

贡献使用 DCO：每个提交必须包含 `Signed-off-by`，Issue/PR 使用 `.gitee/` 模板，行为问题遵循 `CODE_OF_CONDUCT.md`，安全问题走 `SECURITY.md` 私密渠道。

## 发布与传播

- Phase 3 完成时发布 `v0.2.0`，同步公开路线图和变更说明。
- Demo 优先：实拍视频换车色并自动更新阴影、实拍视频加带地面反射的火焰、白天变夜晚并自动点亮路灯。
- 短视频 demo 面向 B 站、抖音和 YouTube Shorts，先展示“拖一下就有效果”，再解释底层接口。
- Gitee 是唯一主仓（`https://gitee.com/xiang-weidian/phys-fx`）；GitHub
  `https://github.com/weidian-xiang/phys-fx` 是只读镜像。`origin` 始终指向 Gitee，
  `github` 仅用于读取和校验，不接受开发推送。

## 开源范围与边界

核心能力不做解锁式收费，不在开源版植入授权阻断、远程开关或网络依赖。商业服务只在开源能力之上提供增量模板、云算力和企业支持。

镜像治理细则见 [`docs/mirror-sync.md`](mirror-sync.md)：同步只允许
Gitee → GitHub，Issue、PR、Release 和标签均以 Gitee 为准。
