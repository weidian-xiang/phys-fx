# PhysFX 单机云服务

```powershell
docker compose -f cloud/deploy/docker-compose.yml up
```

服务默认监听 `127.0.0.1:8765`。这是 Phase 6 的单机参考部署：API、SQLite 队列和本地对象目录均可替换，未提供多副本、高可用或支付能力。生产环境应在反向代理终止 HTTPS，并将任务容器与 API 隔离。
