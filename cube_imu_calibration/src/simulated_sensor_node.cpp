#include <algorithm>
#include <chrono>
#include <map>
#include <memory>
#include <string>

#include "cv_bridge/cv_bridge.hpp"
#include "opencv2/aruco.hpp"
#include "opencv2/core.hpp"
#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/image_encodings.hpp"
#include "sensor_msgs/msg/camera_info.hpp"
#include "sensor_msgs/msg/image.hpp"
#include "sensor_msgs/msg/imu.hpp"

namespace cube_imu_calibration
{

class SimulatedSensorNode : public rclcpp::Node
{
public:
  SimulatedSensorNode()
  : Node("simulated_sensor_node")
  {
    image_topic_ = declare_parameter<std::string>("image_topic", "/head_camera/image_raw");
    camera_info_topic_ = declare_parameter<std::string>(
      "camera_info_topic", "/head_camera/camera_info");
    imu_topic_ = declare_parameter<std::string>("imu_topic", "/hand_imu/data");
    camera_frame_ = declare_parameter<std::string>("camera_frame", "head_camera_link");
    dictionary_name_ = declare_parameter<std::string>("dictionary", "DICT_APRILTAG_36h11");
    tag_id_ = declare_parameter<int>("tag_id", 0);
    width_ = declare_parameter<int>("width", 640);
    height_ = declare_parameter<int>("height", 480);
    marker_pixels_ = declare_parameter<int>("marker_pixels", 160);
    fps_ = declare_parameter<double>("fps", 10.0);
    imu_rate_hz_ = declare_parameter<double>("imu_rate_hz", 100.0);
    fx_ = declare_parameter<double>("fx", 600.0);
    fy_ = declare_parameter<double>("fy", 600.0);
    cx_ = declare_parameter<double>("cx", static_cast<double>(width_) * 0.5);
    cy_ = declare_parameter<double>("cy", static_cast<double>(height_) * 0.5);

    width_ = std::max(width_, 64);
    height_ = std::max(height_, 64);
    marker_pixels_ = std::clamp(marker_pixels_, 20, std::min(width_, height_) - 20);
    fps_ = fps_ > 0.0 ? fps_ : 10.0;
    imu_rate_hz_ = imu_rate_hz_ > 0.0 ? imu_rate_hz_ : 100.0;

    configureDictionary();

    image_pub_ = create_publisher<sensor_msgs::msg::Image>(image_topic_, rclcpp::SensorDataQoS());
    camera_info_pub_ = create_publisher<sensor_msgs::msg::CameraInfo>(camera_info_topic_, 10);
    imu_pub_ = create_publisher<sensor_msgs::msg::Imu>(imu_topic_, rclcpp::SensorDataQoS());

    image_timer_ = create_wall_timer(
      std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::duration<double>(1.0 / fps_)),
      [this]() { publishImageAndCameraInfo(); });
    imu_timer_ = create_wall_timer(
      std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::duration<double>(1.0 / imu_rate_hz_)),
      [this]() { publishImu(); });

    RCLCPP_INFO(
      get_logger(),
      "Publishing simulated image=%s, camera_info=%s, imu=%s, AprilTag id=%d",
      image_topic_.c_str(), camera_info_topic_.c_str(), imu_topic_.c_str(), tag_id_);
  }

