// generated from rosidl_generator_c/resource/idl__functions.h.em
// with input from cube_imu_calibration:msg/CubePoseStatus.idl
// generated code does not contain a copyright notice

// IWYU pragma: private, include "cube_imu_calibration/msg/cube_pose_status.h"


#ifndef CUBE_IMU_CALIBRATION__MSG__DETAIL__CUBE_POSE_STATUS__FUNCTIONS_H_
#define CUBE_IMU_CALIBRATION__MSG__DETAIL__CUBE_POSE_STATUS__FUNCTIONS_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stdlib.h>

#include "rosidl_runtime_c/action_type_support_struct.h"
#include "rosidl_runtime_c/message_type_support_struct.h"
#include "rosidl_runtime_c/service_type_support_struct.h"
#include "rosidl_runtime_c/type_description/type_description__struct.h"
#include "rosidl_runtime_c/type_description/type_source__struct.h"
#include "rosidl_runtime_c/type_hash.h"
#include "rosidl_runtime_c/visibility_control.h"
#include "cube_imu_calibration/msg/rosidl_generator_c__visibility_control.h"

#include "cube_imu_calibration/msg/detail/cube_pose_status__struct.h"

/// Initialize msg/CubePoseStatus message.
/**
 * If the init function is called twice for the same message without
 * calling fini inbetween previously allocated memory will be leaked.
 * \param[in,out] msg The previously allocated message pointer.
 * Fields without a default value will not be initialized by this function.
 * You might want to call memset(msg, 0, sizeof(
 * cube_imu_calibration__msg__CubePoseStatus
 * )) before or use
 * cube_imu_calibration__msg__CubePoseStatus__create()
 * to allocate and initialize the message.
 * \return true if initialization was successful, otherwise false
 */
ROSIDL_GENERATOR_C_PUBLIC_cube_imu_calibration
bool
cube_imu_calibration__msg__CubePoseStatus__init(cube_imu_calibration__msg__CubePoseStatus * msg);

/// Finalize msg/CubePoseStatus message.
/**
 * \param[in,out] msg The allocated message pointer.
 */
ROSIDL_GENERATOR_C_PUBLIC_cube_imu_calibration
void
cube_imu_calibration__msg__CubePoseStatus__fini(cube_imu_calibration__msg__CubePoseStatus * msg);

/// Create msg/CubePoseStatus message.
/**
 * It allocates the memory for the message, sets the memory to zero, and
 * calls
 * cube_imu_calibration__msg__CubePoseStatus__init().
 * \return The pointer to the initialized message if successful,
 * otherwise NULL
 */
ROSIDL_GENERATOR_C_PUBLIC_cube_imu_calibration
cube_imu_calibration__msg__CubePoseStatus *
cube_imu_calibration__msg__CubePoseStatus__create(void);

/// Destroy msg/CubePoseStatus message.
/**
 * It calls
 * cube_imu_calibration__msg__CubePoseStatus__fini()
 * and frees the memory of the message.
 * \param[in,out] msg The allocated message pointer.
 */
ROSIDL_GENERATOR_C_PUBLIC_cube_imu_calibration
void
cube_imu_calibration__msg__CubePoseStatus__destroy(cube_imu_calibration__msg__CubePoseStatus * msg);

/// Check for msg/CubePoseStatus message equality.
/**
 * \param[in] lhs The message on the left hand size of the equality operator.
 * \param[in] rhs The message on the right hand size of the equality operator.
 * \return true if messages are equal, otherwise false.
 */
ROSIDL_GENERATOR_C_PUBLIC_cube_imu_calibration
bool
cube_imu_calibration__msg__CubePoseStatus__are_equal(const cube_imu_calibration__msg__CubePoseStatus * lhs, const cube_imu_calibration__msg__CubePoseStatus * rhs);

/// Copy a msg/CubePoseStatus message.
/**
 * This functions performs a deep copy, as opposed to the shallow copy that
 * plain assignment yields.
 *
 * \param[in] input The source message pointer.
 * \param[out] output The target message pointer, which must
 *   have been initialized before calling this function.
 * \return true if successful, or false if either pointer is null
 *   or memory allocation fails.
 */
ROSIDL_GENERATOR_C_PUBLIC_cube_imu_calibration
bool
cube_imu_calibration__msg__CubePoseStatus__copy(
  const cube_imu_calibration__msg__CubePoseStatus * input,
  cube_imu_calibration__msg__CubePoseStatus * output);

/// Retrieve pointer to the hash of the description of this type.
ROSIDL_GENERATOR_C_PUBLIC_cube_imu_calibration
const rosidl_type_hash_t *
cube_imu_calibration__msg__CubePoseStatus__get_type_hash(
  const rosidl_message_type_support_t * type_support);

