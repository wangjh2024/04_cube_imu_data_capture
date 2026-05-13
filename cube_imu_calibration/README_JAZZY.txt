cube_imu_calibration - ROS 2 Jazzy Cube/IMU 外参标定工程
=========================================================

坐标约定
--------
本工程统一使用 T_A_B 表示“把 B 坐标系下的点变换到 A 坐标系”：

  p_A = T_A_B * p_B

当前按 Kalibr 静态链路工作：

  相机、IMU、AprilTag/Cube 的几何关系在求解链路中必须是同一个静态刚体关系。
  Kalibr 阶段先得到合法的 T_Cam_IMU；GUI/AprilTag 阶段得到同一相机坐标系下的 T_Cam_Cube。

目标输出：

  T_Cube_IMU

即把 IMU 坐标系下的点变换到 Cube/AprilTag 坐标系：

  p_cube = T_Cube_IMU * p_imu

反向变换为：

  T_IMU_Cube = inv(T_Cube_IMU)

矩阵关系：

  T_Cube_IMU = inv(T_Cam_Cube) * T_Cam_IMU

重要：Kalibr 的 T_Cam_IMU 只在相机和 IMU 刚性固定、共同运动时成立。如果实际硬件是“头戴相机观察手持 Cube/IMU”，不要使用本静态链路；那种结构应使用 pose-IMU 时空标定。


1. 正式运行节点
---------------
正式采集推荐三个 ROS2 节点：

  1. real_sensor_node
     复用 04imu_cube_tracker 的真实硬件采集方式：Orbbec Gemini/Mini335 彩色相机发布图像和 CameraInfo，Prometheus v2.1 串口 IMU 发布 /imu。
     重要约束：正式 Cube-IMU 外参标定只允许 /imu 来自 Cube 上的串口 IMU，配置锁定为 imu_source=cube_serial、serial_port=/dev/ttyACM0。Orbbec 相机内置 IMU 是相机安装位，只能用于相机-IMU问题，不能当作 Cube IMU。

  2. data_recorder_node
     订阅图像、IMU、CameraInfo，按消息 header.stamp 写入 rosbag2。

  3. calibration_gui
     Qt GUI 节点。订阅真实图像、IMU、CameraInfo，实时显示相机画面和 IMU 六轴曲线；在 GUI 内部检测 AprilTag，在真实图像上叠加 Tag 边框、XYZ 轴和 Cube 外框，发布 /cube_pose 和 TF camera_link -> cube_link，保存位姿 CSV，并在录制过程中给出手持 Cube 的动作提示。

simulated_sensor_node 只用于无硬件时测试界面，不属于正式采集链路。


2. 环境依赖
-----------
Ubuntu 24.04 + ROS 2 Jazzy：

  source /opt/ros/jazzy/setup.bash
  sudo apt update
  sudo apt install -y \
    ros-jazzy-cv-bridge \
    ros-jazzy-tf2-ros \
    ros-jazzy-rosbag2-cpp \
    ros-jazzy-rosbag2-storage \
    ros-jazzy-rosbag2-storage-sqlite3 \
    libopencv-dev \
    libeigen3-dev \
    python3-pyqt5 \
    python3-numpy \
    python3-yaml

GUI 的 AprilTag 检测使用 OpenCV aruco 模块中的 DICT_APRILTAG_36h11 字典。
real_sensor_node 需要 Orbbec SDK；默认路径复用 04imu_cube_tracker 当前构建使用的 SDK：

  /home/wangjh/code/01历史代码/00历史代码/orbslam3_light/hardware/camera/orbbec_gemini_335/linux/third_party/pyorbbecsdk/sdk

如 SDK 放在其它位置，构建时传入：

  colcon build --packages-select cube_imu_calibration --cmake-args -DORBBEC_SDK_ROOT=/path/to/sdk


3. 编译
-------
把 cube_imu_calibration 放入 ROS 2 工作空间 src 目录后：

  cd ~/ros2_ws
  source /opt/ros/jazzy/setup.bash
  rosdep install --from-paths src --ignore-src -r -y
  colcon build --packages-select cube_imu_calibration --cmake-args -DCMAKE_BUILD_TYPE=Release
  source install/setup.bash

