// generated from rosidl_typesupport_introspection_cpp/resource/idl__type_support.cpp.em
// with input from cube_imu_calibration:msg/CubePoseStatus.idl
// generated code does not contain a copyright notice

#include "array"
#include "cstddef"
#include "string"
#include "vector"
#include "rosidl_runtime_c/message_type_support_struct.h"
#include "rosidl_typesupport_cpp/message_type_support.hpp"
#include "rosidl_typesupport_interface/macros.h"
#include "cube_imu_calibration/msg/detail/cube_pose_status__functions.h"
#include "cube_imu_calibration/msg/detail/cube_pose_status__struct.hpp"
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

void CubePoseStatus_init_function(
  void * message_memory, rosidl_runtime_cpp::MessageInitialization _init)
{
  new (message_memory) cube_imu_calibration::msg::CubePoseStatus(_init);
}

void CubePoseStatus_fini_function(void * message_memory)
{
  auto typed_message = static_cast<cube_imu_calibration::msg::CubePoseStatus *>(message_memory);
  typed_message->~CubePoseStatus();
}

size_t size_function__CubePoseStatus__tag_ids(const void * untyped_member)
{
  const auto * member = reinterpret_cast<const std::vector<int32_t> *>(untyped_member);
  return member->size();
}

const void * get_const_function__CubePoseStatus__tag_ids(const void * untyped_member, size_t index)
{
  const auto & member =
    *reinterpret_cast<const std::vector<int32_t> *>(untyped_member);
  return &member[index];
}

void * get_function__CubePoseStatus__tag_ids(void * untyped_member, size_t index)
{
  auto & member =
    *reinterpret_cast<std::vector<int32_t> *>(untyped_member);
  return &member[index];
}

void fetch_function__CubePoseStatus__tag_ids(
  const void * untyped_member, size_t index, void * untyped_value)
{
  const auto & item = *reinterpret_cast<const int32_t *>(
    get_const_function__CubePoseStatus__tag_ids(untyped_member, index));
  auto & value = *reinterpret_cast<int32_t *>(untyped_value);
  value = item;
}

void assign_function__CubePoseStatus__tag_ids(
  void * untyped_member, size_t index, const void * untyped_value)
{
  auto & item = *reinterpret_cast<int32_t *>(
    get_function__CubePoseStatus__tag_ids(untyped_member, index));
  const auto & value = *reinterpret_cast<const int32_t *>(untyped_value);
  item = value;
}

void resize_function__CubePoseStatus__tag_ids(void * untyped_member, size_t size)
{
  auto * member =
    reinterpret_cast<std::vector<int32_t> *>(untyped_member);
  member->resize(size);
}

