// generated from rosidl_typesupport_introspection_c/resource/idl__type_support.c.em
// with input from cube_imu_calibration:msg/CubePoseStatus.idl
// generated code does not contain a copyright notice

#include <stddef.h>
#include "cube_imu_calibration/msg/detail/cube_pose_status__rosidl_typesupport_introspection_c.h"
#include "cube_imu_calibration/msg/rosidl_typesupport_introspection_c__visibility_control.h"
#include "rosidl_typesupport_introspection_c/field_types.h"
#include "rosidl_typesupport_introspection_c/identifier.h"
#include "rosidl_typesupport_introspection_c/message_introspection.h"
#include "cube_imu_calibration/msg/detail/cube_pose_status__functions.h"
#include "cube_imu_calibration/msg/detail/cube_pose_status__struct.h"


// Include directives for member types
// Member `header`
#include "std_msgs/msg/header.h"
// Member `header`
#include "std_msgs/msg/detail/header__rosidl_typesupport_introspection_c.h"
// Member `tag_ids`
#include "rosidl_runtime_c/primitives_sequence_functions.h"
// Member `message`
#include "rosidl_runtime_c/string_functions.h"

#ifdef __cplusplus
extern "C"
{
#endif

void cube_imu_calibration__msg__CubePoseStatus__rosidl_typesupport_introspection_c__CubePoseStatus_init_function(
  void * message_memory, enum rosidl_runtime_c__message_initialization _init)
{
  // TODO(karsten1987): initializers are not yet implemented for typesupport c
  // see https://github.com/ros2/ros2/issues/397
  (void) _init;
  cube_imu_calibration__msg__CubePoseStatus__init(message_memory);
}

void cube_imu_calibration__msg__CubePoseStatus__rosidl_typesupport_introspection_c__CubePoseStatus_fini_function(void * message_memory)
{
  cube_imu_calibration__msg__CubePoseStatus__fini(message_memory);
}

size_t cube_imu_calibration__msg__CubePoseStatus__rosidl_typesupport_introspection_c__size_function__CubePoseStatus__tag_ids(
  const void * untyped_member)
{
  const rosidl_runtime_c__int32__Sequence * member =
    (const rosidl_runtime_c__int32__Sequence *)(untyped_member);
  return member->size;
}

const void * cube_imu_calibration__msg__CubePoseStatus__rosidl_typesupport_introspection_c__get_const_function__CubePoseStatus__tag_ids(
  const void * untyped_member, size_t index)
{
  const rosidl_runtime_c__int32__Sequence * member =
    (const rosidl_runtime_c__int32__Sequence *)(untyped_member);
  return &member->data[index];
}

void * cube_imu_calibration__msg__CubePoseStatus__rosidl_typesupport_introspection_c__get_function__CubePoseStatus__tag_ids(
  void * untyped_member, size_t index)
{
  rosidl_runtime_c__int32__Sequence * member =
    (rosidl_runtime_c__int32__Sequence *)(untyped_member);
  return &member->data[index];
}

void cube_imu_calibration__msg__CubePoseStatus__rosidl_typesupport_introspection_c__fetch_function__CubePoseStatus__tag_ids(
  const void * untyped_member, size_t index, void * untyped_value)
{
  const int32_t * item =
    ((const int32_t *)
    cube_imu_calibration__msg__CubePoseStatus__rosidl_typesupport_introspection_c__get_const_function__CubePoseStatus__tag_ids(untyped_member, index));
  int32_t * value =
    (int32_t *)(untyped_value);
  *value = *item;
}

void cube_imu_calibration__msg__CubePoseStatus__rosidl_typesupport_introspection_c__assign_function__CubePoseStatus__tag_ids(
  void * untyped_member, size_t index, const void * untyped_value)
{
  int32_t * item =
    ((int32_t *)
    cube_imu_calibration__msg__CubePoseStatus__rosidl_typesupport_introspection_c__get_function__CubePoseStatus__tag_ids(untyped_member, index));
  const int32_t * value =
    (const int32_t *)(untyped_value);
  *item = *value;
}

bool cube_imu_calibration__msg__CubePoseStatus__rosidl_typesupport_introspection_c__resize_function__CubePoseStatus__tag_ids(
  void * untyped_member, size_t size)
{
  rosidl_runtime_c__int32__Sequence * member =
    (rosidl_runtime_c__int32__Sequence *)(untyped_member);
  rosidl_runtime_c__int32__Sequence__fini(member);
  return rosidl_runtime_c__int32__Sequence__init(member, size);
}

static rosidl_typesupport_introspection_c__MessageMember cube_imu_calibration__msg__CubePoseStatus__rosidl_typesupport_introspection_c__CubePoseStatus_message_member_array[6] = {
  {
    "header",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_MESSAGE,  // type
    0,  // upper bound of string
    NULL,  // members of sub message (initialized later)
    false,  // is key
    false,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(cube_imu_calibration__msg__CubePoseStatus, header),  // bytes offset in struct
    NULL,  // default value
    NULL,  // size() function pointer
    NULL,  // get_const(index) function pointer
    NULL,  // get(index) function pointer
    NULL,  // fetch(index, &value) function pointer
    NULL,  // assign(index, value) function pointer
    NULL  // resize(index) function pointer
  },
  {
    "pose_valid",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_BOOLEAN,  // type
    0,  // upper bound of string
    NULL,  // members of sub message
    false,  // is key
    false,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(cube_imu_calibration__msg__CubePoseStatus, pose_valid),  // bytes offset in struct
    NULL,  // default value
    NULL,  // size() function pointer
    NULL,  // get_const(index) function pointer
    NULL,  // get(index) function pointer
    NULL,  // fetch(index, &value) function pointer
    NULL,  // assign(index, value) function pointer
    NULL  // resize(index) function pointer
  },
  {
    "tag_count",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_UINT32,  // type
    0,  // upper bound of string
    NULL,  // members of sub message
    false,  // is key
    false,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(cube_imu_calibration__msg__CubePoseStatus, tag_count),  // bytes offset in struct
    NULL,  // default value
    NULL,  // size() function pointer
    NULL,  // get_const(index) function pointer
    NULL,  // get(index) function pointer
    NULL,  // fetch(index, &value) function pointer
    NULL,  // assign(index, value) function pointer
    NULL  // resize(index) function pointer
  },
  {
    "tag_ids",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_INT32,  // type
    0,  // upper bound of string
    NULL,  // members of sub message
    false,  // is key
    true,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(cube_imu_calibration__msg__CubePoseStatus, tag_ids),  // bytes offset in struct
    NULL,  // default value
    cube_imu_calibration__msg__CubePoseStatus__rosidl_typesupport_introspection_c__size_function__CubePoseStatus__tag_ids,  // size() function pointer
    cube_imu_calibration__msg__CubePoseStatus__rosidl_typesupport_introspection_c__get_const_function__CubePoseStatus__tag_ids,  // get_const(index) function pointer
    cube_imu_calibration__msg__CubePoseStatus__rosidl_typesupport_introspection_c__get_function__CubePoseStatus__tag_ids,  // get(index) function pointer
    cube_imu_calibration__msg__CubePoseStatus__rosidl_typesupport_introspection_c__fetch_function__CubePoseStatus__tag_ids,  // fetch(index, &value) function pointer
    cube_imu_calibration__msg__CubePoseStatus__rosidl_typesupport_introspection_c__assign_function__CubePoseStatus__tag_ids,  // assign(index, value) function pointer
    cube_imu_calibration__msg__CubePoseStatus__rosidl_typesupport_introspection_c__resize_function__CubePoseStatus__tag_ids  // resize(index) function pointer
  },
  {
    "reproj_error",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_DOUBLE,  // type
    0,  // upper bound of string
    NULL,  // members of sub message
    false,  // is key
    false,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(cube_imu_calibration__msg__CubePoseStatus, reproj_error),  // bytes offset in struct
    NULL,  // default value
    NULL,  // size() function pointer
    NULL,  // get_const(index) function pointer
    NULL,  // get(index) function pointer
    NULL,  // fetch(index, &value) function pointer
    NULL,  // assign(index, value) function pointer
    NULL  // resize(index) function pointer
  },
  {
    "message",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_STRING,  // type
    0,  // upper bound of string
    NULL,  // members of sub message
    false,  // is key
    false,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(cube_imu_calibration__msg__CubePoseStatus, message),  // bytes offset in struct
    NULL,  // default value
    NULL,  // size() function pointer
    NULL,  // get_const(index) function pointer
    NULL,  // get(index) function pointer
    NULL,  // fetch(index, &value) function pointer
    NULL,  // assign(index, value) function pointer
    NULL  // resize(index) function pointer
  }
};

static const rosidl_typesupport_introspection_c__MessageMembers cube_imu_calibration__msg__CubePoseStatus__rosidl_typesupport_introspection_c__CubePoseStatus_message_members = {
  "cube_imu_calibration__msg",  // message namespace
  "CubePoseStatus",  // message name
  6,  // number of fields
  sizeof(cube_imu_calibration__msg__CubePoseStatus),
  false,  // has_any_key_member_
  cube_imu_calibration__msg__CubePoseStatus__rosidl_typesupport_introspection_c__CubePoseStatus_message_member_array,  // message members
  cube_imu_calibration__msg__CubePoseStatus__rosidl_typesupport_introspection_c__CubePoseStatus_init_function,  // function to initialize message memory (memory has to be allocated)
  cube_imu_calibration__msg__CubePoseStatus__rosidl_typesupport_introspection_c__CubePoseStatus_fini_function  // function to terminate message instance (will not free memory)
};

// this is not const since it must be initialized on first access
// since C does not allow non-integral compile-time constants
static rosidl_message_type_support_t cube_imu_calibration__msg__CubePoseStatus__rosidl_typesupport_introspection_c__CubePoseStatus_message_type_support_handle = {
  0,
  &cube_imu_calibration__msg__CubePoseStatus__rosidl_typesupport_introspection_c__CubePoseStatus_message_members,
  get_message_typesupport_handle_function,
  &cube_imu_calibration__msg__CubePoseStatus__get_type_hash,
  &cube_imu_calibration__msg__CubePoseStatus__get_type_description,
  &cube_imu_calibration__msg__CubePoseStatus__get_type_description_sources,
};

ROSIDL_TYPESUPPORT_INTROSPECTION_C_EXPORT_cube_imu_calibration
const rosidl_message_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_c, cube_imu_calibration, msg, CubePoseStatus)() {
  cube_imu_calibration__msg__CubePoseStatus__rosidl_typesupport_introspection_c__CubePoseStatus_message_member_array[0].members_ =
    ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_c, std_msgs, msg, Header)();
  if (!cube_imu_calibration__msg__CubePoseStatus__rosidl_typesupport_introspection_c__CubePoseStatus_message_type_support_handle.typesupport_identifier) {
    cube_imu_calibration__msg__CubePoseStatus__rosidl_typesupport_introspection_c__CubePoseStatus_message_type_support_handle.typesupport_identifier =
      rosidl_typesupport_introspection_c__identifier;
  }
  return &cube_imu_calibration__msg__CubePoseStatus__rosidl_typesupport_introspection_c__CubePoseStatus_message_type_support_handle;
}
#ifdef __cplusplus
}
#endif
