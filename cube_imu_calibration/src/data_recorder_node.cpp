#include <chrono>
#include <ctime>
#include <filesystem>
#include <iomanip>
#include <algorithm>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>

#include "rclcpp/rclcpp.hpp"
#include "rosbag2_cpp/writer.hpp"
#include "rosbag2_storage/storage_options.hpp"
#include "rosbag2_storage/topic_metadata.hpp"
#include "sensor_msgs/msg/camera_info.hpp"
#include "sensor_msgs/msg/image.hpp"
#include "sensor_msgs/msg/imu.hpp"
#include "std_srvs/srv/trigger.hpp"

#include "cube_imu_calibration/msg/cube_pose.hpp"
#include "cube_imu_calibration/msg/cube_pose_status.hpp"
#include "cube_imu_calibration/msg/recorder_status.hpp"

namespace cube_imu_calibration
{

class DataRecorderNode : public rclcpp::Node
{
public:
  DataRecorderNode()
  : Node("data_recorder_node")
  {
    image_topic_ = declare_parameter<std::string>("image_topic", "/camera/image_raw");
    imu_topic_ = declare_parameter<std::string>("imu_topic", "/imu");
    camera_info_topic_ = declare_parameter<std::string>(
      "camera_info_topic", "/camera/camera_info");
    cube_pose_topic_ = declare_parameter<std::string>("cube_pose_topic", "/cube_pose");
    cube_pose_status_topic_ = declare_parameter<std::string>(
      "cube_pose_status_topic", "/cube_pose_status");
    bag_path_ = declare_parameter<std::string>("bag_path", "output_bag");
    storage_id_ = declare_parameter<std::string>("storage_id", "sqlite3");
    skip_zero_header_stamp_ = declare_parameter<bool>("skip_zero_header_stamp", true);
    target_duration_sec_ = declare_parameter<double>("record_duration_sec", 0.0);
    image_qos_depth_ = declare_parameter<int>("image_qos_depth", 120);
    imu_qos_depth_ = declare_parameter<int>("imu_qos_depth", 2000);
    status_qos_depth_ = declare_parameter<int>("status_qos_depth", 100);
    const bool start_recording = declare_parameter<bool>("start_recording", false);
    const double status_period_sec = declare_parameter<double>("status_period_sec", 5.0);

    data_callback_group_ = create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive);
    control_callback_group_ = create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive);
    status_callback_group_ = create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive);

    rclcpp::SubscriptionOptions data_subscription_options;
    data_subscription_options.callback_group = data_callback_group_;

    image_sub_ = create_subscription<sensor_msgs::msg::Image>(
      image_topic_, sensorQos(image_qos_depth_),
      [this](sensor_msgs::msg::Image::ConstSharedPtr msg) { imageCallback(msg); },
      data_subscription_options);

    imu_sub_ = create_subscription<sensor_msgs::msg::Imu>(
      imu_topic_, sensorQos(imu_qos_depth_),
      [this](sensor_msgs::msg::Imu::ConstSharedPtr msg) { imuCallback(msg); },
      data_subscription_options);

    camera_info_sub_ = create_subscription<sensor_msgs::msg::CameraInfo>(
      camera_info_topic_,
      rclcpp::QoS(rclcpp::KeepLast(static_cast<size_t>(std::max(10, status_qos_depth_)))),
      [this](sensor_msgs::msg::CameraInfo::ConstSharedPtr msg) { cameraInfoCallback(msg); },
      data_subscription_options);

    cube_pose_sub_ = create_subscription<cube_imu_calibration::msg::CubePose>(
      cube_pose_topic_,
      rclcpp::QoS(rclcpp::KeepLast(static_cast<size_t>(std::max(10, status_qos_depth_)))),
      [this](cube_imu_calibration::msg::CubePose::ConstSharedPtr msg) { cubePoseCallback(msg); },
      data_subscription_options);

    cube_pose_status_sub_ = create_subscription<cube_imu_calibration::msg::CubePoseStatus>(
      cube_pose_status_topic_,
      rclcpp::QoS(rclcpp::KeepLast(static_cast<size_t>(std::max(10, status_qos_depth_)))),
      [this](cube_imu_calibration::msg::CubePoseStatus::ConstSharedPtr msg) {
        cubePoseStatusCallback(msg);
      },
      data_subscription_options);

    status_pub_ = create_publisher<cube_imu_calibration::msg::RecorderStatus>(
      "~/status", rclcpp::QoS(10).transient_local());

    start_service_ = create_service<std_srvs::srv::Trigger>(
      "~/start_recording",
      [this](
        const std::shared_ptr<std_srvs::srv::Trigger::Request> request,
        std::shared_ptr<std_srvs::srv::Trigger::Response> response) {
        (void)request;
        startRecording(*response);
      },
      rclcpp::ServicesQoS(),
      control_callback_group_);

    stop_service_ = create_service<std_srvs::srv::Trigger>(
      "~/stop_recording",
      [this](
        const std::shared_ptr<std_srvs::srv::Trigger::Request> request,
        std::shared_ptr<std_srvs::srv::Trigger::Response> response) {
        (void)request;
        stopRecording(*response);
      },
      rclcpp::ServicesQoS(),
      control_callback_group_);

    const auto period = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::duration<double>(status_period_sec > 0.0 ? status_period_sec : 5.0));
    status_timer_ = create_wall_timer(period, [this]() { logStatus(); }, status_callback_group_);

    RCLCPP_INFO(get_logger(), "Subscribed image topic: %s", image_topic_.c_str());
    RCLCPP_INFO(get_logger(), "Subscribed IMU topic: %s", imu_topic_.c_str());
    RCLCPP_INFO(get_logger(), "Subscribed CameraInfo topic: %s", camera_info_topic_.c_str());
    RCLCPP_INFO(get_logger(), "Subscribed CubePose topic: %s", cube_pose_topic_.c_str());
    RCLCPP_INFO(
      get_logger(), "Subscribed CubePoseStatus topic: %s", cube_pose_status_topic_.c_str());
    RCLCPP_INFO(get_logger(), "Bag base path: %s", bag_path_.c_str());
    RCLCPP_INFO(
      get_logger(), "Recorder QoS depths: image=%d, imu=%d, status=%d",
      image_qos_depth_, imu_qos_depth_, status_qos_depth_);
    if (target_duration_sec_ > 0.0) {
      RCLCPP_INFO(
        get_logger(), "Recording target duration: %.3f seconds", target_duration_sec_);
    } else {
      RCLCPP_INFO(get_logger(), "Recording target duration: manual stop");
    }
    RCLCPP_INFO(
      get_logger(),
      "Manual control: ros2 service call /%s/start_recording std_srvs/srv/Trigger {}",
      get_name());
    RCLCPP_INFO(
      get_logger(),
      "Manual control: ros2 service call /%s/stop_recording std_srvs/srv/Trigger {}",
      get_name());

    if (start_recording) {
      std_srvs::srv::Trigger::Response response;
      startRecording(response);
      if (!response.success) {
        RCLCPP_ERROR(get_logger(), "Auto-start recording failed: %s", response.message.c_str());
      }
    }
  }

  ~DataRecorderNode() override
  {
    std::lock_guard<std::mutex> lock(writer_mutex_);
    closeWriterLocked();
  }

