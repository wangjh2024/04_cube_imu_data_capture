# 多智能体自组织作业规范

本工程与 05 后处理工程按采集-后处理双工程闭环协作。

## 角色分工

| 角色 | 职责 | 主要产物 |
| --- | --- | --- |
| Capture Lead | 拆解采集任务、控制现场顺序和交付边界 | 采集计划、交付说明 |
| Flow Engineer | 维护 `Makefile`、检查脚本和标准流程 | 可执行入口、检查结果 |
| Hardware Operator | 确认相机、串口 IMU、光照、Cube 可见性 | 现场预检结论 |
| Data QA | 校验 `output_bag/` 数据契约和采集门槛 | `capture_manifest.json` |
| Downstream Reviewer | 在 05 工程复核 robust 后处理质量 | `flow_manifest.json`、review notes |
| Release Steward | 区分源码改动和采集产物，确认提交边界 | 交付清单 |

## 推进顺序

```text
01 Intake     明确采集目的、硬件和下游工程
02 Diagnose   检查工程结构、ROS 环境、旧生成目录
03 Plan       明确采集参数、动作阶段、验收命令
04 Capture    按 GUI 六阶段动作采集
05 QA         检查 output_bag 契约和质量门槛
06 Handoff    交给 05 工程 run-robust
07 Close      记录结果、风险和是否可发布
```

## 相互监督

- Hardware Operator 不独自判定成功，必须经过 Data QA 检查。
- Flow Engineer 修改参数后，Data QA 必须复查 `configs/capture.yaml` 和 ROS 参数一致。
- Capture Lead 不把 04 的采集通过等同于外参发布通过。
- Downstream Reviewer 必须在 05 工程完成质量门槛和互审。
- Release Steward 交付前必须查看 `git status --short --branch`。

## 最小验收

```bash
make check
make check-data
```

发布候选外参前还必须在 05 工程完成：

```bash
make run-robust
make check-release
```
