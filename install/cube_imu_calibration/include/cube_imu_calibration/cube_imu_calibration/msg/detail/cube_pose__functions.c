// generated from rosidl_generator_c/resource/idl__functions.c.em
// with input from cube_imu_calibration:msg/CubePose.idl
// generated code does not contain a copyright notice
#include "cube_imu_calibration/msg/detail/cube_pose__functions.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "rcutils/allocator.h"


// Include directives for member types
// Member `header`
#include "std_msgs/msg/detail/header__functions.h"
// Member `translation`
#include "geometry_msgs/msg/detail/vector3__functions.h"
// Member `orientation`
#include "geometry_msgs/msg/detail/quaternion__functions.h"

bool
cube_imu_calibration__msg__CubePose__init(cube_imu_calibration__msg__CubePose * msg)
{
  if (!msg) {
    return false;
  }
  // header
  if (!std_msgs__msg__Header__init(&msg->header)) {
    cube_imu_calibration__msg__CubePose__fini(msg);
    return false;
  }
  // translation
  if (!geometry_msgs__msg__Vector3__init(&msg->translation)) {
    cube_imu_calibration__msg__CubePose__fini(msg);
    return false;
  }
  // orientation
  if (!geometry_msgs__msg__Quaternion__init(&msg->orientation)) {
    cube_imu_calibration__msg__CubePose__fini(msg);
    return false;
  }
  // transform
  return true;
}

void
cube_imu_calibration__msg__CubePose__fini(cube_imu_calibration__msg__CubePose * msg)
{
  if (!msg) {
    return;
  }
  // header
  std_msgs__msg__Header__fini(&msg->header);
  // translation
  geometry_msgs__msg__Vector3__fini(&msg->translation);
  // orientation
  geometry_msgs__msg__Quaternion__fini(&msg->orientation);
  // transform
}

bool
cube_imu_calibration__msg__CubePose__are_equal(const cube_imu_calibration__msg__CubePose * lhs, const cube_imu_calibration__msg__CubePose * rhs)
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
  // translation
  if (!geometry_msgs__msg__Vector3__are_equal(
      &(lhs->translation), &(rhs->translation)))
  {
    return false;
  }
  // orientation
  if (!geometry_msgs__msg__Quaternion__are_equal(
      &(lhs->orientation), &(rhs->orientation)))
  {
    return false;
  }
  // transform
  for (size_t i = 0; i < 16; ++i) {
    if (lhs->transform[i] != rhs->transform[i]) {
      return false;
    }
  }
  return true;
}

bool
cube_imu_calibration__msg__CubePose__copy(
  const cube_imu_calibration__msg__CubePose * input,
  cube_imu_calibration__msg__CubePose * output)
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
  // translation
  if (!geometry_msgs__msg__Vector3__copy(
      &(input->translation), &(output->translation)))
  {
    return false;
  }
  // orientation
  if (!geometry_msgs__msg__Quaternion__copy(
      &(input->orientation), &(output->orientation)))
  {
    return false;
  }
  // transform
  for (size_t i = 0; i < 16; ++i) {
    output->transform[i] = input->transform[i];
  }
  return true;
}

cube_imu_calibration__msg__CubePose *
cube_imu_calibration__msg__CubePose__create(void)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  cube_imu_calibration__msg__CubePose * msg = (cube_imu_calibration__msg__CubePose *)allocator.allocate(sizeof(cube_imu_calibration__msg__CubePose), allocator.state);
  if (!msg) {
    return NULL;
  }
  memset(msg, 0, sizeof(cube_imu_calibration__msg__CubePose));
  bool success = cube_imu_calibration__msg__CubePose__init(msg);
  if (!success) {
    allocator.deallocate(msg, allocator.state);
    return NULL;
  }
  return msg;
}

void
cube_imu_calibration__msg__CubePose__destroy(cube_imu_calibration__msg__CubePose * msg)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (msg) {
    cube_imu_calibration__msg__CubePose__fini(msg);
  }
  allocator.deallocate(msg, allocator.state);
}


