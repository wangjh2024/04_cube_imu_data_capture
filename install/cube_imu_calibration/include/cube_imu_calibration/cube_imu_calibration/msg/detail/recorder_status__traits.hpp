// generated from rosidl_generator_cpp/resource/idl__traits.hpp.em
// with input from cube_imu_calibration:msg/RecorderStatus.idl
// generated code does not contain a copyright notice

// IWYU pragma: private, include "cube_imu_calibration/msg/recorder_status.hpp"


#ifndef CUBE_IMU_CALIBRATION__MSG__DETAIL__RECORDER_STATUS__TRAITS_HPP_
#define CUBE_IMU_CALIBRATION__MSG__DETAIL__RECORDER_STATUS__TRAITS_HPP_

#include <stdint.h>

#include <sstream>
#include <string>
#include <type_traits>

#include "cube_imu_calibration/msg/detail/recorder_status__struct.hpp"
#include "rosidl_runtime_cpp/traits.hpp"

// Include directives for member types
// Member 'header'
#include "std_msgs/msg/detail/header__traits.hpp"

namespace cube_imu_calibration
{

namespace msg
{

inline void to_flow_style_yaml(
  const RecorderStatus & msg,
  std::ostream & out)
{
  out << "{";
  // member: header
  {
    out << "header: ";
    to_flow_style_yaml(msg.header, out);
    out << ", ";
  }

  // member: recording
  {
    out << "recording: ";
    rosidl_generator_traits::value_to_yaml(msg.recording, out);
    out << ", ";
  }

  // member: bag_path
  {
    out << "bag_path: ";
    rosidl_generator_traits::value_to_yaml(msg.bag_path, out);
    out << ", ";
  }

  // member: image_count
  {
    out << "image_count: ";
    rosidl_generator_traits::value_to_yaml(msg.image_count, out);
    out << ", ";
  }

  // member: imu_count
  {
    out << "imu_count: ";
    rosidl_generator_traits::value_to_yaml(msg.imu_count, out);
    out << ", ";
  }

  // member: camera_info_count
  {
    out << "camera_info_count: ";
    rosidl_generator_traits::value_to_yaml(msg.camera_info_count, out);
    out << ", ";
  }

  // member: cube_pose_count
  {
    out << "cube_pose_count: ";
    rosidl_generator_traits::value_to_yaml(msg.cube_pose_count, out);
    out << ", ";
  }

  // member: cube_pose_status_count
  {
    out << "cube_pose_status_count: ";
    rosidl_generator_traits::value_to_yaml(msg.cube_pose_status_count, out);
    out << ", ";
  }

  // member: image_publishers
  {
    out << "image_publishers: ";
    rosidl_generator_traits::value_to_yaml(msg.image_publishers, out);
    out << ", ";
  }

  // member: imu_publishers
  {
    out << "imu_publishers: ";
    rosidl_generator_traits::value_to_yaml(msg.imu_publishers, out);
    out << ", ";
  }

  // member: camera_info_publishers
  {
    out << "camera_info_publishers: ";
    rosidl_generator_traits::value_to_yaml(msg.camera_info_publishers, out);
    out << ", ";
  }

  // member: cube_pose_publishers
  {
    out << "cube_pose_publishers: ";
    rosidl_generator_traits::value_to_yaml(msg.cube_pose_publishers, out);
    out << ", ";
  }

  // member: cube_pose_status_publishers
  {
    out << "cube_pose_status_publishers: ";
    rosidl_generator_traits::value_to_yaml(msg.cube_pose_status_publishers, out);
    out << ", ";
  }

  // member: elapsed_sec
  {
    out << "elapsed_sec: ";
    rosidl_generator_traits::value_to_yaml(msg.elapsed_sec, out);
    out << ", ";
  }

  // member: target_duration_sec
  {
    out << "target_duration_sec: ";
    rosidl_generator_traits::value_to_yaml(msg.target_duration_sec, out);
    out << ", ";
  }

  // member: progress_ratio
  {
    out << "progress_ratio: ";
    rosidl_generator_traits::value_to_yaml(msg.progress_ratio, out);
  }
  out << "}";
}  // NOLINT(readability/fn_size)

inline void to_block_style_yaml(
  const RecorderStatus & msg,
  std::ostream & out, size_t indentation = 0)
{
  // member: header
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "header:\n";
    to_block_style_yaml(msg.header, out, indentation + 2);
  }

