// generated from rosidl_generator_cpp/resource/idl__traits.hpp.em
// with input from cube_imu_calibration:msg/CubePose.idl
// generated code does not contain a copyright notice

// IWYU pragma: private, include "cube_imu_calibration/msg/cube_pose.hpp"


#ifndef CUBE_IMU_CALIBRATION__MSG__DETAIL__CUBE_POSE__TRAITS_HPP_
#define CUBE_IMU_CALIBRATION__MSG__DETAIL__CUBE_POSE__TRAITS_HPP_

#include <stdint.h>

#include <sstream>
#include <string>
#include <type_traits>

#include "cube_imu_calibration/msg/detail/cube_pose__struct.hpp"
#include "rosidl_runtime_cpp/traits.hpp"

// Include directives for member types
// Member 'header'
#include "std_msgs/msg/detail/header__traits.hpp"
// Member 'translation'
#include "geometry_msgs/msg/detail/vector3__traits.hpp"
// Member 'orientation'
#include "geometry_msgs/msg/detail/quaternion__traits.hpp"

namespace cube_imu_calibration
{

namespace msg
{

inline void to_flow_style_yaml(
  const CubePose & msg,
  std::ostream & out)
{
  out << "{";
  // member: header
  {
    out << "header: ";
    to_flow_style_yaml(msg.header, out);
    out << ", ";
  }

  // member: translation
  {
    out << "translation: ";
    to_flow_style_yaml(msg.translation, out);
    out << ", ";
  }

  // member: orientation
  {
    out << "orientation: ";
    to_flow_style_yaml(msg.orientation, out);
    out << ", ";
  }

  // member: transform
  {
    if (msg.transform.size() == 0) {
      out << "transform: []";
    } else {
      out << "transform: [";
      size_t pending_items = msg.transform.size();
      for (auto item : msg.transform) {
        rosidl_generator_traits::value_to_yaml(item, out);
        if (--pending_items > 0) {
          out << ", ";
        }
      }
      out << "]";
    }
  }
  out << "}";
}  // NOLINT(readability/fn_size)

inline void to_block_style_yaml(
  const CubePose & msg,
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

  // member: translation
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "translation:\n";
    to_block_style_yaml(msg.translation, out, indentation + 2);
  }

  // member: orientation
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "orientation:\n";
    to_block_style_yaml(msg.orientation, out, indentation + 2);
  }

  // member: transform
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    if (msg.transform.size() == 0) {
      out << "transform: []\n";
    } else {
      out << "transform:\n";
      for (auto item : msg.transform) {
        if (indentation > 0) {
          out << std::string(indentation, ' ');
        }
        out << "- ";
        rosidl_generator_traits::value_to_yaml(item, out);
        out << "\n";
      }
    }
  }
}  // NOLINT(readability/fn_size)

inline std::string to_yaml(const CubePose & msg, bool use_flow_style = false)
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
  const cube_imu_calibration::msg::CubePose & msg,
  std::ostream & out, size_t indentation = 0)
{
  cube_imu_calibration::msg::to_block_style_yaml(msg, out, indentation);
}

[[deprecated("use cube_imu_calibration::msg::to_yaml() instead")]]
inline std::string to_yaml(const cube_imu_calibration::msg::CubePose & msg)
{
  return cube_imu_calibration::msg::to_yaml(msg);
}

template<>
inline const char * data_type<cube_imu_calibration::msg::CubePose>()
{
  return "cube_imu_calibration::msg::CubePose";
}

template<>
inline const char * name<cube_imu_calibration::msg::CubePose>()
{
  return "cube_imu_calibration/msg/CubePose";
}

template<>
struct has_fixed_size<cube_imu_calibration::msg::CubePose>
  : std::integral_constant<bool, has_fixed_size<geometry_msgs::msg::Quaternion>::value && has_fixed_size<geometry_msgs::msg::Vector3>::value && has_fixed_size<std_msgs::msg::Header>::value> {};

template<>
struct has_bounded_size<cube_imu_calibration::msg::CubePose>
  : std::integral_constant<bool, has_bounded_size<geometry_msgs::msg::Quaternion>::value && has_bounded_size<geometry_msgs::msg::Vector3>::value && has_bounded_size<std_msgs::msg::Header>::value> {};

template<>
struct is_message<cube_imu_calibration::msg::CubePose>
  : std::true_type {};

}  // namespace rosidl_generator_traits

#endif  // CUBE_IMU_CALIBRATION__MSG__DETAIL__CUBE_POSE__TRAITS_HPP_
