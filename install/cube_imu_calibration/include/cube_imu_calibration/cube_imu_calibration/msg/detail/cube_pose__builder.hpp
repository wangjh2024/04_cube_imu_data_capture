// generated from rosidl_generator_cpp/resource/idl__builder.hpp.em
// with input from cube_imu_calibration:msg/CubePose.idl
// generated code does not contain a copyright notice

// IWYU pragma: private, include "cube_imu_calibration/msg/cube_pose.hpp"


#ifndef CUBE_IMU_CALIBRATION__MSG__DETAIL__CUBE_POSE__BUILDER_HPP_
#define CUBE_IMU_CALIBRATION__MSG__DETAIL__CUBE_POSE__BUILDER_HPP_

#include <algorithm>
#include <utility>

#include "cube_imu_calibration/msg/detail/cube_pose__struct.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


namespace cube_imu_calibration
{

namespace msg
{

namespace builder
{

class Init_CubePose_transform
{
public:
  explicit Init_CubePose_transform(::cube_imu_calibration::msg::CubePose & msg)
  : msg_(msg)
  {}
  ::cube_imu_calibration::msg::CubePose transform(::cube_imu_calibration::msg::CubePose::_transform_type arg)
  {
    msg_.transform = std::move(arg);
    return std::move(msg_);
  }

private:
  ::cube_imu_calibration::msg::CubePose msg_;
};

class Init_CubePose_orientation
{
public:
  explicit Init_CubePose_orientation(::cube_imu_calibration::msg::CubePose & msg)
  : msg_(msg)
  {}
  Init_CubePose_transform orientation(::cube_imu_calibration::msg::CubePose::_orientation_type arg)
  {
    msg_.orientation = std::move(arg);
    return Init_CubePose_transform(msg_);
  }

private:
  ::cube_imu_calibration::msg::CubePose msg_;
};

class Init_CubePose_translation
{
public:
  explicit Init_CubePose_translation(::cube_imu_calibration::msg::CubePose & msg)
  : msg_(msg)
  {}
  Init_CubePose_orientation translation(::cube_imu_calibration::msg::CubePose::_translation_type arg)
  {
    msg_.translation = std::move(arg);
    return Init_CubePose_orientation(msg_);
  }

private:
  ::cube_imu_calibration::msg::CubePose msg_;
};

class Init_CubePose_header
{
public:
  Init_CubePose_header()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  Init_CubePose_translation header(::cube_imu_calibration::msg::CubePose::_header_type arg)
  {
    msg_.header = std::move(arg);
    return Init_CubePose_translation(msg_);
  }

private:
  ::cube_imu_calibration::msg::CubePose msg_;
};

}  // namespace builder

}  // namespace msg

template<typename MessageType>
auto build();

template<>
inline
auto build<::cube_imu_calibration::msg::CubePose>()
{
  return cube_imu_calibration::msg::builder::Init_CubePose_header();
}

}  // namespace cube_imu_calibration

#endif  // CUBE_IMU_CALIBRATION__MSG__DETAIL__CUBE_POSE__BUILDER_HPP_
