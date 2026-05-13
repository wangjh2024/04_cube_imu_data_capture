# 04 Cube IMU Data Capture

本工程是 `05_cube_pose_imu_calibration` 的采集端，负责用头戴相机观测手持
AprilTag Cube，并同步采集 Cube 上的串口 IMU。标准产物是 `cube_imu_direct_dataset_v1`
格式的 `output_bag_YYYYMMDD_HHMMSS/`，可直接交给 05 工程做 Pose-IMU 外参后处理。

本工程按 ROS2 数据采集工程管理，不再把 `.venv` 里的旧 `imu-cube-qt` /
`imu-cube-calib` 当作标准入口。旧虚拟环境可删除重建，源码入口以
`cube_imu_calibration/` ROS2 包为准。

## 快速运行

```bash
cd /home/wangjh/code/04_cube_imu_data_capture
make check
make build
make launch-gui
```

采集完成后检查输出契约：

```bash
make check-data
make capture-manifest
```

再交给 05 后处理工程：

```bash
cd /home/wangjh/code/05_cube_pose_imu_calibration
make check
make run-robust
```

## 标准入口

```text
Makefile                                      # 日常工程入口
configs/capture.yaml                         # 采集流程和质量门槛
scripts/check_project.py                     # ROS2 工程、配置和生成目录检查
scripts/check_capture_output.py              # 采集数据集契约检查
scripts/doctor.sh                            # 兼容入口，调用 check_project.py
docs/标准采集流程.md                         # 标准采集顺序流
docs/数据契约.md                             # 04 输出 / 05 输入数据契约
docs/多智能体作业规范.md                     # 多智能体角色分工和互审闭环
docs/交付检查清单.md                         # 交付检查清单
cube_imu_calibration/                        # ROS2 采集包
```

## 常用命令

```bash
make help           # 查看入口
make check          # 工程结构、配置和数据契约检查
make check-env      # ROS2/Python 依赖检查
make check-data     # 检查 DATASET，默认 output_bag，缺失时不阻断
make data-strict DATASET=output_bag_YYYYMMDD_HHMMSS
make capture-manifest DATASET=output_bag_YYYYMMDD_HHMMSS
make build          # colcon build cube_imu_calibration
make launch-gui     # 启动正式采集 GUI
make status         # 查看 git 状态
```

## 标准输出

```text
output_bag_YYYYMMDD_HHMMSS/
├── metadata.yaml
├── image_timestamps.csv
├── imu.csv
├── tag_pose.csv
├── camera_info.yaml
└── images/
```

可选检查产物：

```text
output_bag_YYYYMMDD_HHMMSS/capture_manifest.json
```

## 工程边界

- `04_cube_imu_data_capture`：负责采集、现场预检和数据契约检查。
- `05_cube_pose_imu_calibration`：负责离线 Pose-IMU 外参求解、质量门槛和候选外参互审。
- `03cube_imu_clib/04imu_cube_tracker`：实时跟踪消费端，只有外参通过多数据集复核后再同步配置。

## 注意事项

- `/imu` 必须来自 Cube 上的串口 IMU，不使用 Orbbec/相机内置 IMU。
- 正式数据默认使用 `left_hand_5face` Cube：ID0=center、ID1=top、ID2=right、ID3=bottom、ID4=left。
- GUI 采集默认 150 秒，按静止、X/Y/Z 旋转、多面过渡、小范围 8 字组合动作执行。
- `build/`、`install/`、`log/`、`Log/`、`outputs/`、`output_bag/`、`output_bag_*/`、`data/` 是生成或采集产物，默认不提交。
- `log/` 是 colcon 构建日志，`Log/` 是 Orbbec SDK 运行日志；二者都属于日志产物。
- 顶层 `.venv` 若仍指向旧路径，只作为历史残留处理，不作为标准入口。

更多细节见 `docs/标准采集流程.md` 和 `docs/数据契约.md`。
