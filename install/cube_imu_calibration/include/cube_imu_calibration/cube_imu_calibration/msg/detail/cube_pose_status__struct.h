// generated from rosidl_generator_c/resource/idl__struct.h.em
// with input from cube_imu_calibration:msg/CubePoseStatus.idl
// generated code does not contain a copyright notice

// IWYU pragma: private, include "cube_imu_calibration/msg/cube_pose_status.h"


#ifndef CUBE_IMU_CALIBRATION__MSG__DETAIL__CUBE_POSE_STATUS__STRUCT_H_
#define CUBE_IMU_CALIBRATION__MSG__DETAIL__CUBE_POSE_STATUS__STRUCT_H_

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
// Member 'tag_ids'
#include "rosidl_runtime_c/primitives_sequence.h"
// Member 'message'
#include "rosidl_runtime_c/string.h"

/// Struct defined in msg/CubePoseStatus in the package cube_imu_calibration.
typedef struct cube_imu_calibration__msg__CubePoseStatus
{
  std_msgs__msg__Header header;
  /// Quality report for the Cube pose estimated from AprilTag detections.
  bool pose_valid;
  uint32_t tag_count;
  rosidl_runtime_c__int32__Sequence tag_ids;
  double reproj_error;
  rosidl_runtime_c__String message;
} cube_imu_calibration__msg__CubePoseStatus;

// Struct for a sequence of cube_imu_calibration__msg__CubePoseStatus.
typedef struct cube_imu_calibration__msg__CubePoseStatus__Sequence
{
  cube_imu_calibration__msg__CubePoseStatus * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} cube_imu_calibration__msg__CubePoseStatus__Sequence;

#ifdef __cplusplus
}
#endif

#endif  // CUBE_IMU_CALIBRATION__MSG__DETAIL__CUBE_POSE_STATUS__STRUCT_H_
