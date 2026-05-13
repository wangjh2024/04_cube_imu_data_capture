// generated from rosidl_generator_c/resource/idl__struct.h.em
// with input from cube_imu_calibration:msg/RecorderStatus.idl
// generated code does not contain a copyright notice

// IWYU pragma: private, include "cube_imu_calibration/msg/recorder_status.h"


#ifndef CUBE_IMU_CALIBRATION__MSG__DETAIL__RECORDER_STATUS__STRUCT_H_
#define CUBE_IMU_CALIBRATION__MSG__DETAIL__RECORDER_STATUS__STRUCT_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Constants defined in the message

// Include directives for member types
// Member 'header'
#include "std_msgs/msg/detail/header__struct.h"
// Member 'bag_path'
#include "rosidl_runtime_c/string.h"

/// Struct defined in msg/RecorderStatus in the package cube_imu_calibration.
typedef struct cube_imu_calibration__msg__RecorderStatus
{
  std_msgs__msg__Header header;
  bool recording;
  rosidl_runtime_c__String bag_path;
  uint64_t image_count;
  uint64_t imu_count;
  uint64_t camera_info_count;
  uint64_t cube_pose_count;
  uint64_t cube_pose_status_count;
  uint32_t image_publishers;
  uint32_t imu_publishers;
  uint32_t camera_info_publishers;
  uint32_t cube_pose_publishers;
  uint32_t cube_pose_status_publishers;
  /// Recording progress. target_duration_sec <= 0 means manual/unknown-length recording.
  double elapsed_sec;
  double target_duration_sec;
  double progress_ratio;
} cube_imu_calibration__msg__RecorderStatus;

// Struct for a sequence of cube_imu_calibration__msg__RecorderStatus.
typedef struct cube_imu_calibration__msg__RecorderStatus__Sequence
{
  cube_imu_calibration__msg__RecorderStatus * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} cube_imu_calibration__msg__RecorderStatus__Sequence;

#ifdef __cplusplus
}
#endif

#endif  // CUBE_IMU_CALIBRATION__MSG__DETAIL__RECORDER_STATUS__STRUCT_H_
