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
    bag_path = LaunchConfiguration("bag_path")
    tag_id = LaunchConfiguration("tag_id")
    start_recording = LaunchConfiguration("start_recording")
    record_duration_sec = LaunchConfiguration("record_duration_sec")
    camera_frame = LaunchConfiguration("camera_frame")
    use_gui = LaunchConfiguration("use_gui")
    image_preview_rate_hz = LaunchConfiguration("image_preview_rate_hz")

    return LaunchDescription([
        DeclareLaunchArgument("image_topic", default_value="/camera/image_raw"),
        DeclareLaunchArgument("imu_topic", default_value="/imu"),
        DeclareLaunchArgument("camera_info_topic", default_value="/camera/camera_info"),
        DeclareLaunchArgument("bag_path", default_value="output_bag_sim"),
        DeclareLaunchArgument("tag_id", default_value="0"),
        DeclareLaunchArgument("start_recording", default_value="false"),
        DeclareLaunchArgument("record_duration_sec", default_value="10.0"),
        DeclareLaunchArgument("camera_frame", default_value="camera_link"),
        DeclareLaunchArgument("use_gui", default_value="false"),
        DeclareLaunchArgument("image_preview_rate_hz", default_value="12.0"),

        Node(
            package="cube_imu_calibration",
            executable="simulated_sensor_node",
            name="simulated_sensor_node",
            output="screen",
            parameters=[{
                "image_topic": image_topic,
                "imu_topic": imu_topic,
                "camera_info_topic": camera_info_topic,
                "camera_frame": camera_frame,
                "tag_id": ParameterValue(tag_id, value_type=int),
                "dictionary": "DICT_APRILTAG_36h11",
                "width": 640,
                "height": 480,
                "marker_pixels": 160,
                "fx": 600.0,
                "fy": 600.0,
                "cx": 320.0,
                "cy": 240.0,
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
                "bag_path": bag_path,
                "start_recording": ParameterValue(start_recording, value_type=bool),
                "record_duration_sec": ParameterValue(record_duration_sec, value_type=float),
                "status_period_sec": 2.0,
            }],
        ),

        Node(
            package="cube_imu_calibration",
            executable="calibration_gui.py",
            name="calibration_gui",
            output="screen",
            condition=IfCondition(use_gui),
            parameters=[{
                "image_topic": image_topic,
                "imu_topic": imu_topic,
                "camera_info_topic": camera_info_topic,
                "bag_path": bag_path,
                "record_duration_sec": ParameterValue(record_duration_sec, value_type=float),
                "camera_frame": camera_frame,
                "image_preview_rate_hz": ParameterValue(image_preview_rate_hz, value_type=float),
                "recorder_status_topic": "/data_recorder_node/status",
                "start_service": "/data_recorder_node/start_recording",
                "stop_service": "/data_recorder_node/stop_recording",
            }],
        ),
    ])
