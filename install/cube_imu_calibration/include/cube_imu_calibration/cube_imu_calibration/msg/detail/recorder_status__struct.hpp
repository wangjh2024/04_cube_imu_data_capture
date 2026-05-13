// generated from rosidl_generator_cpp/resource/idl__struct.hpp.em
// with input from cube_imu_calibration:msg/RecorderStatus.idl
// generated code does not contain a copyright notice

// IWYU pragma: private, include "cube_imu_calibration/msg/recorder_status.hpp"


#ifndef CUBE_IMU_CALIBRATION__MSG__DETAIL__RECORDER_STATUS__STRUCT_HPP_
#define CUBE_IMU_CALIBRATION__MSG__DETAIL__RECORDER_STATUS__STRUCT_HPP_

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
# define DEPRECATED__cube_imu_calibration__msg__RecorderStatus __attribute__((deprecated))
#else
# define DEPRECATED__cube_imu_calibration__msg__RecorderStatus __declspec(deprecated)
#endif

namespace cube_imu_calibration
{

namespace msg
{

// message struct
template<class ContainerAllocator>
struct RecorderStatus_
{
  using Type = RecorderStatus_<ContainerAllocator>;

  explicit RecorderStatus_(rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  : header(_init)
  {
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->recording = false;
      this->bag_path = "";
      this->image_count = 0ull;
      this->imu_count = 0ull;
      this->camera_info_count = 0ull;
      this->cube_pose_count = 0ull;
      this->cube_pose_status_count = 0ull;
      this->image_publishers = 0ul;
      this->imu_publishers = 0ul;
      this->camera_info_publishers = 0ul;
      this->cube_pose_publishers = 0ul;
      this->cube_pose_status_publishers = 0ul;
      this->elapsed_sec = 0.0;
      this->target_duration_sec = 0.0;
      this->progress_ratio = 0.0;
    }
  }

  explicit RecorderStatus_(const ContainerAllocator & _alloc, rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  : header(_alloc, _init),
    bag_path(_alloc)
  {
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->recording = false;
      this->bag_path = "";
      this->image_count = 0ull;
      this->imu_count = 0ull;
      this->camera_info_count = 0ull;
      this->cube_pose_count = 0ull;
      this->cube_pose_status_count = 0ull;
      this->image_publishers = 0ul;
      this->imu_publishers = 0ul;
      this->camera_info_publishers = 0ul;
      this->cube_pose_publishers = 0ul;
      this->cube_pose_status_publishers = 0ul;
      this->elapsed_sec = 0.0;
      this->target_duration_sec = 0.0;
      this->progress_ratio = 0.0;
    }
  }

  // field types and members
  using _header_type =
    std_msgs::msg::Header_<ContainerAllocator>;
  _header_type header;
  using _recording_type =
    bool;
  _recording_type recording;
  using _bag_path_type =
    std::basic_string<char, std::char_traits<char>, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<char>>;
  _bag_path_type bag_path;
  using _image_count_type =
    uint64_t;
  _image_count_type image_count;
  using _imu_count_type =
    uint64_t;
  _imu_count_type imu_count;
  using _camera_info_count_type =
    uint64_t;
  _camera_info_count_type camera_info_count;
  using _cube_pose_count_type =
    uint64_t;
  _cube_pose_count_type cube_pose_count;
  using _cube_pose_status_count_type =
    uint64_t;
  _cube_pose_status_count_type cube_pose_status_count;
  using _image_publishers_type =
    uint32_t;
  _image_publishers_type image_publishers;
  using _imu_publishers_type =
    uint32_t;
  _imu_publishers_type imu_publishers;
  using _camera_info_publishers_type =
    uint32_t;
  _camera_info_publishers_type camera_info_publishers;
  using _cube_pose_publishers_type =
    uint32_t;
  _cube_pose_publishers_type cube_pose_publishers;
  using _cube_pose_status_publishers_type =
    uint32_t;
  _cube_pose_status_publishers_type cube_pose_status_publishers;
  using _elapsed_sec_type =
    double;
  _elapsed_sec_type elapsed_sec;
  using _target_duration_sec_type =
    double;
  _target_duration_sec_type target_duration_sec;
  using _progress_ratio_type =
    double;
  _progress_ratio_type progress_ratio;