如果直接在当前目录作为工作空间构建：

  cd /home/wangjh/code/04_cube_imu_data_capture
  source /opt/ros/jazzy/setup.bash
  rosdep install --from-paths . --ignore-src -r -y
  colcon build --packages-select cube_imu_calibration --cmake-args -DCMAKE_BUILD_TYPE=Release
  source install/setup.bash


4. GUI 一键采集
---------------
推荐现场使用 GUI：

  ros2 launch cube_imu_calibration gui_launch.py

界面流程：

  1. 确认图像话题、IMU 话题、CameraInfo 话题、Tag 边长、Cube 边长、Tag ID、bag 路径、录制时长和 IMU 串口。IMU 来源固定为 cube_serial，也就是 Cube 上的 /dev/ttyACM0 串口 IMU。
     当前确认使用 “AprilTag 36h11 Cube - Left Hand Cube - IDs 0,1,2,3,4 - 30mm” 模板：
     Tag 黑色外框边长 0.0300 m，Cube 实测边长 0.0300 m，Tag ID=自动 0-4。
  2. 点击“启动采集准备”。GUI 会启动 real_sensor_node 和 data_recorder_node；real_sensor_node 负责真实 Orbbec 相机与 Cube 串口 IMU 发布，GUI 自己负责相机画面、IMU 波形和 AprilTag 检测。若 /dev/ttyACM0 未出现，GUI 会报 Cube 串口 IMU 未就绪，不会把 Orbbec 相机内置 IMU 当成 Cube IMU。
  3. 左侧检查 IMU 六轴曲线，中间检查真实相机画面，右侧检查图像、IMU、CameraInfo 发布者数量。
     IMU 面板参考 04imu_cube_tracker 的显示方式，同时给出加速度 g、角速度 deg/s、模长和稳定量程波形。
  4. 把 Cube 放入相机视野中央，AprilTag 完整清晰可见；“AprilTag / T_Cam_Cube”区域变绿后再开始录制。
  5. 点击“一键开始采集”。GUI 会等待新鲜图像、IMU、CameraInfo 和 AprilTag 位姿全部就绪后再触发录制；默认录制 150 秒，进度条显示百分比，到时自动停止。
  6. 录制时按“动作提示”和“动作示意”运动 Cube：向导会依次检查居中静止、绕 X/Y/Z 三轴旋转、平移+组合旋转、八字/斜向运动；AprilTag 出视野会立即红色报警。
     GUI 使用 Left Hand Cube 几何识别五个面 ID0=center、ID1=top、ID2=right、ID3=bottom、ID4=left，并统一发布 /cube_pose 的 T_Cam_Cube。双 Tag 同时可见时会显示单 Tag 一致性，用于现场判断模板手性、面编号和 Cube 边长是否正确。

如果 AprilTag 出视野或识别失败，GUI 顶部和动作提示会报警，先把 Cube 放回画面中央再继续。

如果没有真实相机/IMU，可点“启动仿真测试”测试界面。正式采集真实数据时不要同时启动仿真测试，避免两个数据源混在同一话题上。


5. 多节点 launch
----------------
也可以用一个 launch 同时启动 real_sensor_node、data_recorder_node 和 GUI：

  ros2 launch cube_imu_calibration calibration_launch.py \
    start_real_sensor:=true \
    use_gui:=true \
    image_topic:=/camera/image_raw \
    imu_topic:=/imu \
    camera_info_topic:=/camera/camera_info \
    marker_size:=0.03 \
    cube_visual_size:=0.030 \
    cube_layout:=left_hand_5face \
    target_tag_id:=-1 \
    serial_port:=/dev/ttyACM0 \
    bag_path:=output_bag \
    record_duration_sec:=150.0

如果只想启动录包节点：

  ros2 launch cube_imu_calibration runtime_launch.py \
    start_real_sensor:=false \
    image_topic:=/camera/image_raw \
    imu_topic:=/imu \
    camera_info_topic:=/camera/camera_info \
    bag_path:=output_bag \
    record_duration_sec:=150.0

