// generated from rosidl_typesupport_introspection_cpp/resource/idl__type_support.cpp.em
// with input from cube_imu_calibration:msg/CubePose.idl
// generated code does not contain a copyright notice

#include "array"
#include "cstddef"
#include "string"
#include "vector"
#include "rosidl_runtime_c/message_type_support_struct.h"
#include "rosidl_typesupport_cpp/message_type_support.hpp"
#include "rosidl_typesupport_interface/macros.h"
#include "cube_imu_calibration/msg/detail/cube_pose__functions.h"
#include "cube_imu_calibration/msg/detail/cube_pose__struct.hpp"
#include "rosidl_typesupport_introspection_cpp/field_types.hpp"
#include "rosidl_typesupport_introspection_cpp/identifier.hpp"
#include "rosidl_typesupport_introspection_cpp/message_introspection.hpp"
#include "rosidl_typesupport_introspection_cpp/message_type_support_decl.hpp"
#include "rosidl_typesupport_introspection_cpp/visibility_control.h"

namespace cube_imu_calibration
{

namespace msg
{

namespace rosidl_typesupport_introspection_cpp
{

void CubePose_init_function(
  void * message_memory, rosidl_runtime_cpp::MessageInitialization _init)
{
  new (message_memory) cube_imu_calibration::msg::CubePose(_init);
}

void CubePose_fini_function(void * message_memory)
{
  auto typed_message = static_cast<cube_imu_calibration::msg::CubePose *>(message_memory);
  typed_message->~CubePose();
}

size_t size_function__CubePose__transform(const void * untyped_member)
{
  (void)untyped_member;
  return 16;
}

const void * get_const_function__CubePose__transform(const void * untyped_member, size_t index)
{
  const auto & member =
    *reinterpret_cast<const std::array<double, 16> *>(untyped_member);
  return &member[index];
}

void * get_function__CubePose__transform(void * untyped_member, size_t index)
{
  auto & member =
    *reinterpret_cast<std::array<double, 16> *>(untyped_member);
  return &member[index];
}

void fetch_function__CubePose__transform(
  const void * untyped_member, size_t index, void * untyped_value)
{
  const auto & item = *reinterpret_cast<const double *>(
    get_const_function__CubePose__transform(untyped_member, index));
  auto & value = *reinterpret_cast<double *>(untyped_value);
  value = item;
}

void assign_function__CubePose__transform(
  void * untyped_member, size_t index, const void * untyped_value)
{
  auto & item = *reinterpret_cast<double *>(
    get_function__CubePose__transform(untyped_member, index));
  const auto & value = *reinterpret_cast<const double *>(untyped_value);
  item = value;
}

static const ::rosidl_typesupport_introspection_cpp::MessageMember CubePose_message_member_array[4] = {
  {
    "header",  // name
    ::rosidl_typesupport_introspection_cpp::ROS_TYPE_MESSAGE,  // type
    0,  // upper bound of string
    ::rosidl_typesupport_introspection_cpp::get_message_type_support_handle<std_msgs::msg::Header>(),  // members of sub message
    false,  // is key
    false,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(cube_imu_calibration::msg::CubePose, header),  // bytes offset in struct
    nullptr,  // default value
    nullptr,  // size() function pointer
    nullptr,  // get_const(index) function pointer
    nullptr,  // get(index) function pointer
    nullptr,  // fetch(index, &value) function pointer
    nullptr,  // assign(index, value) function pointer
    nullptr  // resize(index) function pointer
  },
  {
    "translation",  // name
    ::rosidl_typesupport_introspection_cpp::ROS_TYPE_MESSAGE,  // type
    0,  // upper bound of string
    ::rosidl_typesupport_introspection_cpp::get_message_type_support_handle<geometry_msgs::msg::Vector3>(),  // members of sub message
    false,  // is key
    false,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(cube_imu_calibration::msg::CubePose, translation),  // bytes offset in struct
    nullptr,  // default value
    nullptr,  // size() function pointer
    nullptr,  // get_const(index) function pointer
    nullptr,  // get(index) function pointer
    nullptr,  // fetch(index, &value) function pointer
    nullptr,  // assign(index, value) function pointer
    nullptr  // resize(index) function pointer
  },
  {
    "orientation",  // name
    ::rosidl_typesupport_introspection_cpp::ROS_TYPE_MESSAGE,  // type
    0,  // upper bound of string
    ::rosidl_typesupport_introspection_cpp::get_message_type_support_handle<geometry_msgs::msg::Quaternion>(),  // members of sub message
    false,  // is key
    false,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(cube_imu_calibration::msg::CubePose, orientation),  // bytes offset in struct
    nullptr,  // default value
    nullptr,  // size() function pointer
    nullptr,  // get_const(index) function pointer
    nullptr,  // get(index) function pointer
    nullptr,  // fetch(index, &value) function pointer
    nullptr,  // assign(index, value) function pointer
    nullptr  // resize(index) function pointer
  },
  {
    "transform",  // name
    ::rosidl_typesupport_introspection_cpp::ROS_TYPE_DOUBLE,  // type
    0,  // upper bound of string
    nullptr,  // members of sub message
    false,  // is key
    true,  // is array
    16,  // array size
    false,  // is upper bound
    offsetof(cube_imu_calibration::msg::CubePose, transform),  // bytes offset in struct
    nullptr,  // default value
    size_function__CubePose__transform,  // size() function pointer
    get_const_function__CubePose__transform,  // get_const(index) function pointer
    get_function__CubePose__transform,  // get(index) function pointer
    fetch_function__CubePose__transform,  // fetch(index, &value) function pointer
    assign_function__CubePose__transform,  // assign(index, value) function pointer
    nullptr  // resize(index) function pointer
  }
};

static const ::rosidl_typesupport_introspection_cpp::MessageMembers CubePose_message_members = {
  "cube_imu_calibration::msg",  // message namespace
  "CubePose",  // message name
  4,  // number of fields
  sizeof(cube_imu_calibration::msg::CubePose),
  false,  // has_any_key_member_
  CubePose_message_member_array,  // message members
  CubePose_init_function,  // function to initialize message memory (memory has to be allocated)
  CubePose_fini_function  // function to terminate message instance (will not free memory)
};

static const rosidl_message_type_support_t CubePose_message_type_support_handle = {
  ::rosidl_typesupport_introspection_cpp::typesupport_identifier,
  &CubePose_message_members,
  get_message_typesupport_handle_function,
  &cube_imu_calibration__msg__CubePose__get_type_hash,
  &cube_imu_calibration__msg__CubePose__get_type_description,
  &cube_imu_calibration__msg__CubePose__get_type_description_sources,
};

}  // namespace rosidl_typesupport_introspection_cpp

}  // namespace msg

}  // namespace cube_imu_calibration


namespace rosidl_typesupport_introspection_cpp
{

template<>
ROSIDL_TYPESUPPORT_INTROSPECTION_CPP_PUBLIC
const rosidl_message_type_support_t *
get_message_type_support_handle<cube_imu_calibration::msg::CubePose>()
{
  return &::cube_imu_calibration::msg::rosidl_typesupport_introspection_cpp::CubePose_message_type_support_handle;
}

}  // namespace rosidl_typesupport_introspection_cpp

#ifdef __cplusplus
extern "C"
{
#endif

ROSIDL_TYPESUPPORT_INTROSPECTION_CPP_PUBLIC
const rosidl_message_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_cpp, cube_imu_calibration, msg, CubePose)() {
  return &::cube_imu_calibration::msg::rosidl_typesupport_introspection_cpp::CubePose_message_type_support_handle;
}

#ifdef __cplusplus
}
#endif
