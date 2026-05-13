# 数据契约

04 采集工程输出的 `output_bag/` 必须满足 05 后处理工程的输入契约。

## 输出目录

```text
output_bag/
├── metadata.yaml
├── image_timestamps.csv
├── imu.csv
├── tag_pose.csv
├── camera_info.yaml
└── images/
```

`metadata.yaml`：

```yaml
format: cube_imu_direct_dataset_v1
image_dir: images
image_timestamps: image_timestamps.csv
imu_csv: imu.csv
camera_info: camera_info.yaml
pose_csv: tag_pose.csv
```

## image_timestamps.csv

```text
timestamp,filename,width,height,encoding,seq
```

- `timestamp` 单调递增
- `filename` 指向 `output_bag/images/`
- `encoding` 推荐 `mono8`

## imu.csv

```text
timestamp,ax,ay,az,gx,gy,gz,index
```

- 加速度单位：`m/s^2`
- 角速度单位：`rad/s`
- IMU 必须是 Cube 串口 IMU，不是相机内置 IMU

## tag_pose.csv

```text
timestamp,tx,ty,tz,qx,qy,qz,qw,tag_count,reproj_error,tag_ids
```

- pose 为 `T_camera_cube`
- 平移单位：`m`
- 四元数顺序：`qx,qy,qz,qw`
- `tag_ids` 使用模板 ID0-4，可为单 Tag 或多 Tag 组合

## camera_info.yaml

必须包含 3x3 `camera_matrix` 和至少 4 个畸变参数。

## 检查命令

```bash
make check-data
make capture-manifest
```

`capture_manifest.json` 是采集端检查报告，不替代 05 的 `flow_manifest.json`。
