// generated from rosidl_generator_cpp/resource/idl__builder.hpp.em
// with input from cube_imu_calibration:msg/CubePoseStatus.idl
// generated code does not contain a copyright notice

// IWYU pragma: private, include "cube_imu_calibration/msg/cube_pose_status.hpp"


#ifndef CUBE_IMU_CALIBRATION__MSG__DETAIL__CUBE_POSE_STATUS__BUILDER_HPP_
#define CUBE_IMU_CALIBRATION__MSG__DETAIL__CUBE_POSE_STATUS__BUILDER_HPP_

#include <algorithm>
#include <utility>

#include "cube_imu_calibration/msg/detail/cube_pose_status__struct.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


namespace cube_imu_calibration
{

namespace msg
{

namespace builder
{

class Init_CubePoseStatus_message
{
public:
  explicit Init_CubePoseStatus_message(::cube_imu_calibration::msg::CubePoseStatus & msg)
  : msg_(msg)
  {}
  ::cube_imu_calibration::msg::CubePoseStatus message(::cube_imu_calibration::msg::CubePoseStatus::_message_type arg)
  {
    msg_.message = std::move(arg);
    return std::move(msg_);
  }

private:
  ::cube_imu_calibration::msg::CubePoseStatus msg_;
};

class Init_CubePoseStatus_reproj_error
{
public:
  explicit Init_CubePoseStatus_reproj_error(::cube_imu_calibration::msg::CubePoseStatus & msg)
  : msg_(msg)
  {}
  Init_CubePoseStatus_message reproj_error(::cube_imu_calibration::msg::CubePoseStatus::_reproj_error_type arg)
  {
    msg_.reproj_error = std::move(arg);
    return Init_CubePoseStatus_message(msg_);
  }

private:
  ::cube_imu_calibration::msg::CubePoseStatus msg_;
};

class Init_CubePoseStatus_tag_ids
{
public:
  explicit Init_CubePoseStatus_tag_ids(::cube_imu_calibration::msg::CubePoseStatus & msg)
  : msg_(msg)
  {}
  Init_CubePoseStatus_reproj_error tag_ids(::cube_imu_calibration::msg::CubePoseStatus::_tag_ids_type arg)
  {
    msg_.tag_ids = std::move(arg);
    return Init_CubePoseStatus_reproj_error(msg_);
  }

private:
  ::cube_imu_calibration::msg::CubePoseStatus msg_;
};

class Init_CubePoseStatus_tag_count
{
public:
  explicit Init_CubePoseStatus_tag_count(::cube_imu_calibration::msg::CubePoseStatus & msg)
  : msg_(msg)
  {}
  Init_CubePoseStatus_tag_ids tag_count(::cube_imu_calibration::msg::CubePoseStatus::_tag_count_type arg)
  {
    msg_.tag_count = std::move(arg);
    return Init_CubePoseStatus_tag_ids(msg_);
  }

private:
  ::cube_imu_calibration::msg::CubePoseStatus msg_;
};

class Init_CubePoseStatus_pose_valid
{
public:
  explicit Init_CubePoseStatus_pose_valid(::cube_imu_calibration::msg::CubePoseStatus & msg)
  : msg_(msg)
  {}
  Init_CubePoseStatus_tag_count pose_valid(::cube_imu_calibration::msg::CubePoseStatus::_pose_valid_type arg)
  {
    msg_.pose_valid = std::move(arg);
    return Init_CubePoseStatus_tag_count(msg_);
  }

private:
  ::cube_imu_calibration::msg::CubePoseStatus msg_;
};

class Init_CubePoseStatus_header
{
public:
  Init_CubePoseStatus_header()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  Init_CubePoseStatus_pose_valid header(::cube_imu_calibration::msg::CubePoseStatus::_header_type arg)
  {
    msg_.header = std::move(arg);
    return Init_CubePoseStatus_pose_valid(msg_);
  }

private:
  ::cube_imu_calibration::msg::CubePoseStatus msg_;
};

}  // namespace builder

}  // namespace msg

template<typename MessageType>
auto build();

template<>
inline
auto build<::cube_imu_calibration::msg::CubePoseStatus>()
{
  return cube_imu_calibration::msg::builder::Init_CubePoseStatus_header();
}

}  // namespace cube_imu_calibration

#endif  // CUBE_IMU_CALIBRATION__MSG__DETAIL__CUBE_POSE_STATUS__BUILDER_HPP_