/// Retrieve pointer to the description of this type.
ROSIDL_GENERATOR_C_PUBLIC_cube_imu_calibration
const rosidl_runtime_c__type_description__TypeDescription *
cube_imu_calibration__msg__CubePoseStatus__get_type_description(
  const rosidl_message_type_support_t * type_support);

/// Retrieve pointer to the single raw source text that defined this type.
ROSIDL_GENERATOR_C_PUBLIC_cube_imu_calibration
const rosidl_runtime_c__type_description__TypeSource *
cube_imu_calibration__msg__CubePoseStatus__get_individual_type_description_source(
  const rosidl_message_type_support_t * type_support);

/// Retrieve pointer to the recursive raw sources that defined the description of this type.
ROSIDL_GENERATOR_C_PUBLIC_cube_imu_calibration
const rosidl_runtime_c__type_description__TypeSource__Sequence *
cube_imu_calibration__msg__CubePoseStatus__get_type_description_sources(
  const rosidl_message_type_support_t * type_support);

/// Initialize array of msg/CubePoseStatus messages.
/**
 * It allocates the memory for the number of elements and calls
 * cube_imu_calibration__msg__CubePoseStatus__init()
 * for each element of the array.
 * \param[in,out] array The allocated array pointer.
 * \param[in] size The size / capacity of the array.
 * \return true if initialization was successful, otherwise false
 * If the array pointer is valid and the size is zero it is guaranteed
 # to return true.
 */
ROSIDL_GENERATOR_C_PUBLIC_cube_imu_calibration
bool
cube_imu_calibration__msg__CubePoseStatus__Sequence__init(cube_imu_calibration__msg__CubePoseStatus__Sequence * array, size_t size);

/// Finalize array of msg/CubePoseStatus messages.
/**
 * It calls
 * cube_imu_calibration__msg__CubePoseStatus__fini()
 * for each element of the array and frees the memory for the number of
 * elements.
 * \param[in,out] array The initialized array pointer.
 */
ROSIDL_GENERATOR_C_PUBLIC_cube_imu_calibration
void
cube_imu_calibration__msg__CubePoseStatus__Sequence__fini(cube_imu_calibration__msg__CubePoseStatus__Sequence * array);

/// Create array of msg/CubePoseStatus messages.
/**
 * It allocates the memory for the array and calls
 * cube_imu_calibration__msg__CubePoseStatus__Sequence__init().
 * \param[in] size The size / capacity of the array.
 * \return The pointer to the initialized array if successful, otherwise NULL
 */
ROSIDL_GENERATOR_C_PUBLIC_cube_imu_calibration
cube_imu_calibration__msg__CubePoseStatus__Sequence *
cube_imu_calibration__msg__CubePoseStatus__Sequence__create(size_t size);

/// Destroy array of msg/CubePoseStatus messages.
/**
 * It calls
 * cube_imu_calibration__msg__CubePoseStatus__Sequence__fini()
 * on the array,
 * and frees the memory of the array.
 * \param[in,out] array The initialized array pointer.
 */
ROSIDL_GENERATOR_C_PUBLIC_cube_imu_calibration
void
cube_imu_calibration__msg__CubePoseStatus__Sequence__destroy(cube_imu_calibration__msg__CubePoseStatus__Sequence * array);

/// Check for msg/CubePoseStatus message array equality.
/**
 * \param[in] lhs The message array on the left hand size of the equality operator.
 * \param[in] rhs The message array on the right hand size of the equality operator.
 * \return true if message arrays are equal in size and content, otherwise false.
 */
ROSIDL_GENERATOR_C_PUBLIC_cube_imu_calibration
bool
cube_imu_calibration__msg__CubePoseStatus__Sequence__are_equal(const cube_imu_calibration__msg__CubePoseStatus__Sequence * lhs, const cube_imu_calibration__msg__CubePoseStatus__Sequence * rhs);

/// Copy an array of msg/CubePoseStatus messages.
/**
 * This functions performs a deep copy, as opposed to the shallow copy that
 * plain assignment yields.
 *
 * \param[in] input The source array pointer.
 * \param[out] output The target array pointer, which must
 *   have been initialized before calling this function.
 * \return true if successful, or false if either pointer
 *   is null or memory allocation fails.
 */
ROSIDL_GENERATOR_C_PUBLIC_cube_imu_calibration
bool
cube_imu_calibration__msg__CubePoseStatus__Sequence__copy(
  const cube_imu_calibration__msg__CubePoseStatus__Sequence * input,
  cube_imu_calibration__msg__CubePoseStatus__Sequence * output);

#ifdef __cplusplus
}
#endif

#endif  // CUBE_IMU_CALIBRATION__MSG__DETAIL__CUBE_POSE_STATUS__FUNCTIONS_H_
