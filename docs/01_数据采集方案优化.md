# 06imu_cube_calib 数据采集方案优化

本文档用于优化 `/home/wangjh/code/06imu_cube_calib` 的 IMU-Cube 标定数据采集流程。

核心目标：

```text
先保证视觉轨迹 T_Cam_Cube(t) 合格，再进行 IMU-Cube 外参标定。
```

当前问题不是优化器本身，而是采集端提供给 07 后处理的数据质量不足：

```text
Tag pose 太少
多面切换时姿态跳变
单面样本不足
视觉轨迹和 IMU gyro 对齐残差过大
```

## 1. AprilTag Cube 物理制作

当前模板是 5 面 AprilTag Cube：

```text
AprilTag 36h11 Cube - Left Hand Cube
ID0 = Center
ID1 = Top
ID2 = Right
ID3 = Bottom
ID4 = Left
```

至少需要完整贴出 5 个面：

```text
ID0, ID1, ID2, ID3, ID4
```

第 6 面可以为空，但采集时不能让空白背面长时间正对相机，否则视觉位姿会中断。

制作要求：

1. 按整张模板折叠，不要单独剪 Tag 后随意旋转粘贴。
2. AprilTag 黑色外框必须完整、平整、无遮挡。
3. 码区不要有折痕、反光、皱褶。
4. IMU 和 Cube 必须刚性固定，不能使用会晃动的软胶、松绑带或弹性连接。
5. 实测 `marker_size`：Tag 黑色外框边长，单位 m。
6. 实测 `cube_visual_size`：Cube 实际边长，单位 m。

## 2. 打开 06 GUI 的 Cube 检测

文件：

```text
/home/wangjh/code/06imu_cube_calib/cube_imu_calibration/src/scripts/calibration_gui.py
```

当前代码中 Cube 检测被强制关闭：

```python
self.node.declare_parameter("enable_cube_detection", False)
self.enable_cube_detection = False
```

建议改成：

```python
self.enable_cube_detection = bool(
    self.node.declare_parameter("enable_cube_detection", True).value
)
```

原因：

```text
如果不打开 Cube 检测，GUI 只会显示原始视频并录制图像、IMU、CameraInfo。
现场无法确认 T_Cam_Cube 是否连续，也无法发现多面切换时的 90/180 度跳变。
```

## 3. 提高 Tag 检测频率和录制时长

当前 06 已经把采集端参数收紧，建议保持：

```yaml
marker_size: 实测值
cube_visual_size: 实测值
cube_layout: left_hand_5face
target_tag_id: -1
dictionary: DICT_APRILTAG_36h11
enable_cube_detection: true
tag_detection_rate_hz: 15.0
tag_detection_scale: 0.5
camera_output_encoding: mono8
image_preview_rate_hz: 5.0
preview_scale: 0.5
gui_display_rate_hz: 5.0
draw_preview_overlay: true
color_fps: 15
record_duration_sec: 150.0
data_recorder_node.image_qos_depth: 120
data_recorder_node.imu_qos_depth: 2000
```

说明：

```text
当前 07 后处理看到有效 tag pose rate 只有约 2 Hz，太低。
目标是让有效 tag pose rate 尽量达到 10 Hz 左右。
GUI 会显示 topic/cube_pose/rec_hz，并在图像或 Cube pose 低于 8 Hz 时阻止正式采集推进。
1280x720 30Hz BGR 对当前 GUI + rosbag 链路压力较大，先用 15Hz 稳定采集。
real_sensor_node 默认发布 mono8，AprilTag 检测只需要灰度图，可把图像带宽降到 bgr8 的约 1/3。
tag_detection_scale=0.5 会在半分辨率灰度图上检测，再把角点缩放回原图做 solvePnP。
显示预览只按 5Hz、0.5 倍尺寸刷新；显示是给现场判断用，不能抢占检测和录包资源。
```

