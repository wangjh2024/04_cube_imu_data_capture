// generated from rosidl_typesupport_fastrtps_c/resource/idl__rosidl_typesupport_fastrtps_c.h.em
// with input from cube_imu_calibration:msg/CubePose.idl
// generated code does not contain a copyright notice
#ifndef CUBE_IMU_CALIBRATION__MSG__DETAIL__CUBE_POSE__ROSIDL_TYPESUPPORT_FASTRTPS_C_H_
#define CUBE_IMU_CALIBRATION__MSG__DETAIL__CUBE_POSE__ROSIDL_TYPESUPPORT_FASTRTPS_C_H_


#include <stddef.h>
#include "rosidl_runtime_c/message_type_support_struct.h"
#include "rosidl_typesupport_interface/macros.h"
#include "cube_imu_calibration/msg/rosidl_typesupport_fastrtps_c__visibility_control.h"
#include "cube_imu_calibration/msg/detail/cube_pose__struct.h"
#include "fastcdr/Cdr.h"

#ifdef __cplusplus
extern "C"
{
#endif

ROSIDL_TYPESUPPORT_FASTRTPS_C_PUBLIC_cube_imu_calibration
bool cdr_serialize_cube_imu_calibration__msg__CubePose(
  const cube_imu_calibration__msg__CubePose * ros_message,
  eprosima::fastcdr::Cdr & cdr);

ROSIDL_TYPESUPPORT_FASTRTPS_C_PUBLIC_cube_imu_calibration
bool cdr_deserialize_cube_imu_calibration__msg__CubePose(
  eprosima::fastcdr::Cdr &,
  cube_imu_calibration__msg__CubePose * ros_message);

ROSIDL_TYPESUPPORT_FASTRTPS_C_PUBLIC_cube_imu_calibration
size_t get_serialized_size_cube_imu_calibration__msg__CubePose(
  const void * untyped_ros_message,
  size_t current_alignment);

ROSIDL_TYPESUPPORT_FASTRTPS_C_PUBLIC_cube_imu_calibration
size_t max_serialized_size_cube_imu_calibration__msg__CubePose(
  bool & full_bounded,
  bool & is_plain,
  size_t current_alignment);

ROSIDL_TYPESUPPORT_FASTRTPS_C_PUBLIC_cube_imu_calibration
bool cdr_serialize_key_cube_imu_calibration__msg__CubePose(
  const cube_imu_calibration__msg__CubePose * ros_message,
  eprosima::fastcdr::Cdr & cdr);

ROSIDL_TYPESUPPORT_FASTRTPS_C_PUBLIC_cube_imu_calibration
size_t get_serialized_size_key_cube_imu_calibration__msg__CubePose(
  const void * untyped_ros_message,
  size_t current_alignment);

ROSIDL_TYPESUPPORT_FASTRTPS_C_PUBLIC_cube_imu_calibration
size_t max_serialized_size_key_cube_imu_calibration__msg__CubePose(
  bool & full_bounded,
  bool & is_plain,
  size_t current_alignment);

ROSIDL_TYPESUPPORT_FASTRTPS_C_PUBLIC_cube_imu_calibration
const rosidl_message_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_fastrtps_c, cube_imu_calibration, msg, CubePose)();

#ifdef __cplusplus
}
#endif

#endif  // CUBE_IMU_CALIBRATION__MSG__DETAIL__CUBE_POSE__ROSIDL_TYPESUPPORT_FASTRTPS_C_H_
