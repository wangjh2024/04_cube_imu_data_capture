# 交付检查清单

## 工程结构

- [ ] `Makefile` 日常入口存在
- [ ] `configs/capture.yaml` 描述采集参数和质量门槛
- [ ] `docs/STANDARD_FLOW.md` 描述顺序流
- [ ] `docs/DATA_CONTRACT.md` 与 05 输入契约一致
- [ ] `docs/AGENT_WORKFLOW.md` 描述角色分工和互审闭环

## 验证命令

```bash
make check
make check-env
make check-data
```

若存在正式采集结果：

```bash
make data-strict
make capture-manifest
```

## 交付说明

- [ ] 说明采集数据目录
- [ ] 说明通过的检查命令
- [ ] 说明未运行的硬件/GUI 命令
- [ ] 说明剩余风险，例如 Tag 跳变、低检测率、串口异常
- [ ] 说明哪些是源码改动，哪些是生成/采集产物
- [ ] 说明 05 后处理是否通过 `make run-robust` 和 `make check-release`
