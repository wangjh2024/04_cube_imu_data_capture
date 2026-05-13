// generated from rosidl_typesupport_introspection_c/resource/idl__type_support.c.em
// with input from cube_imu_calibration:msg/CubePose.idl
// generated code does not contain a copyright notice

#include <stddef.h>
#include "cube_imu_calibration/msg/detail/cube_pose__rosidl_typesupport_introspection_c.h"
#include "cube_imu_calibration/msg/rosidl_typesupport_introspection_c__visibility_control.h"
#include "rosidl_typesupport_introspection_c/field_types.h"
#include "rosidl_typesupport_introspection_c/identifier.h"
#include "rosidl_typesupport_introspection_c/message_introspection.h"
#include "cube_imu_calibration/msg/detail/cube_pose__functions.h"
#include "cube_imu_calibration/msg/detail/cube_pose__struct.h"


// Include directives for member types
// Member `header`
#include "std_msgs/msg/header.h"
// Member `header`
#include "std_msgs/msg/detail/header__rosidl_typesupport_introspection_c.h"
// Member `translation`
#include "geometry_msgs/msg/vector3.h"
// Member `translation`
#include "geometry_msgs/msg/detail/vector3__rosidl_typesupport_introspection_c.h"
// Member `orientation`
#include "geometry_msgs/msg/quaternion.h"
// Member `orientation`
#include "geometry_msgs/msg/detail/quaternion__rosidl_typesupport_introspection_c.h"

