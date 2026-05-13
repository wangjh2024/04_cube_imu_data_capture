// generated from rosidl_generator_c/resource/idl__struct.h.em
// with input from cube_imu_calibration:msg/CubePose.idl
// generated code does not contain a copyright notice

// IWYU pragma: private, include "cube_imu_calibration/msg/cube_pose.h"


#ifndef CUBE_IMU_CALIBRATION__MSG__DETAIL__CUBE_POSE__STRUCT_H_
#define CUBE_IMU_CALIBRATION__MSG__DETAIL__CUBE_POSE__STRUCT_H_

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
// Member 'translation'
#include "geometry_msgs/msg/detail/vector3__struct.h"
// Member 'orientation'
#include "geometry_msgs/msg/detail/quaternion__struct.h"

/// Struct defined in msg/CubePose in the package cube_imu_calibration.
typedef struct cube_imu_calibration__msg__CubePose
{
  std_msgs__msg__Header header;
  /// Pose of cube_link expressed in the configured camera frame.
  geometry_msgs__msg__Vector3 translation;
  geometry_msgs__msg__Quaternion orientation;
  /// Row-major 4x4 homogeneous transform T_Cam_Cube.
  double transform[16];
} cube_imu_calibration__msg__CubePose;

// Struct for a sequence of cube_imu_calibration__msg__CubePose.
typedef struct cube_imu_calibration__msg__CubePose__Sequence
{
  cube_imu_calibration__msg__CubePose * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} cube_imu_calibration__msg__CubePose__Sequence;

#ifdef __cplusplus
}
#endif

#endif  // CUBE_IMU_CALIBRATION__MSG__DETAIL__CUBE_POSE__STRUCT_H_