如果只想启动真实传感器发布和录包准备，不启动 GUI：

  ros2 launch cube_imu_calibration runtime_launch.py \
    start_real_sensor:=true \
    serial_port:=/dev/ttyACM0 \
    bag_path:=output_bag


6. rosbag2 手动启停
-------------------
启动 data_recorder_node 后，手动开始录制：

  ros2 service call /data_recorder_node/start_recording std_srvs/srv/Trigger {}

停止录制：

  ros2 service call /data_recorder_node/stop_recording std_srvs/srv/Trigger {}

录制说明：

  - bag 默认路径为 output_bag，可通过 GUI 或 bag_path 修改。
  - 默认录制 /camera/image_raw、/imu、/camera/camera_info。
  - 若目标路径已存在，节点会自动追加时间戳后缀，避免覆盖旧数据。
  - 写入 rosbag2 的时间戳使用消息 header.stamp，不使用 DDS 接收时刻。
  - 若 header.stamp 为 0，默认跳过该消息并输出警告。


7. GUI 可视化内容
-----------------
GUI 会实时显示：

  - 真实相机画面，显示分辨率、编码、stamp、FPS、延迟。
  - 识别到 AprilTag 后，在真实相机画面上叠加 Tag 边框、XYZ 坐标轴和 Cube 外框；cube_visual_size 为 0 时外框按 Tag 边长画，也可设置为真实 Cube 边长。
  - IMU 数值遥测和六轴波形：加速度按 g 显示，角速度按 deg/s 显示，同时保留原始 m/s^2、rad/s 状态行、Hz 和延迟。
  - 实时数据流状态会同时检查发布者数量、实际接收频率和 CameraInfo 新鲜度；只发现发布者但没有稳定数据时会显示黄色等待状态。
  - AprilTag 是否在视野内，T_Cam_Cube 平移、四元数、矩阵首行。
  - data_recorder_node 在线状态、bag 路径、图像/IMU/CameraInfo 写入计数。
  - 录制进度条和分阶段运动提示；提示会根据 IMU 角速度积分、AprilTag 是否入画和平移范围自动推进。
  - /cube_pose 发布者数量；该话题由 calibration_gui 节点发布。

窗口启动时会按当前屏幕自动设置大小和三栏比例，默认白色界面。


8. AprilTag /cube_pose 输出
---------------------------
/cube_pose 自定义消息包含：

  header.frame_id             相机坐标系
  translation                 T_Cam_Cube 平移 x,y,z
  orientation                 T_Cam_Cube 四元数 x,y,z,w
  transform                   T_Cam_Cube 4x4 齐次矩阵，row-major

TF：

  parent_frame: camera_link
  child_frame:  cube_link

保存一次当前 AprilTag 检测结果：

  在 GUI 中点击“保存当前 T_Cam_Cube”

连续保存位姿序列：

  在 GUI 的“位姿 CSV”中设置路径，例如 tag_pose.csv。GUI 检测到 AprilTag 后会自动追加：

  timestamp,tx,ty,tz,qx,qy,qz,qw


9. 无硬件仿真跑通
-----------------
如果 ros2 topic list 只有 /parameter_events 和 /rosout，说明当前没有相机/IMU 驱动在发布数据。可先运行仿真 demo：

  ros2 launch cube_imu_calibration sim_demo_launch.py \
    start_recording:=true \
    record_duration_sec:=10.0 \
    use_gui:=true \
    pose_csv_path:=tag_pose.csv

该 demo 会启动 simulated_sensor_node、data_recorder_node 和 GUI。它只用于验证界面、录包、/cube_pose 和 TF 链路。


10. Kalibr + AprilTag 静态链路
------------------------------
当前主流程按这四步走：

  1. 同时采集相机图像、CameraInfo 和 IMU 到 rosbag2。
  2. 用 bag 跑 Kalibr，得到 T_Cam_IMU。
  3. 用 GUI/AprilTag 保存当前 T_Cam_Cube。
  4. 矩阵计算：

       T_Cube_IMU = inv(T_Cam_Cube) * T_Cam_IMU

采集：

  ros2 launch cube_imu_calibration gui_launch.py

