// generated from rosidl_generator_c/resource/idl__functions.c.em
// with input from cube_imu_calibration:msg/CubePoseStatus.idl
// generated code does not contain a copyright notice
#include "cube_imu_calibration/msg/detail/cube_pose_status__functions.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "rcutils/allocator.h"


// Include directives for member types
// Member `header`
#include "std_msgs/msg/detail/header__functions.h"
// Member `tag_ids`
#include "rosidl_runtime_c/primitives_sequence_functions.h"
// Member `message`
#include "rosidl_runtime_c/string_functions.h"

bool
cube_imu_calibration__msg__CubePoseStatus__init(cube_imu_calibration__msg__CubePoseStatus * msg)
{
  if (!msg) {
    return false;
  }
  // header
  if (!std_msgs__msg__Header__init(&msg->header)) {
    cube_imu_calibration__msg__CubePoseStatus__fini(msg);
    return false;
  }
  // pose_valid
  // tag_count
  // tag_ids
  if (!rosidl_runtime_c__int32__Sequence__init(&msg->tag_ids, 0)) {
    cube_imu_calibration__msg__CubePoseStatus__fini(msg);
    return false;
  }
  // reproj_error
  // message
  if (!rosidl_runtime_c__String__init(&msg->message)) {
    cube_imu_calibration__msg__CubePoseStatus__fini(msg);
    return false;
  }
  return true;
}

void
cube_imu_calibration__msg__CubePoseStatus__fini(cube_imu_calibration__msg__CubePoseStatus * msg)
{
  if (!msg) {
    return;
  }
  // header
  std_msgs__msg__Header__fini(&msg->header);
  // pose_valid
  // tag_count
  // tag_ids
  rosidl_runtime_c__int32__Sequence__fini(&msg->tag_ids);
  // reproj_error
  // message
  rosidl_runtime_c__String__fini(&msg->message);
}

bool
cube_imu_calibration__msg__CubePoseStatus__are_equal(const cube_imu_calibration__msg__CubePoseStatus * lhs, const cube_imu_calibration__msg__CubePoseStatus * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  // header
  if (!std_msgs__msg__Header__are_equal(
      &(lhs->header), &(rhs->header)))
  {
    return false;
  }
  // pose_valid
  if (lhs->pose_valid != rhs->pose_valid) {
    return false;
  }
  // tag_count
  if (lhs->tag_count != rhs->tag_count) {
    return false;
  }
  // tag_ids
  if (!rosidl_runtime_c__int32__Sequence__are_equal(
      &(lhs->tag_ids), &(rhs->tag_ids)))
  {
    return false;
  }
  // reproj_error
  if (lhs->reproj_error != rhs->reproj_error) {
    return false;
  }
  // message
  if (!rosidl_runtime_c__String__are_equal(
      &(lhs->message), &(rhs->message)))
  {
    return false;
  }
  return true;
}

bool
cube_imu_calibration__msg__CubePoseStatus__copy(
  const cube_imu_calibration__msg__CubePoseStatus * input,
  cube_imu_calibration__msg__CubePoseStatus * output)
{
  if (!input || !output) {
    return false;
  }
  // header
  if (!std_msgs__msg__Header__copy(
      &(input->header), &(output->header)))
  {
    return false;
  }
  // pose_valid
  output->pose_valid = input->pose_valid;
  // tag_count
  output->tag_count = input->tag_count;
  // tag_ids
  if (!rosidl_runtime_c__int32__Sequence__copy(
      &(input->tag_ids), &(output->tag_ids)))
  {
    return false;
  }
  // reproj_error
  output->reproj_error = input->reproj_error;
  // message
  if (!rosidl_runtime_c__String__copy(
      &(input->message), &(output->message)))
  {
    return false;
  }
  return true;
}

cube_imu_calibration__msg__CubePoseStatus *
cube_imu_calibration__msg__CubePoseStatus__create(void)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  cube_imu_calibration__msg__CubePoseStatus * msg = (cube_imu_calibration__msg__CubePoseStatus *)allocator.allocate(sizeof(cube_imu_calibration__msg__CubePoseStatus), allocator.state);
  if (!msg) {
    return NULL;
  }
  memset(msg, 0, sizeof(cube_imu_calibration__msg__CubePoseStatus));
  bool success = cube_imu_calibration__msg__CubePoseStatus__init(msg);
  if (!success) {
    allocator.deallocate(msg, allocator.state);
    return NULL;
  }
  return msg;
}

void
cube_imu_calibration__msg__CubePoseStatus__destroy(cube_imu_calibration__msg__CubePoseStatus * msg)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (msg) {
    cube_imu_calibration__msg__CubePoseStatus__fini(msg);
  }
  allocator.deallocate(msg, allocator.state);
}