  // member: recording
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "recording: ";
    rosidl_generator_traits::value_to_yaml(msg.recording, out);
    out << "\n";
  }

  // member: bag_path
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "bag_path: ";
    rosidl_generator_traits::value_to_yaml(msg.bag_path, out);
    out << "\n";
  }

  // member: image_count
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "image_count: ";
    rosidl_generator_traits::value_to_yaml(msg.image_count, out);
    out << "\n";
  }

  // member: imu_count
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "imu_count: ";
    rosidl_generator_traits::value_to_yaml(msg.imu_count, out);
    out << "\n";
  }

  // member: camera_info_count
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "camera_info_count: ";
    rosidl_generator_traits::value_to_yaml(msg.camera_info_count, out);
    out << "\n";
  }

  // member: cube_pose_count
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "cube_pose_count: ";
    rosidl_generator_traits::value_to_yaml(msg.cube_pose_count, out);
    out << "\n";
  }

  // member: cube_pose_status_count
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "cube_pose_status_count: ";
    rosidl_generator_traits::value_to_yaml(msg.cube_pose_status_count, out);
    out << "\n";
  }

  // member: image_publishers
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "image_publishers: ";
    rosidl_generator_traits::value_to_yaml(msg.image_publishers, out);
    out << "\n";
  }

  // member: imu_publishers
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "imu_publishers: ";
    rosidl_generator_traits::value_to_yaml(msg.imu_publishers, out);
    out << "\n";
  }

  // member: camera_info_publishers
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "camera_info_publishers: ";
    rosidl_generator_traits::value_to_yaml(msg.camera_info_publishers, out);
    out << "\n";
  }

  // member: cube_pose_publishers
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "cube_pose_publishers: ";
    rosidl_generator_traits::value_to_yaml(msg.cube_pose_publishers, out);
    out << "\n";
  }

  // member: cube_pose_status_publishers
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "cube_pose_status_publishers: ";
    rosidl_generator_traits::value_to_yaml(msg.cube_pose_status_publishers, out);
    out << "\n";
  }

  // member: elapsed_sec
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "elapsed_sec: ";
    rosidl_generator_traits::value_to_yaml(msg.elapsed_sec, out);
    out << "\n";
  }

  // member: target_duration_sec
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "target_duration_sec: ";
    rosidl_generator_traits::value_to_yaml(msg.target_duration_sec, out);
    out << "\n";
  }

  // member: progress_ratio
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "progress_ratio: ";
    rosidl_generator_traits::value_to_yaml(msg.progress_ratio, out);
    out << "\n";
  }
}  // NOLINT(readability/fn_size)

inline std::string to_yaml(const RecorderStatus & msg, bool use_flow_style = false)
{
  std::ostringstream out;
  if (use_flow_style) {
    to_flow_style_yaml(msg, out);
  } else {
    to_block_style_yaml(msg, out);
  }
  return out.str();
}

}  // namespace msg

}  // namespace cube_imu_calibration

namespace rosidl_generator_traits
{

[[deprecated("use cube_imu_calibration::msg::to_block_style_yaml() instead")]]
inline void to_yaml(
  const cube_imu_calibration::msg::RecorderStatus & msg,
  std::ostream & out, size_t indentation = 0)
{
  cube_imu_calibration::msg::to_block_style_yaml(msg, out, indentation);
}

[[deprecated("use cube_imu_calibration::msg::to_yaml() instead")]]
inline std::string to_yaml(const cube_imu_calibration::msg::RecorderStatus & msg)
{
  return cube_imu_calibration::msg::to_yaml(msg);
}

template<>
inline const char * data_type<cube_imu_calibration::msg::RecorderStatus>()
{
  return "cube_imu_calibration::msg::RecorderStatus";
}

template<>
inline const char * name<cube_imu_calibration::msg::RecorderStatus>()
{
  return "cube_imu_calibration/msg/RecorderStatus";
}

template<>
struct has_fixed_size<cube_imu_calibration::msg::RecorderStatus>
  : std::integral_constant<bool, false> {};

template<>
struct has_bounded_size<cube_imu_calibration::msg::RecorderStatus>
  : std::integral_constant<bool, false> {};

template<>
struct is_message<cube_imu_calibration::msg::RecorderStatus>
  : std::true_type {};

}  // namespace rosidl_generator_traits

#endif  // CUBE_IMU_CALIBRATION__MSG__DETAIL__RECORDER_STATUS__TRAITS_HPP_