private:
  static rclcpp::QoS sensorQos(int depth)
  {
    auto qos = rclcpp::QoS(rclcpp::KeepLast(static_cast<size_t>(std::max(10, depth))));
    qos.best_effort();
    qos.durability_volatile();
    return qos;
  }

  static bool isZeroStamp(const builtin_interfaces::msg::Time & stamp)
  {
    return stamp.sec == 0 && stamp.nanosec == 0;
  }

  static std::string makeTimestampSuffix()
  {
    const auto now = std::chrono::system_clock::now();
    const std::time_t time = std::chrono::system_clock::to_time_t(now);
    std::tm local_time {};
    localtime_r(&time, &local_time);
    std::ostringstream oss;
    oss << std::put_time(&local_time, "%Y%m%d_%H%M%S");
    return oss.str();
  }

  std::filesystem::path resolveBagPath() const
  {
    std::filesystem::path resolved(bag_path_);
    if (resolved.is_relative()) {
      resolved = std::filesystem::absolute(resolved);
    }

    if (std::filesystem::exists(resolved)) {
      resolved += "_" + makeTimestampSuffix();
    }

    const auto parent = resolved.parent_path();
    if (!parent.empty() && !std::filesystem::exists(parent)) {
      std::filesystem::create_directories(parent);
    }
    return resolved;
  }

  static rosbag2_storage::TopicMetadata topicMetadata(
    const std::string & topic_name, const std::string & type_name)
  {
    rosbag2_storage::TopicMetadata metadata;
    metadata.name = topic_name;
    metadata.type = type_name;
    metadata.serialization_format = "cdr";
    return metadata;
  }

  void startRecording(std_srvs::srv::Trigger::Response & response)
  {
    std::lock_guard<std::mutex> lock(writer_mutex_);
    if (recording_) {
      response.success = false;
      response.message = "Recorder is already running at " + active_bag_path_;
      return;
    }

    try {
      const std::filesystem::path resolved_path = resolveBagPath();
      rosbag2_storage::StorageOptions storage_options;
      storage_options.uri = resolved_path.string();
      storage_options.storage_id = storage_id_;

      auto writer = std::make_unique<rosbag2_cpp::Writer>();
      writer->open(storage_options);
      writer->create_topic(topicMetadata(image_topic_, "sensor_msgs/msg/Image"));
      writer->create_topic(topicMetadata(imu_topic_, "sensor_msgs/msg/Imu"));
      writer->create_topic(topicMetadata(camera_info_topic_, "sensor_msgs/msg/CameraInfo"));
      writer->create_topic(topicMetadata(cube_pose_topic_, "cube_imu_calibration/msg/CubePose"));
      writer->create_topic(
        topicMetadata(cube_pose_status_topic_, "cube_imu_calibration/msg/CubePoseStatus"));

      writer_ = std::move(writer);
      active_bag_path_ = resolved_path.string();
      image_count_ = 0;
      imu_count_ = 0;
      camera_info_count_ = 0;
      cube_pose_count_ = 0;
      cube_pose_status_count_ = 0;
      last_image_stamp_ns_ = -1;
      last_imu_stamp_ns_ = -1;
      last_camera_info_stamp_ns_ = -1;
      last_cube_pose_stamp_ns_ = -1;
      last_cube_pose_status_stamp_ns_ = -1;
      record_start_wall_ = std::chrono::steady_clock::now();
      last_elapsed_sec_ = 0.0;
      recording_ = true;

      response.success = true;
      response.message = "Recording started: " + active_bag_path_;
      RCLCPP_INFO(get_logger(), "%s", response.message.c_str());
    } catch (const std::exception & e) {
      closeWriterLocked();
      response.success = false;
      response.message = std::string("Failed to open rosbag2 writer: ") + e.what();
      RCLCPP_ERROR(get_logger(), "%s", response.message.c_str());
    }
  }

  void stopRecording(std_srvs::srv::Trigger::Response & response)
  {
    std::lock_guard<std::mutex> lock(writer_mutex_);
    if (!recording_) {
      response.success = false;
      response.message = "Recorder is not running";
      return;
    }

    try {
      const std::string closed_path = active_bag_path_;
      closeWriterLocked();
      response.success = true;
      response.message = "Recording stopped: " + closed_path;
      RCLCPP_INFO(
        get_logger(),
        "%s; image messages=%lu, imu messages=%lu, camera_info messages=%lu, cube_pose messages=%lu, cube_pose_status messages=%lu",
        response.message.c_str(), image_count_, imu_count_, camera_info_count_, cube_pose_count_,
        cube_pose_status_count_);
    } catch (const std::exception & e) {
      response.success = false;
      response.message = std::string("Failed to close rosbag2 writer cleanly: ") + e.what();
      RCLCPP_ERROR(get_logger(), "%s", response.message.c_str());
    }
  }

  void closeWriterLocked()
  {
    if (recording_) {
      last_elapsed_sec_ = elapsedSecondsLocked();
    }
    if (writer_) {
      writer_->close();
      writer_.reset();
    }
    recording_ = false;
  }

  double elapsedSecondsLocked() const
  {
    if (!recording_) {
      return last_elapsed_sec_;
    }
    return std::chrono::duration<double>(
      std::chrono::steady_clock::now() - record_start_wall_).count();
  }

  bool stopIfDurationReachedLocked()
  {
    if (!recording_ || target_duration_sec_ <= 0.0 || elapsedSecondsLocked() < target_duration_sec_) {
      return false;
    }

    const std::string closed_path = active_bag_path_;
    closeWriterLocked();
    RCLCPP_INFO(
      get_logger(),
      "Recording target duration reached; stopped '%s'; image messages=%lu, imu messages=%lu, camera_info messages=%lu, cube_pose messages=%lu, cube_pose_status messages=%lu",
      closed_path.c_str(), image_count_, imu_count_, camera_info_count_, cube_pose_count_,
      cube_pose_status_count_);
    return true;
  }

  bool prepareStamp(
    const builtin_interfaces::msg::Time & header_stamp,
    const std::string & topic_name,
    rclcpp::Time & stamp) const
  {
    if (isZeroStamp(header_stamp)) {
      if (skip_zero_header_stamp_) {
        RCLCPP_WARN_THROTTLE(
          get_logger(), *get_clock(), 5000,
          "Skipping %s because header.stamp is zero", topic_name.c_str());
        return false;
      }
      RCLCPP_WARN_THROTTLE(
        get_logger(), *get_clock(), 5000,
        "%s has zero header.stamp; writing with node clock time", topic_name.c_str());
      stamp = now();
      return true;
    }

    stamp = rclcpp::Time(header_stamp, RCL_ROS_TIME);
    return true;
  }

  template<typename MessageT>
  void writeMessage(
    const MessageT & msg,
    const std::string & topic_name,
    int64_t & last_stamp_ns,
    uint64_t & count)
  {
    rclcpp::Time stamp;
    if (!prepareStamp(msg.header.stamp, topic_name, stamp)) {
      return;
    }

    std::lock_guard<std::mutex> lock(writer_mutex_);
    if (!recording_ || !writer_) {
      return;
    }
    if (stopIfDurationReachedLocked()) {
      return;
    }

    const int64_t stamp_ns = stamp.nanoseconds();
    if (last_stamp_ns >= 0 && stamp_ns < last_stamp_ns) {
      RCLCPP_WARN_THROTTLE(
        get_logger(), *get_clock(), 5000,
        "Non-monotonic header.stamp on %s; keeping original stamp for calibration fidelity",
        topic_name.c_str());
    }
    last_stamp_ns = stamp_ns;

    try {
      writer_->write(msg, topic_name, stamp);
      ++count;
    } catch (const std::exception & e) {
      RCLCPP_ERROR(
        get_logger(), "rosbag2 write failed on %s: %s. Closing current bag.",
        topic_name.c_str(), e.what());
      closeWriterLocked();
    }
  }

  void imageCallback(const sensor_msgs::msg::Image::ConstSharedPtr msg)
  {
    writeMessage(*msg, image_topic_, last_image_stamp_ns_, image_count_);
  }

  void imuCallback(const sensor_msgs::msg::Imu::ConstSharedPtr msg)
  {
    writeMessage(*msg, imu_topic_, last_imu_stamp_ns_, imu_count_);
  }

  void cameraInfoCallback(const sensor_msgs::msg::CameraInfo::ConstSharedPtr msg)
  {
    writeMessage(*msg, camera_info_topic_, last_camera_info_stamp_ns_, camera_info_count_);
  }

  void cubePoseCallback(const cube_imu_calibration::msg::CubePose::ConstSharedPtr msg)
  {
    writeMessage(*msg, cube_pose_topic_, last_cube_pose_stamp_ns_, cube_pose_count_);
  }

  void cubePoseStatusCallback(const cube_imu_calibration::msg::CubePoseStatus::ConstSharedPtr msg)
  {
    writeMessage(
      *msg, cube_pose_status_topic_, last_cube_pose_status_stamp_ns_,
      cube_pose_status_count_);
  }

  void logStatus()
  {
    const size_t image_publishers = count_publishers(image_topic_);
    const size_t imu_publishers = count_publishers(imu_topic_);
    const size_t camera_info_publishers = count_publishers(camera_info_topic_);
    const size_t cube_pose_publishers = count_publishers(cube_pose_topic_);
    const size_t cube_pose_status_publishers = count_publishers(cube_pose_status_topic_);

    std::lock_guard<std::mutex> lock(writer_mutex_);
    stopIfDurationReachedLocked();

    const double elapsed_sec = elapsedSecondsLocked();
    double progress_ratio = 0.0;
    if (target_duration_sec_ > 0.0) {
      progress_ratio = std::clamp(elapsed_sec / target_duration_sec_, 0.0, 1.0);
    }

    cube_imu_calibration::msg::RecorderStatus status_msg;
    status_msg.header.stamp = now();
    status_msg.recording = recording_;
    status_msg.bag_path = active_bag_path_.empty() ? bag_path_ : active_bag_path_;
    status_msg.image_count = image_count_;
    status_msg.imu_count = imu_count_;
    status_msg.camera_info_count = camera_info_count_;
    status_msg.cube_pose_count = cube_pose_count_;
    status_msg.cube_pose_status_count = cube_pose_status_count_;
    status_msg.image_publishers = static_cast<uint32_t>(image_publishers);
    status_msg.imu_publishers = static_cast<uint32_t>(imu_publishers);
    status_msg.camera_info_publishers = static_cast<uint32_t>(camera_info_publishers);
    status_msg.cube_pose_publishers = static_cast<uint32_t>(cube_pose_publishers);
    status_msg.cube_pose_status_publishers =
      static_cast<uint32_t>(cube_pose_status_publishers);
    status_msg.elapsed_sec = elapsed_sec;
    status_msg.target_duration_sec = target_duration_sec_;
    status_msg.progress_ratio = progress_ratio;
    status_pub_->publish(status_msg);

    if (image_publishers == 0) {
      RCLCPP_WARN(get_logger(), "No publisher currently discovered for %s", image_topic_.c_str());
    }
    if (imu_publishers == 0) {
      RCLCPP_WARN(get_logger(), "No publisher currently discovered for %s", imu_topic_.c_str());
    }
    if (camera_info_publishers == 0) {
      RCLCPP_WARN(
        get_logger(), "No publisher currently discovered for %s", camera_info_topic_.c_str());
    }
    if (cube_pose_publishers == 0) {
      RCLCPP_WARN(
        get_logger(), "No publisher currently discovered for %s", cube_pose_topic_.c_str());
    }
    if (cube_pose_status_publishers == 0) {
      RCLCPP_WARN(
        get_logger(), "No publisher currently discovered for %s",
        cube_pose_status_topic_.c_str());
    }

    RCLCPP_INFO(
      get_logger(),
      "Recorder status: %s, bag='%s', image_pub=%zu, imu_pub=%zu, camera_info_pub=%zu, cube_pose_pub=%zu, cube_pose_status_pub=%zu, image_msgs=%lu, imu_msgs=%lu, camera_info_msgs=%lu, cube_pose_msgs=%lu, cube_pose_status_msgs=%lu, elapsed=%.1fs",
      recording_ ? "recording" : "idle",
      active_bag_path_.empty() ? bag_path_.c_str() : active_bag_path_.c_str(),
      image_publishers, imu_publishers, camera_info_publishers, cube_pose_publishers,
      cube_pose_status_publishers, image_count_, imu_count_, camera_info_count_,
      cube_pose_count_, cube_pose_status_count_, elapsed_sec);
  }

  std::string image_topic_;
  std::string imu_topic_;
  std::string camera_info_topic_;
  std::string cube_pose_topic_;
  std::string cube_pose_status_topic_;
  std::string bag_path_;
  std::string storage_id_;
  bool skip_zero_header_stamp_ {true};
  double target_duration_sec_ {0.0};
  int image_qos_depth_ {120};
  int imu_qos_depth_ {2000};
  int status_qos_depth_ {100};

  rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr image_sub_;
  rclcpp::Subscription<sensor_msgs::msg::Imu>::SharedPtr imu_sub_;
  rclcpp::Subscription<sensor_msgs::msg::CameraInfo>::SharedPtr camera_info_sub_;
  rclcpp::Subscription<cube_imu_calibration::msg::CubePose>::SharedPtr cube_pose_sub_;
  rclcpp::Subscription<cube_imu_calibration::msg::CubePoseStatus>::SharedPtr
    cube_pose_status_sub_;
  rclcpp::Publisher<cube_imu_calibration::msg::RecorderStatus>::SharedPtr status_pub_;
  rclcpp::Service<std_srvs::srv::Trigger>::SharedPtr start_service_;
  rclcpp::Service<std_srvs::srv::Trigger>::SharedPtr stop_service_;
  rclcpp::TimerBase::SharedPtr status_timer_;
  rclcpp::CallbackGroup::SharedPtr data_callback_group_;
  rclcpp::CallbackGroup::SharedPtr control_callback_group_;
  rclcpp::CallbackGroup::SharedPtr status_callback_group_;

  mutable std::mutex writer_mutex_;
  std::unique_ptr<rosbag2_cpp::Writer> writer_;
  bool recording_ {false};
  std::string active_bag_path_;
  std::chrono::steady_clock::time_point record_start_wall_;
  double last_elapsed_sec_ {0.0};
  uint64_t image_count_ {0};
  uint64_t imu_count_ {0};
  uint64_t camera_info_count_ {0};
  uint64_t cube_pose_count_ {0};
  uint64_t cube_pose_status_count_ {0};
  int64_t last_image_stamp_ns_ {-1};
  int64_t last_imu_stamp_ns_ {-1};
  int64_t last_camera_info_stamp_ns_ {-1};
  int64_t last_cube_pose_stamp_ns_ {-1};
  int64_t last_cube_pose_status_stamp_ns_ {-1};
};

}  // namespace cube_imu_calibration

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::executors::MultiThreadedExecutor executor(rclcpp::ExecutorOptions(), 3);
  auto node = std::make_shared<cube_imu_calibration::DataRecorderNode>();
  executor.add_node(node);
  executor.spin();
  rclcpp::shutdown();
  return 0;
}