如果 GUI 显示仍然拖慢，可以临时进一步降低显示负载：

```bash
ros2 launch cube_imu_calibration calibration_launch.py \
  image_preview_rate_hz:=2.0 \
  gui_display_rate_hz:=2.0 \
  draw_preview_overlay:=false
```

不要优先降低 `tag_detection_rate_hz`，除非 CPU 已经满载；它直接影响 `/cube_pose` 频率。

## 4. 正式采集前增加预检查

预检查不计入正式数据。

操作流程：

1. 固定头戴相机，不要移动相机。
2. Cube 放在画面中央。
3. 分别转到 `ID0/1/2/3/4`，确认每个 ID 都能识别。
4. 缓慢做相邻面过渡：

```text
ID0 -> ID1
ID0 -> ID2
ID0 -> ID3
ID0 -> ID4
```

5. 过渡时尽量让两个 Tag 同时出现在画面中。
6. GUI 中的 Cube 坐标轴不能突然翻转 90/180 度。
7. Image、IMU、CameraInfo 的 header.stamp 状态必须正常。
8. GUI 显示 `image`、`cube_pose`、录制 `rec_hz image` 均应稳定不低于 `8 Hz`。

不通过预检查时不要正式采集。

典型不通过情况：

```text
某个 ID 长时间识别不到
切换面时 Cube 坐标轴突然翻转
Tag 检测频繁丢失
图像模糊或反光
IMU 数据无变化或时间戳异常
image/cube_pose/rec_hz 只有 1-2Hz
```

## 5. 正式 6 阶段动作方案

建议总时长：`150s`。

动作原则：

```text
慢速
连续
多轴激励充分
Tag 始终在画面内
避免快速甩动
避免图像拖影
避免空白背面长时间正对相机
```

### 阶段 0：静止

时长：`10s`

操作：

```text
Cube 放在画面中央，保持静止。
```

目的：

```text
估计 gyro bias，检查图像/IMU/CameraInfo 时间戳稳定性。
```

### 阶段 1：绕 Cube X 轴慢速旋转

时长：`20s`

操作：

```text
绕 Cube X 轴缓慢往返旋转。
不要甩动，不要出画。
```

### 阶段 2：绕 Cube Y 轴慢速旋转

时长：`20s`

操作：

```text
绕 Cube Y 轴缓慢往返旋转。
至少保持一个 Tag 始终可见。
```

### 阶段 3：绕 Cube Z 轴慢速旋转

时长：`20s`

操作：

```text
绕 Cube Z 轴缓慢往返旋转。
避免空白背面正对相机。
```

### 阶段 4：多面过渡 + 小平移

时长：`30s`

操作：

```text
慢慢经过 ID0-ID1、ID0-ID2、ID0-ID3、ID0-ID4 的边界。
尽量让两个 Tag 同时可见。
同时加入 5-10cm 小平移。
```

目的：

```text
验证多面几何关系 T_Cube_Tag_i 是否一致。
减少单面切换带来的姿态跳变。
```

### 阶段 5：小范围 8 字 + 三轴组合

时长：`50s`

操作：

```text
做小范围 8 字运动。
平移幅度约 5-15cm。
同时加入三轴慢速组合旋转。
```

目的：

```text
提供旋转和平移联合激励，提升外参和时间偏移可观性。
```

## 6. 06 建议增加记录内容

当前主要录制：

```text
/camera/image_raw
/camera/camera_info
/imu
```

建议增加：

```text
/cube_pose
/cube_pose_status
```

最低要求仍然是录制完整原始图像和 IMU，因为 07 可以离线重新检测 AprilTag。

但如果能同时记录 `/cube_pose` 和质量状态，现场就能快速判断数据是否合格。

## 7. tag_pose.csv 建议字段

建议从：

```text
timestamp,tx,ty,tz,qx,qy,qz,qw
```

扩展为：

```text
timestamp,tx,ty,tz,qx,qy,qz,qw,tag_count,reproj_error,tag_ids
```