private:
  void configureDictionary()
  {
    const std::map<std::string, cv::aruco::PREDEFINED_DICTIONARY_NAME> dictionaries = {
      {"DICT_APRILTAG_16h5", cv::aruco::DICT_APRILTAG_16h5},
      {"DICT_APRILTAG_25h9", cv::aruco::DICT_APRILTAG_25h9},
      {"DICT_APRILTAG_36h10", cv::aruco::DICT_APRILTAG_36h10},
      {"DICT_APRILTAG_36h11", cv::aruco::DICT_APRILTAG_36h11},
    };

    auto found = dictionaries.find(dictionary_name_);
    if (found == dictionaries.end()) {
      RCLCPP_WARN(
        get_logger(), "Unknown dictionary '%s'; using DICT_APRILTAG_36h11",
        dictionary_name_.c_str());
      found = dictionaries.find("DICT_APRILTAG_36h11");
    }
    dictionary_ = cv::aruco::getPredefinedDictionary(found->second);
  }

  sensor_msgs::msg::CameraInfo makeCameraInfo(const rclcpp::Time & stamp) const
  {
    sensor_msgs::msg::CameraInfo info;
    info.header.stamp = stamp;
    info.header.frame_id = camera_frame_;
    info.width = static_cast<uint32_t>(width_);
    info.height = static_cast<uint32_t>(height_);
    info.distortion_model = "plumb_bob";
    info.d = {0.0, 0.0, 0.0, 0.0, 0.0};
    info.k = {
      fx_, 0.0, cx_,
      0.0, fy_, cy_,
      0.0, 0.0, 1.0};
    info.r = {
      1.0, 0.0, 0.0,
      0.0, 1.0, 0.0,
      0.0, 0.0, 1.0};
    info.p = {
      fx_, 0.0, cx_, 0.0,
      0.0, fy_, cy_, 0.0,
      0.0, 0.0, 1.0, 0.0};
    return info;
  }

  void publishImageAndCameraInfo()
  {
    cv::Mat image(height_, width_, CV_8UC1, cv::Scalar(255));
    cv::Mat marker;
    cv::aruco::drawMarker(dictionary_, tag_id_, marker_pixels_, marker, 1);

    const int x0 = (width_ - marker_pixels_) / 2;
    const int y0 = (height_ - marker_pixels_) / 2;
    marker.copyTo(image(cv::Rect(x0, y0, marker_pixels_, marker_pixels_)));

    const rclcpp::Time stamp = now();
    std_msgs::msg::Header header;
    header.stamp = stamp;
    header.frame_id = camera_frame_;

    sensor_msgs::msg::Image::SharedPtr image_msg =
      cv_bridge::CvImage(header, sensor_msgs::image_encodings::MONO8, image).toImageMsg();
    image_pub_->publish(*image_msg);
    camera_info_pub_->publish(makeCameraInfo(stamp));
  }

  void publishImu()
  {
    sensor_msgs::msg::Imu imu;
    imu.header.stamp = now();
    imu.header.frame_id = "imu_link";

    // Simulated raw IMU: no orientation estimate, stationary angular velocity,
    // gravity magnitude on +Z for pipeline smoke testing.
    imu.orientation_covariance[0] = -1.0;
    imu.angular_velocity.x = 0.0;
    imu.angular_velocity.y = 0.0;
    imu.angular_velocity.z = 0.0;
    imu.linear_acceleration.x = 0.0;
    imu.linear_acceleration.y = 0.0;
    imu.linear_acceleration.z = 9.80665;

    imu.angular_velocity_covariance[0] = 1e-6;
    imu.angular_velocity_covariance[4] = 1e-6;
    imu.angular_velocity_covariance[8] = 1e-6;
    imu.linear_acceleration_covariance[0] = 1e-4;
    imu.linear_acceleration_covariance[4] = 1e-4;
    imu.linear_acceleration_covariance[8] = 1e-4;
    imu_pub_->publish(imu);
  }

  std::string image_topic_;
  std::string camera_info_topic_;
  std::string imu_topic_;
  std::string camera_frame_;
  std::string dictionary_name_;
  int tag_id_ {0};
  int width_ {640};
  int height_ {480};
  int marker_pixels_ {160};
  double fps_ {10.0};
  double imu_rate_hz_ {100.0};
  double fx_ {600.0};
  double fy_ {600.0};
  double cx_ {320.0};
  double cy_ {240.0};

  cv::Ptr<cv::aruco::Dictionary> dictionary_;
  rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr image_pub_;
  rclcpp::Publisher<sensor_msgs::msg::CameraInfo>::SharedPtr camera_info_pub_;
  rclcpp::Publisher<sensor_msgs::msg::Imu>::SharedPtr imu_pub_;
  rclcpp::TimerBase::SharedPtr image_timer_;
  rclcpp::TimerBase::SharedPtr imu_timer_;
};

}  // namespace cube_imu_calibration

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<cube_imu_calibration::SimulatedSensorNode>());
  rclcpp::shutdown();
  return 0;
}
