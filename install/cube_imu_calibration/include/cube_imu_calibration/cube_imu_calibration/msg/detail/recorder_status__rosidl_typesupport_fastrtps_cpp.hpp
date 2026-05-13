// generated from rosidl_typesupport_fastrtps_cpp/resource/idl__rosidl_typesupport_fastrtps_cpp.hpp.em
// with input from cube_imu_calibration:msg/RecorderStatus.idl
// generated code does not contain a copyright notice

#ifndef CUBE_IMU_CALIBRATION__MSG__DETAIL__RECORDER_STATUS__ROSIDL_TYPESUPPORT_FASTRTPS_CPP_HPP_
#define CUBE_IMU_CALIBRATION__MSG__DETAIL__RECORDER_STATUS__ROSIDL_TYPESUPPORT_FASTRTPS_CPP_HPP_

#include <cstddef>
#include "rosidl_runtime_c/message_type_support_struct.h"
#include "rosidl_typesupport_interface/macros.h"
#include "cube_imu_calibration/msg/rosidl_typesupport_fastrtps_cpp__visibility_control.h"
#include "cube_imu_calibration/msg/detail/recorder_status__struct.hpp"

#ifndef _WIN32
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wunused-parameter"
# ifdef __clang__
#  pragma clang diagnostic ignored "-Wdeprecated-register"
#  pragma clang diagnostic ignored "-Wreturn-type-c-linkage"
# endif
#endif
#ifndef _WIN32
# pragma GCC diagnostic pop
#endif

#include "fastcdr/Cdr.h"

namespace cube_imu_calibration
{

namespace msg
{

namespace typesupport_fastrtps_cpp
{

bool
ROSIDL_TYPESUPPORT_FASTRTPS_CPP_PUBLIC_cube_imu_calibration
cdr_serialize(
  const cube_imu_calibration::msg::RecorderStatus & ros_message,
  eprosima::fastcdr::Cdr & cdr);

bool
ROSIDL_TYPESUPPORT_FASTRTPS_CPP_PUBLIC_cube_imu_calibration
cdr_deserialize(
  eprosima::fastcdr::Cdr & cdr,
  cube_imu_calibration::msg::RecorderStatus & ros_message);

size_t
ROSIDL_TYPESUPPORT_FASTRTPS_CPP_PUBLIC_cube_imu_calibration
get_serialized_size(
  const cube_imu_calibration::msg::RecorderStatus & ros_message,
  size_t current_alignment);

size_t
ROSIDL_TYPESUPPORT_FASTRTPS_CPP_PUBLIC_cube_imu_calibration
max_serialized_size_RecorderStatus(
  bool & full_bounded,
  bool & is_plain,
  size_t current_alignment);

bool
ROSIDL_TYPESUPPORT_FASTRTPS_CPP_PUBLIC_cube_imu_calibration
cdr_serialize_key(
  const cube_imu_calibration::msg::RecorderStatus & ros_message,
  eprosima::fastcdr::Cdr &);

size_t
ROSIDL_TYPESUPPORT_FASTRTPS_CPP_PUBLIC_cube_imu_calibration
get_serialized_size_key(
  const cube_imu_calibration::msg::RecorderStatus & ros_message,
  size_t current_alignment);

size_t
ROSIDL_TYPESUPPORT_FASTRTPS_CPP_PUBLIC_cube_imu_calibration
max_serialized_size_key_RecorderStatus(
  bool & full_bounded,
  bool & is_plain,
  size_t current_alignment);

}  // namespace typesupport_fastrtps_cpp

}  // namespace msg

}  // namespace cube_imu_calibration

#ifdef __cplusplus
extern "C"
{
#endif

ROSIDL_TYPESUPPORT_FASTRTPS_CPP_PUBLIC_cube_imu_calibration
const rosidl_message_type_support_t *
  ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_fastrtps_cpp, cube_imu_calibration, msg, RecorderStatus)();

#ifdef __cplusplus
}
#endif

#endif  // CUBE_IMU_CALIBRATION__MSG__DETAIL__RECORDER_STATUS__ROSIDL_TYPESUPPORT_FASTRTPS_CPP_HPP_
