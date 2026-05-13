// generated from rosidl_generator_cpp/resource/idl__builder.hpp.em
// with input from cube_imu_calibration:msg/RecorderStatus.idl
// generated code does not contain a copyright notice

// IWYU pragma: private, include "cube_imu_calibration/msg/recorder_status.hpp"


#ifndef CUBE_IMU_CALIBRATION__MSG__DETAIL__RECORDER_STATUS__BUILDER_HPP_
#define CUBE_IMU_CALIBRATION__MSG__DETAIL__RECORDER_STATUS__BUILDER_HPP_

#include <algorithm>
#include <utility>

#include "cube_imu_calibration/msg/detail/recorder_status__struct.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


namespace cube_imu_calibration
{

namespace msg
{

namespace builder
{

class Init_RecorderStatus_progress_ratio
{
public:
  explicit Init_RecorderStatus_progress_ratio(::cube_imu_calibration::msg::RecorderStatus & msg)
  : msg_(msg)
  {}
  ::cube_imu_calibration::msg::RecorderStatus progress_ratio(::cube_imu_calibration::msg::RecorderStatus::_progress_ratio_type arg)
  {
    msg_.progress_ratio = std::move(arg);
    return std::move(msg_);
  }

private:
  ::cube_imu_calibration::msg::RecorderStatus msg_;
};

class Init_RecorderStatus_target_duration_sec
{
public:
  explicit Init_RecorderStatus_target_duration_sec(::cube_imu_calibration::msg::RecorderStatus & msg)
  : msg_(msg)
  {}
  Init_RecorderStatus_progress_ratio target_duration_sec(::cube_imu_calibration::msg::RecorderStatus::_target_duration_sec_type arg)
  {
    msg_.target_duration_sec = std::move(arg);
    return Init_RecorderStatus_progress_ratio(msg_);
  }

private:
  ::cube_imu_calibration::msg::RecorderStatus msg_;
};

class Init_RecorderStatus_elapsed_sec
{
public:
  explicit Init_RecorderStatus_elapsed_sec(::cube_imu_calibration::msg::RecorderStatus & msg)
  : msg_(msg)
  {}
  Init_RecorderStatus_target_duration_sec elapsed_sec(::cube_imu_calibration::msg::RecorderStatus::_elapsed_sec_type arg)
  {
    msg_.elapsed_sec = std::move(arg);
    return Init_RecorderStatus_target_duration_sec(msg_);
  }

private:
  ::cube_imu_calibration::msg::RecorderStatus msg_;
};

class Init_RecorderStatus_cube_pose_status_publishers
{
public:
  explicit Init_RecorderStatus_cube_pose_status_publishers(::cube_imu_calibration::msg::RecorderStatus & msg)
  : msg_(msg)
  {}
  Init_RecorderStatus_elapsed_sec cube_pose_status_publishers(::cube_imu_calibration::msg::RecorderStatus::_cube_pose_status_publishers_type arg)
  {
    msg_.cube_pose_status_publishers = std::move(arg);
    return Init_RecorderStatus_elapsed_sec(msg_);
  }

private:
  ::cube_imu_calibration::msg::RecorderStatus msg_;
};

class Init_RecorderStatus_cube_pose_publishers
{
public:
  explicit Init_RecorderStatus_cube_pose_publishers(::cube_imu_calibration::msg::RecorderStatus & msg)
  : msg_(msg)
  {}
  Init_RecorderStatus_cube_pose_status_publishers cube_pose_publishers(::cube_imu_calibration::msg::RecorderStatus::_cube_pose_publishers_type arg)
  {
    msg_.cube_pose_publishers = std::move(arg);
    return Init_RecorderStatus_cube_pose_status_publishers(msg_);
  }

private:
  ::cube_imu_calibration::msg::RecorderStatus msg_;
};

class Init_RecorderStatus_camera_info_publishers
{
public:
  explicit Init_RecorderStatus_camera_info_publishers(::cube_imu_calibration::msg::RecorderStatus & msg)
  : msg_(msg)
  {}
  Init_RecorderStatus_cube_pose_publishers camera_info_publishers(::cube_imu_calibration::msg::RecorderStatus::_camera_info_publishers_type arg)
  {
    msg_.camera_info_publishers = std::move(arg);
    return Init_RecorderStatus_cube_pose_publishers(msg_);
  }

private:
  ::cube_imu_calibration::msg::RecorderStatus msg_;
};

class Init_RecorderStatus_imu_publishers
{
public:
  explicit Init_RecorderStatus_imu_publishers(::cube_imu_calibration::msg::RecorderStatus & msg)
  : msg_(msg)
  {}
  Init_RecorderStatus_camera_info_publishers imu_publishers(::cube_imu_calibration::msg::RecorderStatus::_imu_publishers_type arg)
  {
    msg_.imu_publishers = std::move(arg);
    return Init_RecorderStatus_camera_info_publishers(msg_);
  }

private:
  ::cube_imu_calibration::msg::RecorderStatus msg_;
};

class Init_RecorderStatus_image_publishers
{
public:
  explicit Init_RecorderStatus_image_publishers(::cube_imu_calibration::msg::RecorderStatus & msg)
  : msg_(msg)
  {}
  Init_RecorderStatus_imu_publishers image_publishers(::cube_imu_calibration::msg::RecorderStatus::_image_publishers_type arg)
  {
    msg_.image_publishers = std::move(arg);
    return Init_RecorderStatus_imu_publishers(msg_);
  }

private:
  ::cube_imu_calibration::msg::RecorderStatus msg_;
};

class Init_RecorderStatus_cube_pose_status_count
{
public:
  explicit Init_RecorderStatus_cube_pose_status_count(::cube_imu_calibration::msg::RecorderStatus & msg)
  : msg_(msg)
  {}
  Init_RecorderStatus_image_publishers cube_pose_status_count(::cube_imu_calibration::msg::RecorderStatus::_cube_pose_status_count_type arg)
  {
    msg_.cube_pose_status_count = std::move(arg);
    return Init_RecorderStatus_image_publishers(msg_);
  }

private:
  ::cube_imu_calibration::msg::RecorderStatus msg_;
};

class Init_RecorderStatus_cube_pose_count
{
public:
  explicit Init_RecorderStatus_cube_pose_count(::cube_imu_calibration::msg::RecorderStatus & msg)
  : msg_(msg)
  {}
  Init_RecorderStatus_cube_pose_status_count cube_pose_count(::cube_imu_calibration::msg::RecorderStatus::_cube_pose_count_type arg)
  {
    msg_.cube_pose_count = std::move(arg);
    return Init_RecorderStatus_cube_pose_status_count(msg_);
  }

private:
  ::cube_imu_calibration::msg::RecorderStatus msg_;
};

class Init_RecorderStatus_camera_info_count
{
public:
  explicit Init_RecorderStatus_camera_info_count(::cube_imu_calibration::msg::RecorderStatus & msg)
  : msg_(msg)
  {}
  Init_RecorderStatus_cube_pose_count camera_info_count(::cube_imu_calibration::msg::RecorderStatus::_camera_info_count_type arg)
  {
    msg_.camera_info_count = std::move(arg);
    return Init_RecorderStatus_cube_pose_count(msg_);
  }

private:
  ::cube_imu_calibration::msg::RecorderStatus msg_;
};

class Init_RecorderStatus_imu_count
{
public:
  explicit Init_RecorderStatus_imu_count(::cube_imu_calibration::msg::RecorderStatus & msg)
  : msg_(msg)
  {}
  Init_RecorderStatus_camera_info_count imu_count(::cube_imu_calibration::msg::RecorderStatus::_imu_count_type arg)
  {
    msg_.imu_count = std::move(arg);
    return Init_RecorderStatus_camera_info_count(msg_);
  }

private:
  ::cube_imu_calibration::msg::RecorderStatus msg_;
};

class Init_RecorderStatus_image_count
{
public:
  explicit Init_RecorderStatus_image_count(::cube_imu_calibration::msg::RecorderStatus & msg)
  : msg_(msg)
  {}
  Init_RecorderStatus_imu_count image_count(::cube_imu_calibration::msg::RecorderStatus::_image_count_type arg)
  {
    msg_.image_count = std::move(arg);
    return Init_RecorderStatus_imu_count(msg_);
  }

private:
  ::cube_imu_calibration::msg::RecorderStatus msg_;
};

class Init_RecorderStatus_bag_path
{
public:
  explicit Init_RecorderStatus_bag_path(::cube_imu_calibration::msg::RecorderStatus & msg)
  : msg_(msg)
  {}
  Init_RecorderStatus_image_count bag_path(::cube_imu_calibration::msg::RecorderStatus::_bag_path_type arg)
  {
    msg_.bag_path = std::move(arg);
    return Init_RecorderStatus_image_count(msg_);
  }

private:
  ::cube_imu_calibration::msg::RecorderStatus msg_;
};

class Init_RecorderStatus_recording
{
public:
  explicit Init_RecorderStatus_recording(::cube_imu_calibration::msg::RecorderStatus & msg)
  : msg_(msg)
  {}
  Init_RecorderStatus_bag_path recording(::cube_imu_calibration::msg::RecorderStatus::_recording_type arg)
  {
    msg_.recording = std::move(arg);
    return Init_RecorderStatus_bag_path(msg_);
  }

private:
  ::cube_imu_calibration::msg::RecorderStatus msg_;
};

class Init_RecorderStatus_header
{
public:
  Init_RecorderStatus_header()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  Init_RecorderStatus_recording header(::cube_imu_calibration::msg::RecorderStatus::_header_type arg)
  {
    msg_.header = std::move(arg);
    return Init_RecorderStatus_recording(msg_);
  }

private:
  ::cube_imu_calibration::msg::RecorderStatus msg_;
};

}  // namespace builder

}  // namespace msg

template<typename MessageType>
auto build();

template<>
inline
auto build<::cube_imu_calibration::msg::RecorderStatus>()
{
  return cube_imu_calibration::msg::builder::Init_RecorderStatus_header();
}

}  // namespace cube_imu_calibration

#endif  // CUBE_IMU_CALIBRATION__MSG__DETAIL__RECORDER_STATUS__BUILDER_HPP_