#ifdef __cplusplus
extern "C"
{
#endif

void cube_imu_calibration__msg__CubePose__rosidl_typesupport_introspection_c__CubePose_init_function(
  void * message_memory, enum rosidl_runtime_c__message_initialization _init)
{
  // TODO(karsten1987): initializers are not yet implemented for typesupport c
  // see https://github.com/ros2/ros2/issues/397
  (void) _init;
  cube_imu_calibration__msg__CubePose__init(message_memory);
}

void cube_imu_calibration__msg__CubePose__rosidl_typesupport_introspection_c__CubePose_fini_function(void * message_memory)
{
  cube_imu_calibration__msg__CubePose__fini(message_memory);
}

size_t cube_imu_calibration__msg__CubePose__rosidl_typesupport_introspection_c__size_function__CubePose__transform(
  const void * untyped_member)
{
  (void)untyped_member;
  return 16;
}

const void * cube_imu_calibration__msg__CubePose__rosidl_typesupport_introspection_c__get_const_function__CubePose__transform(
  const void * untyped_member, size_t index)
{
  const double * member =
    (const double *)(untyped_member);
  return &member[index];
}

void * cube_imu_calibration__msg__CubePose__rosidl_typesupport_introspection_c__get_function__CubePose__transform(
  void * untyped_member, size_t index)
{
  double * member =
    (double *)(untyped_member);
  return &member[index];
}

void cube_imu_calibration__msg__CubePose__rosidl_typesupport_introspection_c__fetch_function__CubePose__transform(
  const void * untyped_member, size_t index, void * untyped_value)
{
  const double * item =
    ((const double *)
    cube_imu_calibration__msg__CubePose__rosidl_typesupport_introspection_c__get_const_function__CubePose__transform(untyped_member, index));
  double * value =
    (double *)(untyped_value);
  *value = *item;
}

void cube_imu_calibration__msg__CubePose__rosidl_typesupport_introspection_c__assign_function__CubePose__transform(
  void * untyped_member, size_t index, const void * untyped_value)
{
  double * item =
    ((double *)
    cube_imu_calibration__msg__CubePose__rosidl_typesupport_introspection_c__get_function__CubePose__transform(untyped_member, index));
  const double * value =
    (const double *)(untyped_value);
  *item = *value;
}

static rosidl_typesupport_introspection_c__MessageMember cube_imu_calibration__msg__CubePose__rosidl_typesupport_introspection_c__CubePose_message_member_array[4] = {
  {
    "header",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_MESSAGE,  // type
    0,  // upper bound of string
    NULL,  // members of sub message (initialized later)
    false,  // is key
    false,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(cube_imu_calibration__msg__CubePose, header),  // bytes offset in struct
    NULL,  // default value
    NULL,  // size() function pointer
    NULL,  // get_const(index) function pointer
    NULL,  // get(index) function pointer
    NULL,  // fetch(index, &value) function pointer
    NULL,  // assign(index, value) function pointer
    NULL  // resize(index) function pointer
  },
  {
    "translation",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_MESSAGE,  // type
    0,  // upper bound of string
    NULL,  // members of sub message (initialized later)
    false,  // is key
    false,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(cube_imu_calibration__msg__CubePose, translation),  // bytes offset in struct
    NULL,  // default value
    NULL,  // size() function pointer
    NULL,  // get_const(index) function pointer
    NULL,  // get(index) function pointer
    NULL,  // fetch(index, &value) function pointer
    NULL,  // assign(index, value) function pointer
    NULL  // resize(index) function pointer
  },
  {
    "orientation",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_MESSAGE,  // type
    0,  // upper bound of string
    NULL,  // members of sub message (initialized later)
    false,  // is key
    false,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(cube_imu_calibration__msg__CubePose, orientation),  // bytes offset in struct
    NULL,  // default value
    NULL,  // size() function pointer
    NULL,  // get_const(index) function pointer
    NULL,  // get(index) function pointer
    NULL,  // fetch(index, &value) function pointer
    NULL,  // assign(index, value) function pointer
    NULL  // resize(index) function pointer
  },
  {
    "transform",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_DOUBLE,  // type
    0,  // upper bound of string
    NULL,  // members of sub message
    false,  // is key
    true,  // is array
    16,  // array size
    false,  // is upper bound
    offsetof(cube_imu_calibration__msg__CubePose, transform),  // bytes offset in struct
    NULL,  // default value
    cube_imu_calibration__msg__CubePose__rosidl_typesupport_introspection_c__size_function__CubePose__transform,  // size() function pointer
    cube_imu_calibration__msg__CubePose__rosidl_typesupport_introspection_c__get_const_function__CubePose__transform,  // get_const(index) function pointer
    cube_imu_calibration__msg__CubePose__rosidl_typesupport_introspection_c__get_function__CubePose__transform,  // get(index) function pointer
    cube_imu_calibration__msg__CubePose__rosidl_typesupport_introspection_c__fetch_function__CubePose__transform,  // fetch(index, &value) function pointer
    cube_imu_calibration__msg__CubePose__rosidl_typesupport_introspection_c__assign_function__CubePose__transform,  // assign(index, value) function pointer
    NULL  // resize(index) function pointer
  }
};

static const rosidl_typesupport_introspection_c__MessageMembers cube_imu_calibration__msg__CubePose__rosidl_typesupport_introspection_c__CubePose_message_members = {
  "cube_imu_calibration__msg",  // message namespace
  "CubePose",  // message name
  4,  // number of fields
  sizeof(cube_imu_calibration__msg__CubePose),
  false,  // has_any_key_member_
  cube_imu_calibration__msg__CubePose__rosidl_typesupport_introspection_c__CubePose_message_member_array,  // message members
  cube_imu_calibration__msg__CubePose__rosidl_typesupport_introspection_c__CubePose_init_function,  // function to initialize message memory (memory has to be allocated)
  cube_imu_calibration__msg__CubePose__rosidl_typesupport_introspection_c__CubePose_fini_function  // function to terminate message instance (will not free memory)
};

// this is not const since it must be initialized on first access
// since C does not allow non-integral compile-time constants
static rosidl_message_type_support_t cube_imu_calibration__msg__CubePose__rosidl_typesupport_introspection_c__CubePose_message_type_support_handle = {
  0,
  &cube_imu_calibration__msg__CubePose__rosidl_typesupport_introspection_c__CubePose_message_members,
  get_message_typesupport_handle_function,
  &cube_imu_calibration__msg__CubePose__get_type_hash,
  &cube_imu_calibration__msg__CubePose__get_type_description,
  &cube_imu_calibration__msg__CubePose__get_type_description_sources,
};

ROSIDL_TYPESUPPORT_INTROSPECTION_C_EXPORT_cube_imu_calibration
const rosidl_message_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_c, cube_imu_calibration, msg, CubePose)() {
  cube_imu_calibration__msg__CubePose__rosidl_typesupport_introspection_c__CubePose_message_member_array[0].members_ =
    ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_c, std_msgs, msg, Header)();
  cube_imu_calibration__msg__CubePose__rosidl_typesupport_introspection_c__CubePose_message_member_array[1].members_ =
    ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_c, geometry_msgs, msg, Vector3)();
  cube_imu_calibration__msg__CubePose__rosidl_typesupport_introspection_c__CubePose_message_member_array[2].members_ =
    ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_c, geometry_msgs, msg, Quaternion)();
  if (!cube_imu_calibration__msg__CubePose__rosidl_typesupport_introspection_c__CubePose_message_type_support_handle.typesupport_identifier) {
    cube_imu_calibration__msg__CubePose__rosidl_typesupport_introspection_c__CubePose_message_type_support_handle.typesupport_identifier =
      rosidl_typesupport_introspection_c__identifier;
  }
  return &cube_imu_calibration__msg__CubePose__rosidl_typesupport_introspection_c__CubePose_message_type_support_handle;
}
#ifdef __cplusplus
}
#endif
