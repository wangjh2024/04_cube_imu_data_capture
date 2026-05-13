from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.conditions import IfCondition
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution
from launch_ros.actions import Node
from launch_ros.parameter_descriptions import ParameterValue
from launch_ros.substitutions import FindPackageShare


def generate_launch_description():
    pkg_share = FindPackageShare("cube_imu_calibration")
    default_config = PathJoinSubstitution([pkg_share, "config", "apriltag_params.yaml"])

    image_topic = LaunchConfiguration("image_topic")
    imu_topic = LaunchConfiguration("imu_topic")
    camera_info_topic = LaunchConfiguration("camera_info_topic")
    cube_pose_topic = LaunchConfiguration("cube_pose_topic")
    cube_pose_status_topic = LaunchConfiguration("cube_pose_status_topic")
    bag_path = LaunchConfiguration("bag_path")
    start_recording = LaunchConfiguration("start_recording")
    record_duration_sec = LaunchConfiguration("record_duration_sec")
    recorder_record_duration_sec = LaunchConfiguration("recorder_record_duration_sec")
    camera_frame = LaunchConfiguration("camera_frame")
    use_gui = LaunchConfiguration("use_gui")
    start_real_sensor = LaunchConfiguration("start_real_sensor")
    imu_frame = LaunchConfiguration("imu_frame")
    imu_source = LaunchConfiguration("imu_source")
    serial_port = LaunchConfiguration("serial_port")
    serial_baudrate = LaunchConfiguration("serial_baudrate")
    serial_protocol_mode = LaunchConfiguration("serial_protocol_mode")
    serial_imu_rate_hz = LaunchConfiguration("serial_imu_rate_hz")
    serial_startup_command = LaunchConfiguration("serial_startup_command")
    camera_output_encoding = LaunchConfiguration("camera_output_encoding")
    color_fps = LaunchConfiguration("color_fps")
    real_sensor_image_qos_depth = LaunchConfiguration("real_sensor_image_qos_depth")
    real_sensor_imu_qos_depth = LaunchConfiguration("real_sensor_imu_qos_depth")
    recorder_image_qos_depth = LaunchConfiguration("recorder_image_qos_depth")
    recorder_imu_qos_depth = LaunchConfiguration("recorder_imu_qos_depth")
    recorder_status_qos_depth = LaunchConfiguration("recorder_status_qos_depth")
    image_preview_rate_hz = LaunchConfiguration("image_preview_rate_hz")
    enable_cube_detection = LaunchConfiguration("enable_cube_detection")
    marker_size = LaunchConfiguration("marker_size")
    cube_visual_size = LaunchConfiguration("cube_visual_size")
    cube_layout = LaunchConfiguration("cube_layout")
    tag_detection_rate_hz = LaunchConfiguration("tag_detection_rate_hz")
    tag_detection_scale = LaunchConfiguration("tag_detection_scale")
    preview_scale = LaunchConfiguration("preview_scale")
    gui_display_rate_hz = LaunchConfiguration("gui_display_rate_hz")
    draw_preview_overlay = LaunchConfiguration("draw_preview_overlay")
    pose_csv_path = LaunchConfiguration("pose_csv_path")

    return LaunchDescription([
        DeclareLaunchArgument("config_file", default_value=default_config),
        DeclareLaunchArgument("image_topic", default_value="/camera/image_raw"),
        DeclareLaunchArgument("imu_topic", default_value="/imu"),
        DeclareLaunchArgument("camera_info_topic", default_value="/camera/camera_info"),
        DeclareLaunchArgument("cube_pose_topic", default_value="/cube_pose"),
        DeclareLaunchArgument("cube_pose_status_topic", default_value="/cube_pose_status"),
        DeclareLaunchArgument("bag_path", default_value="output_bag"),
        DeclareLaunchArgument("start_recording", default_value="false"),
        DeclareLaunchArgument("record_duration_sec", default_value="150.0"),
        DeclareLaunchArgument("recorder_record_duration_sec", default_value="0.0"),
        DeclareLaunchArgument("camera_frame", default_value="camera_link"),
        DeclareLaunchArgument("use_gui", default_value="false"),
        DeclareLaunchArgument("start_real_sensor", default_value="false"),
        DeclareLaunchArgument("imu_frame", default_value="imu_link"),
        DeclareLaunchArgument("imu_source", default_value="cube_serial"),
        DeclareLaunchArgument("serial_port", default_value="/dev/ttyACM0"),
        DeclareLaunchArgument("serial_baudrate", default_value="2000000"),
        DeclareLaunchArgument("serial_protocol_mode", default_value="v2.1"),
        DeclareLaunchArgument("serial_imu_rate_hz", default_value="500.0"),
        DeclareLaunchArgument("serial_startup_command", default_value="uartout"),
        DeclareLaunchArgument("camera_output_encoding", default_value="mono8"),
        DeclareLaunchArgument("color_fps", default_value="15"),
        DeclareLaunchArgument("real_sensor_image_qos_depth", default_value="30"),
        DeclareLaunchArgument("real_sensor_imu_qos_depth", default_value="1000"),
        DeclareLaunchArgument("recorder_image_qos_depth", default_value="120"),
        DeclareLaunchArgument("recorder_imu_qos_depth", default_value="2000"),
        DeclareLaunchArgument("recorder_status_qos_depth", default_value="100"),
        DeclareLaunchArgument("image_preview_rate_hz", default_value="5.0"),
        DeclareLaunchArgument("enable_cube_detection", default_value="true"),
        DeclareLaunchArgument("marker_size", default_value="0.030"),
        DeclareLaunchArgument("cube_visual_size", default_value="0.033"),
        DeclareLaunchArgument("cube_layout", default_value="left_hand_5face"),
        DeclareLaunchArgument("tag_detection_rate_hz", default_value="15.0"),
        DeclareLaunchArgument("tag_detection_scale", default_value="0.5"),
        DeclareLaunchArgument("preview_scale", default_value="0.5"),
        DeclareLaunchArgument("gui_display_rate_hz", default_value="5.0"),
        DeclareLaunchArgument("draw_preview_overlay", default_value="true"),
        DeclareLaunchArgument("pose_csv_path", default_value="tag_pose.csv"),

        Node(
            package="cube_imu_calibration",
            executable="real_sensor_node",
            name="real_sensor_node",
            output="screen",
            condition=IfCondition(start_real_sensor),
            parameters=[
                LaunchConfiguration("config_file"),
                {
                    "image_topic": image_topic,
                    "imu_topic": imu_topic,
                    "camera_info_topic": camera_info_topic,
                    "camera_frame": camera_frame,
                    "imu_frame": imu_frame,
                    "imu_source": imu_source,
                    "serial_port": serial_port,
                    "serial_baudrate": ParameterValue(serial_baudrate, value_type=int),
                    "serial_protocol_mode": serial_protocol_mode,
                    "serial_imu_rate_hz": ParameterValue(serial_imu_rate_hz, value_type=float),
                    "serial_startup_command": serial_startup_command,
                    "output_encoding": camera_output_encoding,
                    "color_fps": ParameterValue(color_fps, value_type=int),
                    "image_qos_depth": ParameterValue(
                        real_sensor_image_qos_depth,
                        value_type=int,
                    ),
                    "imu_qos_depth": ParameterValue(real_sensor_imu_qos_depth, value_type=int),
                },
            ],
        ),

        Node(
            package="cube_imu_calibration",
            executable="data_recorder_node",
            name="data_recorder_node",
            output="screen",
            parameters=[
                LaunchConfiguration("config_file"),
                {
                    "image_topic": image_topic,
                    "imu_topic": imu_topic,
                    "camera_info_topic": camera_info_topic,
                    "cube_pose_topic": cube_pose_topic,
                    "cube_pose_status_topic": cube_pose_status_topic,
                    "bag_path": bag_path,
                    "start_recording": ParameterValue(start_recording, value_type=bool),
                    "record_duration_sec": ParameterValue(
                        recorder_record_duration_sec,
                        value_type=float,
                    ),
                    "image_qos_depth": ParameterValue(recorder_image_qos_depth, value_type=int),
                    "imu_qos_depth": ParameterValue(recorder_imu_qos_depth, value_type=int),
                    "status_qos_depth": ParameterValue(recorder_status_qos_depth, value_type=int),
                },
            ],
        ),

        Node(
            package="cube_imu_calibration",
            executable="calibration_gui.py",
            name="calibration_gui",
            output="screen",
            condition=IfCondition(use_gui),
            parameters=[
                LaunchConfiguration("config_file"),
                {
                    "image_topic": image_topic,
                    "imu_topic": imu_topic,
                    "camera_info_topic": camera_info_topic,
                    "cube_pose_topic": cube_pose_topic,
                    "cube_pose_status_topic": cube_pose_status_topic,
                    "bag_path": bag_path,
                    "record_duration_sec": ParameterValue(record_duration_sec, value_type=float),
                    "camera_frame": camera_frame,
                    "imu_frame": imu_frame,
                    "imu_source": imu_source,
                    "serial_port": serial_port,
                    "serial_baudrate": ParameterValue(serial_baudrate, value_type=int),
                    "serial_protocol_mode": serial_protocol_mode,
                    "serial_imu_rate_hz": ParameterValue(serial_imu_rate_hz, value_type=float),
                    "serial_startup_command": serial_startup_command,
                    "camera_output_encoding": camera_output_encoding,
                    "color_fps": ParameterValue(color_fps, value_type=int),
                    "real_sensor_image_qos_depth": ParameterValue(
                        real_sensor_image_qos_depth,
                        value_type=int,
                    ),
                    "real_sensor_imu_qos_depth": ParameterValue(
                        real_sensor_imu_qos_depth,
                        value_type=int,
                    ),
                    "recorder_image_qos_depth": ParameterValue(
                        recorder_image_qos_depth,
                        value_type=int,
                    ),
                    "recorder_imu_qos_depth": ParameterValue(
                        recorder_imu_qos_depth,
                        value_type=int,
                    ),
                    "recorder_status_qos_depth": ParameterValue(
                        recorder_status_qos_depth,
                        value_type=int,
                    ),
                    "image_preview_rate_hz": ParameterValue(image_preview_rate_hz, value_type=float),
                    "enable_cube_detection": ParameterValue(enable_cube_detection, value_type=bool),
                    "marker_size": ParameterValue(marker_size, value_type=float),
                    "cube_visual_size": ParameterValue(cube_visual_size, value_type=float),
                    "cube_layout": cube_layout,
                    "tag_detection_rate_hz": ParameterValue(tag_detection_rate_hz, value_type=float),
                    "tag_detection_scale": ParameterValue(tag_detection_scale, value_type=float),
                    "preview_scale": ParameterValue(preview_scale, value_type=float),
                    "gui_display_rate_hz": ParameterValue(gui_display_rate_hz, value_type=float),
                    "draw_preview_overlay": ParameterValue(
                        draw_preview_overlay,
                        value_type=bool,
                    ),
                    "pose_csv_path": pose_csv_path,
                    "recorder_status_topic": "/data_recorder_node/status",
                    "start_service": "/data_recorder_node/start_recording",
                    "stop_service": "/data_recorder_node/stop_recording",
                },
            ],
        ),
    ])