GUI 中点击“启动采集准备”，确认图像、IMU、CameraInfo 都有新鲜数据，AprilTag 可识别；界面下方“动作示意”会提示 Cube/刚体运动方式。点击“一键开始采集”后生成 bag，同时 GUI 可保存 T_Cam_Cube：

  保存当前 T_Cam_Cube -> cam_cube_pose.yaml

GUI 会在“时间戳诊断”中显示 image/imu/camera_info 的 header.stamp 差值；如果 header.stamp 为 0、倒退，或三路时间差过大，会报警并阻止直接开始录制。

为了保证录包稳定，GUI 只做降频预览和降频 AprilTag 检测，默认 5Hz；原始图像、IMU、CameraInfo 仍由 data_recorder_node 全量写入 rosbag2。如果屏幕刷新仍然卡，可降低 GUI 负载：

  ros2 launch cube_imu_calibration gui_launch.py \
    image_preview_rate_hz:=2.0 \
    tag_detection_rate_hz:=2.0

时间戳：

  data_recorder_node 按每条消息的 header.stamp 写入 rosbag2，不按 DDS 接收时刻写入。
  real_sensor_node 当前对相机帧和串口 IMU 都使用系统时间戳；外部驱动也必须保证图像和 IMU 的 header.stamp 来自同一时间基准，或至少只有稳定常量偏移。
  如果 header.stamp 为 0，默认跳过该消息，避免 Kalibr 读到无效时间。

Kalibr 示例命令，参数文件按你的相机模型、IMU 噪声和标定板实际配置填写：

  ros2 run cube_imu_calibration kalibr_workflow_check.py \
    --bag output_bag \
    --cam camchain.yaml \
    --imu imu.yaml \
    --target aprilgrid.yaml \
    --cam-cube cam_cube_pose.yaml \
    --kalibr-result kalibr_result.yaml

  kalibr_calibrate_imu_camera \
    --bag output_bag \
    --cam camchain.yaml \
    --imu imu.yaml \
    --target aprilgrid.yaml \
    --topics /camera/image_raw /imu

如果你使用的是只支持 ROS1 bag 的原版 Kalibr，需要先把 rosbag2 转成 ROS1 bag，再运行 Kalibr。

Kalibr 输出中取 T_Cam_IMU，例如 camchain 里的 cam0.T_cam_imu。然后计算 Cube-IMU 外参：

  ros2 run cube_imu_calibration solve_extrinsics.py \
    --cam-imu kalibr_result.yaml \
    --cam-cube cam_cube_pose.yaml \
    --output cube_imu_extrinsics.txt

输出包含：

  - T_Cube_IMU
  - R_Cube_IMU
  - t_Cube_IMU
  - q_Cube_IMU_xyzw
  - T_IMU_Cube 逆矩阵


11. 采集时长和动作建议
----------------------
正式标定建议每组录制 60 到 90 秒。3 秒只够验证链路，不适合作为标定数据。

推荐动作：

  - 相机和 IMU 必须刚性固定，共同运动；不要让二者之间有相对位移。
  - 用 Kalibr 标定板时，让相机持续看到足够多的标定板姿态和视野位置。
  - 做绕 X/Y/Z 三轴的缓慢旋转，并组合小幅左右、上下、前后平移。
  - 动作平稳连贯，避免快速甩动、运动模糊和 AprilTag/标定板出画。
  - 开头和结尾各静止 2 秒。

建议至少采集 3 组数据分别求解；如果 R/t 差异大，优先检查时间同步、IMU 串口数据、CameraInfo、Kalibr 标定板配置、Tag 尺寸和刚性固定。


12. pose-IMU 备用工具
---------------------
如果真实结构不是“相机和 IMU 同一刚体”，而是头戴相机观察手持 Cube/IMU，则不要使用 Kalibr 静态链路。此时可用 pose_imu_calibrator.py 输入 AprilTag/Cube 位姿 CSV 和 IMU CSV，输出 T_Tag_IMU 初值。

求解示例：

  ros2 run cube_imu_calibration pose_imu_calibrator.py \
    --pose-csv tag_pose.csv \
    --imu-csv imu.csv \
    --time-offset-min -0.05 \
    --time-offset-max 0.05 \
    --time-offset-step 0.002 \
    --output tag_imu_extrinsics.txt