bool
cube_imu_calibration__msg__CubePoseStatus__Sequence__init(cube_imu_calibration__msg__CubePoseStatus__Sequence * array, size_t size)
{
  if (!array) {
    return false;
  }
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  cube_imu_calibration__msg__CubePoseStatus * data = NULL;

  if (size) {
    data = (cube_imu_calibration__msg__CubePoseStatus *)allocator.zero_allocate(size, sizeof(cube_imu_calibration__msg__CubePoseStatus), allocator.state);
    if (!data) {
      return false;
    }
    // initialize all array elements
    size_t i;
    for (i = 0; i < size; ++i) {
      bool success = cube_imu_calibration__msg__CubePoseStatus__init(&data[i]);
      if (!success) {
        break;
      }
    }
    if (i < size) {
      // if initialization failed finalize the already initialized array elements
      for (; i > 0; --i) {
        cube_imu_calibration__msg__CubePoseStatus__fini(&data[i - 1]);
      }
      allocator.deallocate(data, allocator.state);
      return false;
    }
  }
  array->data = data;
  array->size = size;
  array->capacity = size;
  return true;
}

void
cube_imu_calibration__msg__CubePoseStatus__Sequence__fini(cube_imu_calibration__msg__CubePoseStatus__Sequence * array)
{
  if (!array) {
    return;
  }
  rcutils_allocator_t allocator = rcutils_get_default_allocator();

  if (array->data) {
    // ensure that data and capacity values are consistent
    assert(array->capacity > 0);
    // finalize all array elements
    for (size_t i = 0; i < array->capacity; ++i) {
      cube_imu_calibration__msg__CubePoseStatus__fini(&array->data[i]);
    }
    allocator.deallocate(array->data, allocator.state);
    array->data = NULL;
    array->size = 0;
    array->capacity = 0;
  } else {
    // ensure that data, size, and capacity values are consistent
    assert(0 == array->size);
    assert(0 == array->capacity);
  }
}

cube_imu_calibration__msg__CubePoseStatus__Sequence *
cube_imu_calibration__msg__CubePoseStatus__Sequence__create(size_t size)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  cube_imu_calibration__msg__CubePoseStatus__Sequence * array = (cube_imu_calibration__msg__CubePoseStatus__Sequence *)allocator.allocate(sizeof(cube_imu_calibration__msg__CubePoseStatus__Sequence), allocator.state);
  if (!array) {
    return NULL;
  }
  bool success = cube_imu_calibration__msg__CubePoseStatus__Sequence__init(array, size);
  if (!success) {
    allocator.deallocate(array, allocator.state);
    return NULL;
  }
  return array;
}

void
cube_imu_calibration__msg__CubePoseStatus__Sequence__destroy(cube_imu_calibration__msg__CubePoseStatus__Sequence * array)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (array) {
    cube_imu_calibration__msg__CubePoseStatus__Sequence__fini(array);
  }
  allocator.deallocate(array, allocator.state);
}

bool
cube_imu_calibration__msg__CubePoseStatus__Sequence__are_equal(const cube_imu_calibration__msg__CubePoseStatus__Sequence * lhs, const cube_imu_calibration__msg__CubePoseStatus__Sequence * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  if (lhs->size != rhs->size) {
    return false;
  }
  for (size_t i = 0; i < lhs->size; ++i) {
    if (!cube_imu_calibration__msg__CubePoseStatus__are_equal(&(lhs->data[i]), &(rhs->data[i]))) {
      return false;
    }
  }
  return true;
}

bool
cube_imu_calibration__msg__CubePoseStatus__Sequence__copy(
  const cube_imu_calibration__msg__CubePoseStatus__Sequence * input,
  cube_imu_calibration__msg__CubePoseStatus__Sequence * output)
{
  if (!input || !output) {
    return false;
  }
  if (output->capacity < input->size) {
    const size_t allocation_size =
      input->size * sizeof(cube_imu_calibration__msg__CubePoseStatus);
    rcutils_allocator_t allocator = rcutils_get_default_allocator();
    cube_imu_calibration__msg__CubePoseStatus * data =
      (cube_imu_calibration__msg__CubePoseStatus *)allocator.reallocate(
      output->data, allocation_size, allocator.state);
    if (!data) {
      return false;
    }
    // If reallocation succeeded, memory may or may not have been moved
    // to fulfill the allocation request, invalidating output->data.
    output->data = data;
    for (size_t i = output->capacity; i < input->size; ++i) {
      if (!cube_imu_calibration__msg__CubePoseStatus__init(&output->data[i])) {
        // If initialization of any new item fails, roll back
        // all previously initialized items. Existing items
        // in output are to be left unmodified.
        for (; i-- > output->capacity; ) {
          cube_imu_calibration__msg__CubePoseStatus__fini(&output->data[i]);
        }
        return false;
      }
    }
    output->capacity = input->size;
  }
  output->size = input->size;
  for (size_t i = 0; i < input->size; ++i) {
    if (!cube_imu_calibration__msg__CubePoseStatus__copy(
        &(input->data[i]), &(output->data[i])))
    {
      return false;
    }
  }
  return true;
}