  // setters for named parameter idiom
  Type & set__header(
    const std_msgs::msg::Header_<ContainerAllocator> & _arg)
  {
    this->header = _arg;
    return *this;
  }
  Type & set__recording(
    const bool & _arg)
  {
    this->recording = _arg;
    return *this;
  }
  Type & set__bag_path(
    const std::basic_string<char, std::char_traits<char>, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<char>> & _arg)
  {
    this->bag_path = _arg;
    return *this;
  }
  Type & set__image_count(
    const uint64_t & _arg)
  {
    this->image_count = _arg;
    return *this;
  }
  Type & set__imu_count(
    const uint64_t & _arg)
  {
    this->imu_count = _arg;
    return *this;
  }
  Type & set__camera_info_count(
    const uint64_t & _arg)
  {
    this->camera_info_count = _arg;
    return *this;
  }
  Type & set__cube_pose_count(
    const uint64_t & _arg)
  {
    this->cube_pose_count = _arg;
    return *this;
  }
  Type & set__cube_pose_status_count(
    const uint64_t & _arg)
  {
    this->cube_pose_status_count = _arg;
    return *this;
  }
  Type & set__image_publishers(
    const uint32_t & _arg)
  {
    this->image_publishers = _arg;
    return *this;
  }
  Type & set__imu_publishers(
    const uint32_t & _arg)
  {
    this->imu_publishers = _arg;
    return *this;
  }
  Type & set__camera_info_publishers(
    const uint32_t & _arg)
  {
    this->camera_info_publishers = _arg;
    return *this;
  }
  Type & set__cube_pose_publishers(
    const uint32_t & _arg)
  {
    this->cube_pose_publishers = _arg;
    return *this;
  }
  Type & set__cube_pose_status_publishers(
    const uint32_t & _arg)
  {
    this->cube_pose_status_publishers = _arg;
    return *this;
  }
  Type & set__elapsed_sec(
    const double & _arg)
  {
    this->elapsed_sec = _arg;
    return *this;
  }
  Type & set__target_duration_sec(
    const double & _arg)
  {
    this->target_duration_sec = _arg;
    return *this;
  }
  Type & set__progress_ratio(
    const double & _arg)
  {
    this->progress_ratio = _arg;
    return *this;
  }

  // constant declarations

  // pointer types
  using RawPtr =
    cube_imu_calibration::msg::RecorderStatus_<ContainerAllocator> *;
  using ConstRawPtr =
    const cube_imu_calibration::msg::RecorderStatus_<ContainerAllocator> *;
  using SharedPtr =
    std::shared_ptr<cube_imu_calibration::msg::RecorderStatus_<ContainerAllocator>>;
  using ConstSharedPtr =
    std::shared_ptr<cube_imu_calibration::msg::RecorderStatus_<ContainerAllocator> const>;

  template<typename Deleter = std::default_delete<
      cube_imu_calibration::msg::RecorderStatus_<ContainerAllocator>>>
  using UniquePtrWithDeleter =
    std::unique_ptr<cube_imu_calibration::msg::RecorderStatus_<ContainerAllocator>, Deleter>;

  using UniquePtr = UniquePtrWithDeleter<>;

  template<typename Deleter = std::default_delete<
      cube_imu_calibration::msg::RecorderStatus_<ContainerAllocator>>>
  using ConstUniquePtrWithDeleter =
    std::unique_ptr<cube_imu_calibration::msg::RecorderStatus_<ContainerAllocator> const, Deleter>;
  using ConstUniquePtr = ConstUniquePtrWithDeleter<>;

  using WeakPtr =
    std::weak_ptr<cube_imu_calibration::msg::RecorderStatus_<ContainerAllocator>>;
  using ConstWeakPtr =
    std::weak_ptr<cube_imu_calibration::msg::RecorderStatus_<ContainerAllocator> const>;

  // pointer types similar to ROS 1, use SharedPtr / ConstSharedPtr instead
  // NOTE: Can't use 'using' here because GNU C++ can't parse attributes properly
  typedef DEPRECATED__cube_imu_calibration__msg__RecorderStatus
    std::shared_ptr<cube_imu_calibration::msg::RecorderStatus_<ContainerAllocator>>
    Ptr;
  typedef DEPRECATED__cube_imu_calibration__msg__RecorderStatus
    std::shared_ptr<cube_imu_calibration::msg::RecorderStatus_<ContainerAllocator> const>
    ConstPtr;

  // comparison operators
  bool operator==(const RecorderStatus_ & other) const
  {
    if (this->header != other.header) {
      return false;
    }
    if (this->recording != other.recording) {
      return false;
    }
    if (this->bag_path != other.bag_path) {
      return false;
    }
    if (this->image_count != other.image_count) {
      return false;
    }
    if (this->imu_count != other.imu_count) {
      return false;
    }
    if (this->camera_info_count != other.camera_info_count) {
      return false;
    }
    if (this->cube_pose_count != other.cube_pose_count) {
      return false;
    }
    if (this->cube_pose_status_count != other.cube_pose_status_count) {
      return false;
    }
    if (this->image_publishers != other.image_publishers) {
      return false;
    }
    if (this->imu_publishers != other.imu_publishers) {
      return false;
    }
    if (this->camera_info_publishers != other.camera_info_publishers) {
      return false;
    }
    if (this->cube_pose_publishers != other.cube_pose_publishers) {
      return false;
    }
    if (this->cube_pose_status_publishers != other.cube_pose_status_publishers) {
      return false;
    }
    if (this->elapsed_sec != other.elapsed_sec) {
      return false;
    }
    if (this->target_duration_sec != other.target_duration_sec) {
      return false;
    }
    if (this->progress_ratio != other.progress_ratio) {
      return false;
    }
    return true;
  }
  bool operator!=(const RecorderStatus_ & other) const
  {
    return !this->operator==(other);
  }
};  // struct RecorderStatus_

// alias to use template instance with default allocator
using RecorderStatus =
  cube_imu_calibration::msg::RecorderStatus_<std::allocator<void>>;

// constant definitions

}  // namespace msg

}  // namespace cube_imu_calibration

#endif  // CUBE_IMU_CALIBRATION__MSG__DETAIL__RECORDER_STATUS__STRUCT_HPP_
