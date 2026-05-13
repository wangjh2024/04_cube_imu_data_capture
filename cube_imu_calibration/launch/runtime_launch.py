from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.conditions import IfCondition
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node
from launch_ros.parameter_descriptions import ParameterValue


def generate_launch_description():
    image_topic = LaunchConfiguration("image_topic")
    imu_topic = LaunchConfiguration("imu_topic")
    camera_info_topic = LaunchConfiguration("camera_info_topic")
    cube_pose_topic = LaunchConfiguration("cube_pose_topic")
    cube_pose_status_topic = LaunchConfiguration("cube_pose_status_topic")
    bag_path = LaunchConfiguration("bag_path")
    start_recording = LaunchConfiguration("start_recording")
    record_duration_sec = LaunchConfiguration("record_duration_sec")
    start_real_sensor = LaunchConfiguration("start_real_sensor")
    camera_frame = LaunchConfiguration("camera_frame")
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

    return LaunchDescription([
        DeclareLaunchArgument("image_topic", default_value="/camera/image_raw"),
        DeclareLaunchArgument("imu_topic", default_value="/imu"),
        DeclareLaunchArgument("camera_info_topic", default_value="/camera/camera_info"),
        DeclareLaunchArgument("cube_pose_topic", default_value="/cube_pose"),
        DeclareLaunchArgument("cube_pose_status_topic", default_value="/cube_pose_status"),
        DeclareLaunchArgument("bag_path", default_value="output_bag"),
        DeclareLaunchArgument("start_recording", default_value="false"),
        DeclareLaunchArgument("record_duration_sec", default_value="0.0"),
        DeclareLaunchArgument("start_real_sensor", default_value="false"),
        DeclareLaunchArgument("camera_frame", default_value="camera_link"),
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

        Node(
            package="cube_imu_calibration",
            executable="real_sensor_node",
            name="real_sensor_node",
            output="screen",
            condition=IfCondition(start_real_sensor),
            parameters=[{
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
                "image_qos_depth": ParameterValue(real_sensor_image_qos_depth, value_type=int),
                "imu_qos_depth": ParameterValue(real_sensor_imu_qos_depth, value_type=int),
            }],
        ),

        Node(
            package="cube_imu_calibration",
            executable="data_recorder_node",
            name="data_recorder_node",
            output="screen",
            parameters=[{
                "image_topic": image_topic,
                "imu_topic": imu_topic,
                "camera_info_topic": camera_info_topic,
                "cube_pose_topic": cube_pose_topic,
                "cube_pose_status_topic": cube_pose_status_topic,
                "bag_path": bag_path,
                "start_recording": ParameterValue(start_recording, value_type=bool),
                "record_duration_sec": ParameterValue(record_duration_sec, value_type=float),
                "status_period_sec": 1.0,
                "image_qos_depth": ParameterValue(recorder_image_qos_depth, value_type=int),
                "imu_qos_depth": ParameterValue(recorder_imu_qos_depth, value_type=int),
                "status_qos_depth": ParameterValue(recorder_status_qos_depth, value_type=int),
            }],
        ),
    ])
