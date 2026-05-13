// generated from rosidl_generator_c/resource/idl__functions.c.em
// with input from cube_imu_calibration:msg/RecorderStatus.idl
// generated code does not contain a copyright notice
#include "cube_imu_calibration/msg/detail/recorder_status__functions.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "rcutils/allocator.h"


// Include directives for member types
// Member `header`
#include "std_msgs/msg/detail/header__functions.h"
// Member `bag_path`
#include "rosidl_runtime_c/string_functions.h"

bool
cube_imu_calibration__msg__RecorderStatus__init(cube_imu_calibration__msg__RecorderStatus * msg)
{
  if (!msg) {
    return false;
  }
  // header
  if (!std_msgs__msg__Header__init(&msg->header)) {
    cube_imu_calibration__msg__RecorderStatus__fini(msg);
    return false;
  }
  // recording
  // bag_path
  if (!rosidl_runtime_c__String__init(&msg->bag_path)) {
    cube_imu_calibration__msg__RecorderStatus__fini(msg);
    return false;
  }
  // image_count
  // imu_count
  // camera_info_count
  // cube_pose_count
  // cube_pose_status_count
  // image_publishers
  // imu_publishers
  // camera_info_publishers
  // cube_pose_publishers
  // cube_pose_status_publishers
  // elapsed_sec
  // target_duration_sec
  // progress_ratio
  return true;
}

void
cube_imu_calibration__msg__RecorderStatus__fini(cube_imu_calibration__msg__RecorderStatus * msg)
{
  if (!msg) {
    return;
  }
  // header
  std_msgs__msg__Header__fini(&msg->header);
  // recording
  // bag_path
  rosidl_runtime_c__String__fini(&msg->bag_path);
  // image_count
  // imu_count
  // camera_info_count
  // cube_pose_count
  // cube_pose_status_count
  // image_publishers
  // imu_publishers
  // camera_info_publishers
  // cube_pose_publishers
  // cube_pose_status_publishers
  // elapsed_sec
  // target_duration_sec
  // progress_ratio
}

bool
cube_imu_calibration__msg__RecorderStatus__are_equal(const cube_imu_calibration__msg__RecorderStatus * lhs, const cube_imu_calibration__msg__RecorderStatus * rhs)
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
  // recording
  if (lhs->recording != rhs->recording) {
    return false;
  }
  // bag_path
  if (!rosidl_runtime_c__String__are_equal(
      &(lhs->bag_path), &(rhs->bag_path)))
  {
    return false;
  }
  // image_count
  if (lhs->image_count != rhs->image_count) {
    return false;
  }
  // imu_count
  if (lhs->imu_count != rhs->imu_count) {
    return false;
  }
  // camera_info_count
  if (lhs->camera_info_count != rhs->camera_info_count) {
    return false;
  }
  // cube_pose_count
  if (lhs->cube_pose_count != rhs->cube_pose_count) {
    return false;
  }
  // cube_pose_status_count
  if (lhs->cube_pose_status_count != rhs->cube_pose_status_count) {
    return false;
  }
  // image_publishers
  if (lhs->image_publishers != rhs->image_publishers) {
    return false;
  }
  // imu_publishers
  if (lhs->imu_publishers != rhs->imu_publishers) {
    return false;
  }
  // camera_info_publishers
  if (lhs->camera_info_publishers != rhs->camera_info_publishers) {
    return false;
  }
  // cube_pose_publishers
  if (lhs->cube_pose_publishers != rhs->cube_pose_publishers) {
    return false;
  }
  // cube_pose_status_publishers
  if (lhs->cube_pose_status_publishers != rhs->cube_pose_status_publishers) {
    return false;
  }
  // elapsed_sec
  if (lhs->elapsed_sec != rhs->elapsed_sec) {
    return false;
  }
  // target_duration_sec
  if (lhs->target_duration_sec != rhs->target_duration_sec) {
    return false;
  }
  // progress_ratio
  if (lhs->progress_ratio != rhs->progress_ratio) {
    return false;
  }
  return true;
}

bool
cube_imu_calibration__msg__RecorderStatus__copy(
  const cube_imu_calibration__msg__RecorderStatus * input,
  cube_imu_calibration__msg__RecorderStatus * output)
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
  // recording
  output->recording = input->recording;
  // bag_path
  if (!rosidl_runtime_c__String__copy(
      &(input->bag_path), &(output->bag_path)))
  {
    return false;
  }
  // image_count
  output->image_count = input->image_count;
  // imu_count
  output->imu_count = input->imu_count;
  // camera_info_count
  output->camera_info_count = input->camera_info_count;
  // cube_pose_count
  output->cube_pose_count = input->cube_pose_count;
  // cube_pose_status_count
  output->cube_pose_status_count = input->cube_pose_status_count;
  // image_publishers
  output->image_publishers = input->image_publishers;
  // imu_publishers
  output->imu_publishers = input->imu_publishers;
  // camera_info_publishers
  output->camera_info_publishers = input->camera_info_publishers;
  // cube_pose_publishers
  output->cube_pose_publishers = input->cube_pose_publishers;
  // cube_pose_status_publishers
  output->cube_pose_status_publishers = input->cube_pose_status_publishers;
  // elapsed_sec
  output->elapsed_sec = input->elapsed_sec;
  // target_duration_sec
  output->target_duration_sec = input->target_duration_sec;
  // progress_ratio
  output->progress_ratio = input->progress_ratio;
  return true;
}

