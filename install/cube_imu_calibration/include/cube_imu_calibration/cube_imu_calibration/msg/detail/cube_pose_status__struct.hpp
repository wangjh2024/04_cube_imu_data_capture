// generated from rosidl_generator_cpp/resource/idl__struct.hpp.em
// with input from cube_imu_calibration:msg/CubePoseStatus.idl
// generated code does not contain a copyright notice

// IWYU pragma: private, include "cube_imu_calibration/msg/cube_pose_status.hpp"


#ifndef CUBE_IMU_CALIBRATION__MSG__DETAIL__CUBE_POSE_STATUS__STRUCT_HPP_
#define CUBE_IMU_CALIBRATION__MSG__DETAIL__CUBE_POSE_STATUS__STRUCT_HPP_

#include <algorithm>
#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "rosidl_runtime_cpp/bounded_vector.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


// Include directives for member types
// Member 'header'
#include "std_msgs/msg/detail/header__struct.hpp"

#ifndef _WIN32
# define DEPRECATED__cube_imu_calibration__msg__CubePoseStatus __attribute__((deprecated))
#else
# define DEPRECATED__cube_imu_calibration__msg__CubePoseStatus __declspec(deprecated)
#endif

namespace cube_imu_calibration
{

namespace msg
{

// message struct
template<class ContainerAllocator>
struct CubePoseStatus_
{
  using Type = CubePoseStatus_<ContainerAllocator>;

  explicit CubePoseStatus_(rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  : header(_init)
  {
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->pose_valid = false;
      this->tag_count = 0ul;
      this->reproj_error = 0.0;
      this->message = "";
    }
  }

  explicit CubePoseStatus_(const ContainerAllocator & _alloc, rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  : header(_alloc, _init),
    message(_alloc)
  {
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->pose_valid = false;
      this->tag_count = 0ul;
      this->reproj_error = 0.0;
      this->message = "";
    }
  }

  // field types and members
  using _header_type =
    std_msgs::msg::Header_<ContainerAllocator>;
  _header_type header;
  using _pose_valid_type =
    bool;
  _pose_valid_type pose_valid;
  using _tag_count_type =
    uint32_t;
  _tag_count_type tag_count;
  using _tag_ids_type =
    std::vector<int32_t, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<int32_t>>;
  _tag_ids_type tag_ids;
  using _reproj_error_type =
    double;
  _reproj_error_type reproj_error;
  using _message_type =
    std::basic_string<char, std::char_traits<char>, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<char>>;
  _message_type message;

  // setters for named parameter idiom
  Type & set__header(
    const std_msgs::msg::Header_<ContainerAllocator> & _arg)
  {
    this->header = _arg;
    return *this;
  }
  Type & set__pose_valid(
    const bool & _arg)
  {
    this->pose_valid = _arg;
    return *this;
  }
  Type & set__tag_count(
    const uint32_t & _arg)
  {
    this->tag_count = _arg;
    return *this;
  }
  Type & set__tag_ids(
    const std::vector<int32_t, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<int32_t>> & _arg)
  {
    this->tag_ids = _arg;
    return *this;
  }
  Type & set__reproj_error(
    const double & _arg)
  {
    this->reproj_error = _arg;
    return *this;
  }
  Type & set__message(
    const std::basic_string<char, std::char_traits<char>, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<char>> & _arg)
  {
    this->message = _arg;
    return *this;
  }

  // constant declarations

  // pointer types
  using RawPtr =
    cube_imu_calibration::msg::CubePoseStatus_<ContainerAllocator> *;
  using ConstRawPtr =
    const cube_imu_calibration::msg::CubePoseStatus_<ContainerAllocator> *;
  using SharedPtr =
    std::shared_ptr<cube_imu_calibration::msg::CubePoseStatus_<ContainerAllocator>>;
  using ConstSharedPtr =
    std::shared_ptr<cube_imu_calibration::msg::CubePoseStatus_<ContainerAllocator> const>;

  template<typename Deleter = std::default_delete<
      cube_imu_calibration::msg::CubePoseStatus_<ContainerAllocator>>>
  using UniquePtrWithDeleter =
    std::unique_ptr<cube_imu_calibration::msg::CubePoseStatus_<ContainerAllocator>, Deleter>;

  using UniquePtr = UniquePtrWithDeleter<>;

  template<typename Deleter = std::default_delete<
      cube_imu_calibration::msg::CubePoseStatus_<ContainerAllocator>>>
  using ConstUniquePtrWithDeleter =
    std::unique_ptr<cube_imu_calibration::msg::CubePoseStatus_<ContainerAllocator> const, Deleter>;
  using ConstUniquePtr = ConstUniquePtrWithDeleter<>;

  using WeakPtr =
    std::weak_ptr<cube_imu_calibration::msg::CubePoseStatus_<ContainerAllocator>>;
  using ConstWeakPtr =
    std::weak_ptr<cube_imu_calibration::msg::CubePoseStatus_<ContainerAllocator> const>;

  // pointer types similar to ROS 1, use SharedPtr / ConstSharedPtr instead
  // NOTE: Can't use 'using' here because GNU C++ can't parse attributes properly
  typedef DEPRECATED__cube_imu_calibration__msg__CubePoseStatus
    std::shared_ptr<cube_imu_calibration::msg::CubePoseStatus_<ContainerAllocator>>
    Ptr;
  typedef DEPRECATED__cube_imu_calibration__msg__CubePoseStatus
    std::shared_ptr<cube_imu_calibration::msg::CubePoseStatus_<ContainerAllocator> const>
    ConstPtr;

  // comparison operators
  bool operator==(const CubePoseStatus_ & other) const
  {
    if (this->header != other.header) {
      return false;
    }
    if (this->pose_valid != other.pose_valid) {
      return false;
    }
    if (this->tag_count != other.tag_count) {
      return false;
    }
    if (this->tag_ids != other.tag_ids) {
      return false;
    }
    if (this->reproj_error != other.reproj_error) {
      return false;
    }
    if (this->message != other.message) {
      return false;
    }
    return true;
  }
  bool operator!=(const CubePoseStatus_ & other) const
  {
    return !this->operator==(other);
  }
};  // struct CubePoseStatus_

// alias to use template instance with default allocator
using CubePoseStatus =
  cube_imu_calibration::msg::CubePoseStatus_<std::allocator<void>>;

// constant definitions

}  // namespace msg

}  // namespace cube_imu_calibration

#endif  // CUBE_IMU_CALIBRATION__MSG__DETAIL__CUBE_POSE_STATUS__STRUCT_HPP_