字段用途：

```text
tag_count     当前帧参与位姿估计的 Tag 数量
reproj_error  重投影误差，用于判断视觉位姿质量
tag_ids       当前帧使用的 Tag ID 列表
```

## 8. 多 Tag 联合 PnP

当前单 Tag 最大面积选择方式容易在跨面切换时出现姿态跳变。

建议改为：

1. 检测当前帧所有可见的 `ID0-4`。
2. 根据每个 ID 的 `T_Cube_Tag_i` 生成 Cube 坐标系下的 3D 角点。
3. 合并所有可见 Tag 的 2D-3D 点。
4. 使用 `solvePnP` 联合估计 `T_Cam_Cube`。
5. 输出 `tag_count/reproj_error/tag_ids`。
6. 双 Tag 或多 Tag 同时可见时，显示单 Tag 反推 Cube 位姿的一致性：

```text
单Tag一致性 = 最大旋转差 deg / 最大平移差 mm
```

若该值明显偏大，优先检查：

```text
Cube 是否确认为 Left Hand Cube
ID0-4 是否按模板整张折叠
cube_visual_size 是否为折好后的实测边长
```

这样可以在相邻面同时可见时约束 Cube 位姿，减少 90/180 度跳变。

## 9. 采集完成后立即用 07 验收

采完后立刻运行：

```bash
cd /home/wangjh/code/07process_data
./scripts/process_imu_cube_extrinsics.py --bag output_bag --output-dir processed/output_bag
```

合格标准：

```text
Tag 检测率 > 80%
5 个 ID 都出现
tag_pose rate 尽量 > 10 Hz
重投影 RMSE < 1-2 px
最大姿态跳变 < 45 deg
gyro_alignment_rmse_rad_s < 0.2-0.35
至少 3 组数据外参结果一致
```

## 10. 当前数据暴露的问题

上一版数据的主要问题已经从“检测不到”变成“频率和跨面一致性仍不足”。当前已改善：

```text
/cube_pose 已记录
/cube_pose_status 已记录
Tag 检测率 ≈ 97.9%
全 Cube gyro RMSE 从 ≈ 2.55 rad/s 降到 ≈ 0.80 rad/s
重投影 RMSE ≈ 0.648 px，仍可接受
5 个 ID 都出现
```

但仍未达到可标定标准：

```text
pose rate ≈ 1.76 Hz，目标 > 10 Hz
最大姿态跳变 ≈ 167.5 deg，目标 < 45 deg
ID2 单面 gyro RMSE ≈ 1.25 rad/s，明显可疑
215s 只有 386 张图像，目标应是 2000+ 张量级
```

下一轮优化重点：

```text
让图像和 cube_pose 频率先稳定到 8-10Hz 以上
采集前单独检查 ID2 的贴法、方向和成像质量
继续观察双 Tag/多 Tag 同时可见时的单 Tag 一致性
避免跨面切换时 Cube 坐标轴 90/180 度翻转
采完立刻用 07 的 gyro_alignment_rmse_rad_s 和 max_rotation_jump_deg 验收
```

## 11. 推荐执行顺序

```text
1. 重新确认 AprilTag Cube 是否按模板折叠。
2. 实测 marker_size 和 cube_visual_size。
3. 启动 06 GUI，确认 `enable_cube_detection=true`。
4. 等待 `image`、`cube_pose`、录制 `rec_hz image` 都稳定 ≥ 8Hz。
5. 单独检查 ID2：正对、倾斜、与 ID0/ID1/ID3/ID4 过渡时都不能跳变。
6. 检查双 Tag/多 Tag 同时可见时 `单Tag一致性` 不明显偏大。
7. 正式采集前做 ID0-4 预检查。
8. 按 150s 六阶段方案重采。
9. 用 07 后处理验收。
10. 连续采 3 组，检查 T_tag_imu 一致性。
```