static const ::rosidl_typesupport_introspection_cpp::MessageMember CubePoseStatus_message_member_array[6] = {
  {
    "header",  // name
    ::rosidl_typesupport_introspection_cpp::ROS_TYPE_MESSAGE,  // type
    0,  // upper bound of string
    ::rosidl_typesupport_introspection_cpp::get_message_type_support_handle<std_msgs::msg::Header>(),  // members of sub message
    false,  // is key
    false,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(cube_imu_calibration::msg::CubePoseStatus, header),  // bytes offset in struct
    nullptr,  // default value
    nullptr,  // size() function pointer
    nullptr,  // get_const(index) function pointer
    nullptr,  // get(index) function pointer
    nullptr,  // fetch(index, &value) function pointer
    nullptr,  // assign(index, value) function pointer
    nullptr  // resize(index) function pointer
  },
  {
    "pose_valid",  // name
    ::rosidl_typesupport_introspection_cpp::ROS_TYPE_BOOLEAN,  // type
    0,  // upper bound of string
    nullptr,  // members of sub message
    false,  // is key
    false,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(cube_imu_calibration::msg::CubePoseStatus, pose_valid),  // bytes offset in struct
    nullptr,  // default value
    nullptr,  // size() function pointer
    nullptr,  // get_const(index) function pointer
    nullptr,  // get(index) function pointer
    nullptr,  // fetch(index, &value) function pointer
    nullptr,  // assign(index, value) function pointer
    nullptr  // resize(index) function pointer
  },
  {
    "tag_count",  // name
    ::rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT32,  // type
    0,  // upper bound of string
    nullptr,  // members of sub message
    false,  // is key
    false,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(cube_imu_calibration::msg::CubePoseStatus, tag_count),  // bytes offset in struct
    nullptr,  // default value
    nullptr,  // size() function pointer
    nullptr,  // get_const(index) function pointer
    nullptr,  // get(index) function pointer
    nullptr,  // fetch(index, &value) function pointer
    nullptr,  // assign(index, value) function pointer
    nullptr  // resize(index) function pointer
  },
  {
    "tag_ids",  // name
    ::rosidl_typesupport_introspection_cpp::ROS_TYPE_INT32,  // type
    0,  // upper bound of string
    nullptr,  // members of sub message
    false,  // is key
    true,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(cube_imu_calibration::msg::CubePoseStatus, tag_ids),  // bytes offset in struct
    nullptr,  // default value
    size_function__CubePoseStatus__tag_ids,  // size() function pointer
    get_const_function__CubePoseStatus__tag_ids,  // get_const(index) function pointer
    get_function__CubePoseStatus__tag_ids,  // get(index) function pointer
    fetch_function__CubePoseStatus__tag_ids,  // fetch(index, &value) function pointer
    assign_function__CubePoseStatus__tag_ids,  // assign(index, value) function pointer
    resize_function__CubePoseStatus__tag_ids  // resize(index) function pointer
  },
  {
    "reproj_error",  // name
    ::rosidl_typesupport_introspection_cpp::ROS_TYPE_DOUBLE,  // type
    0,  // upper bound of string
    nullptr,  // members of sub message
    false,  // is key
    false,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(cube_imu_calibration::msg::CubePoseStatus, reproj_error),  // bytes offset in struct
    nullptr,  // default value
    nullptr,  // size() function pointer
    nullptr,  // get_const(index) function pointer
    nullptr,  // get(index) function pointer
    nullptr,  // fetch(index, &value) function pointer
    nullptr,  // assign(index, value) function pointer
    nullptr  // resize(index) function pointer
  },
  {
    "message",  // name
    ::rosidl_typesupport_introspection_cpp::ROS_TYPE_STRING,  // type
    0,  // upper bound of string
    nullptr,  // members of sub message
    false,  // is key
    false,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(cube_imu_calibration::msg::CubePoseStatus, message),  // bytes offset in struct
    nullptr,  // default value
    nullptr,  // size() function pointer
    nullptr,  // get_const(index) function pointer
    nullptr,  // get(index) function pointer
    nullptr,  // fetch(index, &value) function pointer
    nullptr,  // assign(index, value) function pointer
    nullptr  // resize(index) function pointer
  }
};

static const ::rosidl_typesupport_introspection_cpp::MessageMembers CubePoseStatus_message_members = {
  "cube_imu_calibration::msg",  // message namespace
  "CubePoseStatus",  // message name
  6,  // number of fields
  sizeof(cube_imu_calibration::msg::CubePoseStatus),
  false,  // has_any_key_member_
  CubePoseStatus_message_member_array,  // message members
  CubePoseStatus_init_function,  // function to initialize message memory (memory has to be allocated)
  CubePoseStatus_fini_function  // function to terminate message instance (will not free memory)
};

static const rosidl_message_type_support_t CubePoseStatus_message_type_support_handle = {
  ::rosidl_typesupport_introspection_cpp::typesupport_identifier,
  &CubePoseStatus_message_members,
  get_message_typesupport_handle_function,
  &cube_imu_calibration__msg__CubePoseStatus__get_type_hash,
  &cube_imu_calibration__msg__CubePoseStatus__get_type_description,
  &cube_imu_calibration__msg__CubePoseStatus__get_type_description_sources,
};

}  // namespace rosidl_typesupport_introspection_cpp

}  // namespace msg

}  // namespace cube_imu_calibration


namespace rosidl_typesupport_introspection_cpp
{

template<>
ROSIDL_TYPESUPPORT_INTROSPECTION_CPP_PUBLIC
const rosidl_message_type_support_t *
get_message_type_support_handle<cube_imu_calibration::msg::CubePoseStatus>()
{
  return &::cube_imu_calibration::msg::rosidl_typesupport_introspection_cpp::CubePoseStatus_message_type_support_handle;
}

}  // namespace rosidl_typesupport_introspection_cpp

#ifdef __cplusplus
extern "C"
{
#endif

ROSIDL_TYPESUPPORT_INTROSPECTION_CPP_PUBLIC
const rosidl_message_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_cpp, cube_imu_calibration, msg, CubePoseStatus)() {
  return &::cube_imu_calibration::msg::rosidl_typesupport_introspection_cpp::CubePoseStatus_message_type_support_handle;
}

#ifdef __cplusplus
}
#endif