cube_imu_calibration__msg__RecorderStatus *
cube_imu_calibration__msg__RecorderStatus__create(void)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  cube_imu_calibration__msg__RecorderStatus * msg = (cube_imu_calibration__msg__RecorderStatus *)allocator.allocate(sizeof(cube_imu_calibration__msg__RecorderStatus), allocator.state);
  if (!msg) {
    return NULL;
  }
  memset(msg, 0, sizeof(cube_imu_calibration__msg__RecorderStatus));
  bool success = cube_imu_calibration__msg__RecorderStatus__init(msg);
  if (!success) {
    allocator.deallocate(msg, allocator.state);
    return NULL;
  }
  return msg;
}

void
cube_imu_calibration__msg__RecorderStatus__destroy(cube_imu_calibration__msg__RecorderStatus * msg)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (msg) {
    cube_imu_calibration__msg__RecorderStatus__fini(msg);
  }
  allocator.deallocate(msg, allocator.state);
}


bool
cube_imu_calibration__msg__RecorderStatus__Sequence__init(cube_imu_calibration__msg__RecorderStatus__Sequence * array, size_t size)
{
  if (!array) {
    return false;
  }
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  cube_imu_calibration__msg__RecorderStatus * data = NULL;

  if (size) {
    data = (cube_imu_calibration__msg__RecorderStatus *)allocator.zero_allocate(size, sizeof(cube_imu_calibration__msg__RecorderStatus), allocator.state);
    if (!data) {
      return false;
    }
    // initialize all array elements
    size_t i;
    for (i = 0; i < size; ++i) {
      bool success = cube_imu_calibration__msg__RecorderStatus__init(&data[i]);
      if (!success) {
        break;
      }
    }
    if (i < size) {
      // if initialization failed finalize the already initialized array elements
      for (; i > 0; --i) {
        cube_imu_calibration__msg__RecorderStatus__fini(&data[i - 1]);
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
cube_imu_calibration__msg__RecorderStatus__Sequence__fini(cube_imu_calibration__msg__RecorderStatus__Sequence * array)
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
      cube_imu_calibration__msg__RecorderStatus__fini(&array->data[i]);
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

cube_imu_calibration__msg__RecorderStatus__Sequence *
cube_imu_calibration__msg__RecorderStatus__Sequence__create(size_t size)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  cube_imu_calibration__msg__RecorderStatus__Sequence * array = (cube_imu_calibration__msg__RecorderStatus__Sequence *)allocator.allocate(sizeof(cube_imu_calibration__msg__RecorderStatus__Sequence), allocator.state);
  if (!array) {
    return NULL;
  }
  bool success = cube_imu_calibration__msg__RecorderStatus__Sequence__init(array, size);
  if (!success) {
    allocator.deallocate(array, allocator.state);
    return NULL;
  }
  return array;
}

void
cube_imu_calibration__msg__RecorderStatus__Sequence__destroy(cube_imu_calibration__msg__RecorderStatus__Sequence * array)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (array) {
    cube_imu_calibration__msg__RecorderStatus__Sequence__fini(array);
  }
  allocator.deallocate(array, allocator.state);
}

bool
cube_imu_calibration__msg__RecorderStatus__Sequence__are_equal(const cube_imu_calibration__msg__RecorderStatus__Sequence * lhs, const cube_imu_calibration__msg__RecorderStatus__Sequence * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  if (lhs->size != rhs->size) {
    return false;
  }
  for (size_t i = 0; i < lhs->size; ++i) {
    if (!cube_imu_calibration__msg__RecorderStatus__are_equal(&(lhs->data[i]), &(rhs->data[i]))) {
      return false;
    }
  }
  return true;
}

bool
cube_imu_calibration__msg__RecorderStatus__Sequence__copy(
  const cube_imu_calibration__msg__RecorderStatus__Sequence * input,
  cube_imu_calibration__msg__RecorderStatus__Sequence * output)
{
  if (!input || !output) {
    return false;
  }
  if (output->capacity < input->size) {
    const size_t allocation_size =
      input->size * sizeof(cube_imu_calibration__msg__RecorderStatus);
    rcutils_allocator_t allocator = rcutils_get_default_allocator();
    cube_imu_calibration__msg__RecorderStatus * data =
      (cube_imu_calibration__msg__RecorderStatus *)allocator.reallocate(
      output->data, allocation_size, allocator.state);
    if (!data) {
      return false;
    }
    // If reallocation succeeded, memory may or may not have been moved
    // to fulfill the allocation request, invalidating output->data.
    output->data = data;
    for (size_t i = output->capacity; i < input->size; ++i) {
      if (!cube_imu_calibration__msg__RecorderStatus__init(&output->data[i])) {
        // If initialization of any new item fails, roll back
        // all previously initialized items. Existing items
        // in output are to be left unmodified.
        for (; i-- > output->capacity; ) {
          cube_imu_calibration__msg__RecorderStatus__fini(&output->data[i]);
        }
        return false;
      }
    }
    output->capacity = input->size;
  }
  output->size = input->size;
  for (size_t i = 0; i < input->size; ++i) {
    if (!cube_imu_calibration__msg__RecorderStatus__copy(
        &(input->data[i]), &(output->data[i])))
    {
      return false;
    }
  }
  return true;
}
