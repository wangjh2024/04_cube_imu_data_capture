# 工程顺序流规范

本工程现在按 ROS2 数据采集工程管理。完整、可执行的标准流程见
`docs/STANDARD_FLOW.md`；输入输出字段契约见 `docs/DATA_CONTRACT.md`。

日常入口：

```bash
cd /home/wangjh/code/04_cube_imu_data_capture
make check
make build
make launch-gui
make check-data
```

采集结果 `output_bag/` 交给：

```bash
cd /home/wangjh/code/05_cube_pose_imu_calibration
make check
make run-robust
```
