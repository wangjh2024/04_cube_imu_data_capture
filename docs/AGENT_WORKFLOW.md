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

## 标准化五段门禁

本工程采用“拆解 -> SOP -> 执行 -> 固化 -> 复盘”的串行门禁。每一段必须有明确产物，监督 Agent 不通过时直接打回上一段，不允许带病进入下一步。

| 阶段 | 主责 Agent | 输入 | 输出 | 监督门禁 |
| --- | --- | --- | --- | --- |
| 1. 任务拆解 | 任务拆解 Agent | 用户目标、现场约束、工程现状 | 《任务拆解书》：三阶段顺序、步骤流、时间节点 | 监督 Agent 审核目标、边界、顺序是否完整；不合格打回重拆 |
| 2. 流程编制 | 流程编制 Agent | 通过审核的任务拆解书 | 《步骤 SOP 文档》：结构化、模板化、可执行命令 | 监督 Agent 审核命名、步骤、验收口径；不规范打回重编 |
| 3. 执行落地 | 执行落地 Agent | 通过审核的 SOP | 分步骤成果物、执行日志、验证结果 | 每步完成后监督 Agent 节点验收；不通过不得进入下一步 |
| 4. 标准固化 | 标准固化 Agent | 已验收的执行结果 | 《标准化文档》《模板库》《术语表》 | 监督 Agent 终审；通过后入库、版本归档、可复用 |
| 5. 复盘迭代 | 全体 Agent | 闭环交付记录、问题清单 | 规则迭代记录、下一轮 SOP 优化项 | 监督 Agent 汇总分工漏洞、流程卡点、标准盲区 |

规则遗传原则：好规则保留并进入通用 SOP；坏规则淘汰或降级为风险提示；新增规则必须能被命令、文档或检查清单验证。

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
