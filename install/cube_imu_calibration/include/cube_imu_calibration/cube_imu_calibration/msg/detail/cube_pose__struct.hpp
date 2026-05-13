// generated from rosidl_generator_cpp/resource/idl__struct.hpp.em
// with input from cube_imu_calibration:msg/CubePose.idl
// generated code does not contain a copyright notice

// IWYU pragma: private, include "cube_imu_calibration/msg/cube_pose.hpp"


#ifndef CUBE_IMU_CALIBRATION__MSG__DETAIL__CUBE_POSE__STRUCT_HPP_
#define CUBE_IMU_CALIBRATION__MSG__DETAIL__CUBE_POSE__STRUCT_HPP_

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
// Member 'translation'
#include "geometry_msgs/msg/detail/vector3__struct.hpp"
// Member 'orientation'
#include "geometry_msgs/msg/detail/quaternion__struct.hpp"

#ifndef _WIN32
# define DEPRECATED__cube_imu_calibration__msg__CubePose __attribute__((deprecated))
#else
# define DEPRECATED__cube_imu_calibration__msg__CubePose __declspec(deprecated)
#endif

namespace cube_imu_calibration
{

namespace msg
{

// message struct
template<class ContainerAllocator>
struct CubePose_
{
  using Type = CubePose_<ContainerAllocator>;

  explicit CubePose_(rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  : header(_init),
    translation(_init),
    orientation(_init)
  {
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      std::fill<typename std::array<double, 16>::iterator, double>(this->transform.begin(), this->transform.end(), 0.0);
    }
  }

  explicit CubePose_(const ContainerAllocator & _alloc, rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  : header(_alloc, _init),
    translation(_alloc, _init),
    orientation(_alloc, _init),
    transform(_alloc)
  {
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      std::fill<typename std::array<double, 16>::iterator, double>(this->transform.begin(), this->transform.end(), 0.0);
    }
  }

  // field types and members
  using _header_type =
    std_msgs::msg::Header_<ContainerAllocator>;
  _header_type header;
  using _translation_type =
    geometry_msgs::msg::Vector3_<ContainerAllocator>;
  _translation_type translation;
  using _orientation_type =
    geometry_msgs::msg::Quaternion_<ContainerAllocator>;
  _orientation_type orientation;
  using _transform_type =
    std::array<double, 16>;
  _transform_type transform;

  // setters for named parameter idiom
  Type & set__header(
    const std_msgs::msg::Header_<ContainerAllocator> & _arg)
  {
    this->header = _arg;
    return *this;
  }
  Type & set__translation(
    const geometry_msgs::msg::Vector3_<ContainerAllocator> & _arg)
  {
    this->translation = _arg;
    return *this;
  }
  Type & set__orientation(
    const geometry_msgs::msg::Quaternion_<ContainerAllocator> & _arg)
  {
    this->orientation = _arg;
    return *this;
  }
  Type & set__transform(
    const std::array<double, 16> & _arg)
  {
    this->transform = _arg;
    return *this;
  }

  // constant declarations

  // pointer types
  using RawPtr =
    cube_imu_calibration::msg::CubePose_<ContainerAllocator> *;
  using ConstRawPtr =
    const cube_imu_calibration::msg::CubePose_<ContainerAllocator> *;
  using SharedPtr =
    std::shared_ptr<cube_imu_calibration::msg::CubePose_<ContainerAllocator>>;
  using ConstSharedPtr =
    std::shared_ptr<cube_imu_calibration::msg::CubePose_<ContainerAllocator> const>;

  template<typename Deleter = std::default_delete<
      cube_imu_calibration::msg::CubePose_<ContainerAllocator>>>
  using UniquePtrWithDeleter =
    std::unique_ptr<cube_imu_calibration::msg::CubePose_<ContainerAllocator>, Deleter>;

  using UniquePtr = UniquePtrWithDeleter<>;

  template<typename Deleter = std::default_delete<
      cube_imu_calibration::msg::CubePose_<ContainerAllocator>>>
  using ConstUniquePtrWithDeleter =
    std::unique_ptr<cube_imu_calibration::msg::CubePose_<ContainerAllocator> const, Deleter>;
  using ConstUniquePtr = ConstUniquePtrWithDeleter<>;

  using WeakPtr =
    std::weak_ptr<cube_imu_calibration::msg::CubePose_<ContainerAllocator>>;
  using ConstWeakPtr =
    std::weak_ptr<cube_imu_calibration::msg::CubePose_<ContainerAllocator> const>;

  // pointer types similar to ROS 1, use SharedPtr / ConstSharedPtr instead
  // NOTE: Can't use 'using' here because GNU C++ can't parse attributes properly
  typedef DEPRECATED__cube_imu_calibration__msg__CubePose
    std::shared_ptr<cube_imu_calibration::msg::CubePose_<ContainerAllocator>>
    Ptr;
  typedef DEPRECATED__cube_imu_calibration__msg__CubePose
    std::shared_ptr<cube_imu_calibration::msg::CubePose_<ContainerAllocator> const>
    ConstPtr;

  // comparison operators
  bool operator==(const CubePose_ & other) const
  {
    if (this->header != other.header) {
      return false;
    }
    if (this->translation != other.translation) {
      return false;
    }
    if (this->orientation != other.orientation) {
      return false;
    }
    if (this->transform != other.transform) {
      return false;
    }
    return true;
  }
  bool operator!=(const CubePose_ & other) const
  {
    return !this->operator==(other);
  }
};  // struct CubePose_

// alias to use template instance with default allocator
using CubePose =
  cube_imu_calibration::msg::CubePose_<std::allocator<void>>;

// constant definitions

}  // namespace msg

}  // namespace cube_imu_calibration

#endif  // CUBE_IMU_CALIBRATION__MSG__DETAIL__CUBE_POSE__STRUCT_HPP_
