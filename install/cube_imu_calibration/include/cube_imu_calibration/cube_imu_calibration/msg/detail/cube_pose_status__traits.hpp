// generated from rosidl_generator_cpp/resource/idl__traits.hpp.em
// with input from cube_imu_calibration:msg/CubePoseStatus.idl
// generated code does not contain a copyright notice

// IWYU pragma: private, include "cube_imu_calibration/msg/cube_pose_status.hpp"


#ifndef CUBE_IMU_CALIBRATION__MSG__DETAIL__CUBE_POSE_STATUS__TRAITS_HPP_
#define CUBE_IMU_CALIBRATION__MSG__DETAIL__CUBE_POSE_STATUS__TRAITS_HPP_

#include <stdint.h>

#include <sstream>
#include <string>
#include <type_traits>

#include "cube_imu_calibration/msg/detail/cube_pose_status__struct.hpp"
#include "rosidl_runtime_cpp/traits.hpp"

// Include directives for member types
// Member 'header'
#include "std_msgs/msg/detail/header__traits.hpp"

namespace cube_imu_calibration
{

namespace msg
{

inline void to_flow_style_yaml(
  const CubePoseStatus & msg,
  std::ostream & out)
{
  out << "{";
  // member: header
  {
    out << "header: ";
    to_flow_style_yaml(msg.header, out);
    out << ", ";
  }

  // member: pose_valid
  {
    out << "pose_valid: ";
    rosidl_generator_traits::value_to_yaml(msg.pose_valid, out);
    out << ", ";
  }

  // member: tag_count
  {
    out << "tag_count: ";
    rosidl_generator_traits::value_to_yaml(msg.tag_count, out);
    out << ", ";
  }

  // member: tag_ids
  {
    if (msg.tag_ids.size() == 0) {
      out << "tag_ids: []";
    } else {
      out << "tag_ids: [";
      size_t pending_items = msg.tag_ids.size();
      for (auto item : msg.tag_ids) {
        rosidl_generator_traits::value_to_yaml(item, out);
        if (--pending_items > 0) {
          out << ", ";
        }
      }
      out << "]";
    }
    out << ", ";
  }

  // member: reproj_error
  {
    out << "reproj_error: ";
    rosidl_generator_traits::value_to_yaml(msg.reproj_error, out);
    out << ", ";
  }

  // member: message
  {
    out << "message: ";
    rosidl_generator_traits::value_to_yaml(msg.message, out);
  }
  out << "}";
}  // NOLINT(readability/fn_size)

inline void to_block_style_yaml(
  const CubePoseStatus & msg,
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

  // member: pose_valid
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "pose_valid: ";
    rosidl_generator_traits::value_to_yaml(msg.pose_valid, out);
    out << "\n";
  }

  // member: tag_count
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "tag_count: ";
    rosidl_generator_traits::value_to_yaml(msg.tag_count, out);
    out << "\n";
  }

  // member: tag_ids
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    if (msg.tag_ids.size() == 0) {
      out << "tag_ids: []\n";
    } else {
      out << "tag_ids:\n";
      for (auto item : msg.tag_ids) {
        if (indentation > 0) {
          out << std::string(indentation, ' ');
        }
        out << "- ";
        rosidl_generator_traits::value_to_yaml(item, out);
        out << "\n";
      }
    }
  }

  // member: reproj_error
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "reproj_error: ";
    rosidl_generator_traits::value_to_yaml(msg.reproj_error, out);
    out << "\n";
  }

  // member: message
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "message: ";
    rosidl_generator_traits::value_to_yaml(msg.message, out);
    out << "\n";
  }
}  // NOLINT(readability/fn_size)

inline std::string to_yaml(const CubePoseStatus & msg, bool use_flow_style = false)
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
  const cube_imu_calibration::msg::CubePoseStatus & msg,
  std::ostream & out, size_t indentation = 0)
{
  cube_imu_calibration::msg::to_block_style_yaml(msg, out, indentation);
}

[[deprecated("use cube_imu_calibration::msg::to_yaml() instead")]]
inline std::string to_yaml(const cube_imu_calibration::msg::CubePoseStatus & msg)
{
  return cube_imu_calibration::msg::to_yaml(msg);
}

template<>
inline const char * data_type<cube_imu_calibration::msg::CubePoseStatus>()
{
  return "cube_imu_calibration::msg::CubePoseStatus";
}

template<>
inline const char * name<cube_imu_calibration::msg::CubePoseStatus>()
{
  return "cube_imu_calibration/msg/CubePoseStatus";
}

template<>
struct has_fixed_size<cube_imu_calibration::msg::CubePoseStatus>
  : std::integral_constant<bool, false> {};

template<>
struct has_bounded_size<cube_imu_calibration::msg::CubePoseStatus>
  : std::integral_constant<bool, false> {};

template<>
struct is_message<cube_imu_calibration::msg::CubePoseStatus>
  : std::true_type {};

}  // namespace rosidl_generator_traits

#endif  // CUBE_IMU_CALIBRATION__MSG__DETAIL__CUBE_POSE_STATUS__TRAITS_HPP_