bool
cube_imu_calibration__msg__CubePose__Sequence__init(cube_imu_calibration__msg__CubePose__Sequence * array, size_t size)
{
  if (!array) {
    return false;
  }
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  cube_imu_calibration__msg__CubePose * data = NULL;

  if (size) {
    data = (cube_imu_calibration__msg__CubePose *)allocator.zero_allocate(size, sizeof(cube_imu_calibration__msg__CubePose), allocator.state);
    if (!data) {
      return false;
    }
    // initialize all array elements
    size_t i;
    for (i = 0; i < size; ++i) {
      bool success = cube_imu_calibration__msg__CubePose__init(&data[i]);
      if (!success) {
        break;
      }
    }
    if (i < size) {
      // if initialization failed finalize the already initialized array elements
      for (; i > 0; --i) {
        cube_imu_calibration__msg__CubePose__fini(&data[i - 1]);
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
cube_imu_calibration__msg__CubePose__Sequence__fini(cube_imu_calibration__msg__CubePose__Sequence * array)
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
      cube_imu_calibration__msg__CubePose__fini(&array->data[i]);
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

cube_imu_calibration__msg__CubePose__Sequence *
cube_imu_calibration__msg__CubePose__Sequence__create(size_t size)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  cube_imu_calibration__msg__CubePose__Sequence * array = (cube_imu_calibration__msg__CubePose__Sequence *)allocator.allocate(sizeof(cube_imu_calibration__msg__CubePose__Sequence), allocator.state);
  if (!array) {
    return NULL;
  }
  bool success = cube_imu_calibration__msg__CubePose__Sequence__init(array, size);
  if (!success) {
    allocator.deallocate(array, allocator.state);
    return NULL;
  }
  return array;
}

void
cube_imu_calibration__msg__CubePose__Sequence__destroy(cube_imu_calibration__msg__CubePose__Sequence * array)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (array) {
    cube_imu_calibration__msg__CubePose__Sequence__fini(array);
  }
  allocator.deallocate(array, allocator.state);
}

bool
cube_imu_calibration__msg__CubePose__Sequence__are_equal(const cube_imu_calibration__msg__CubePose__Sequence * lhs, const cube_imu_calibration__msg__CubePose__Sequence * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  if (lhs->size != rhs->size) {
    return false;
  }
  for (size_t i = 0; i < lhs->size; ++i) {
    if (!cube_imu_calibration__msg__CubePose__are_equal(&(lhs->data[i]), &(rhs->data[i]))) {
      return false;
    }
  }
  return true;
}

bool
cube_imu_calibration__msg__CubePose__Sequence__copy(
  const cube_imu_calibration__msg__CubePose__Sequence * input,
  cube_imu_calibration__msg__CubePose__Sequence * output)
{
  if (!input || !output) {
    return false;
  }
  if (output->capacity < input->size) {
    const size_t allocation_size =
      input->size * sizeof(cube_imu_calibration__msg__CubePose);
    rcutils_allocator_t allocator = rcutils_get_default_allocator();
    cube_imu_calibration__msg__CubePose * data =
      (cube_imu_calibration__msg__CubePose *)allocator.reallocate(
      output->data, allocation_size, allocator.state);
    if (!data) {
      return false;
    }
    // If reallocation succeeded, memory may or may not have been moved
    // to fulfill the allocation request, invalidating output->data.
    output->data = data;
    for (size_t i = output->capacity; i < input->size; ++i) {
      if (!cube_imu_calibration__msg__CubePose__init(&output->data[i])) {
        // If initialization of any new item fails, roll back
        // all previously initialized items. Existing items
        // in output are to be left unmodified.
        for (; i-- > output->capacity; ) {
          cube_imu_calibration__msg__CubePose__fini(&output->data[i]);
        }
        return false;
      }
    }
    output->capacity = input->size;
  }
  output->size = input->size;
  for (size_t i = 0; i < input->size; ++i) {
    if (!cube_imu_calibration__msg__CubePose__copy(
        &(input->data[i]), &(output->data[i])))
    {
      return false;
    }
  }
  return true;
}
