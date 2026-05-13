#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cctype>
#include <cstring>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>

#include <Eigen/Core>
#include <cv_bridge/cv_bridge.hpp>
#include <libobsensor/ObSensor.hpp>
#include <opencv2/imgproc.hpp>

#include "builtin_interfaces/msg/time.hpp"
#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/image_encodings.hpp"
#include "sensor_msgs/msg/camera_info.hpp"
#include "sensor_msgs/msg/image.hpp"
#include "sensor_msgs/msg/imu.hpp"

#include "serial_imu_reader.hpp"

namespace cube_imu_calibration
{

namespace
{

std::string formatName(OBFormat format)
{
  switch (format) {
    case OB_FORMAT_MJPG:
      return "MJPG";
    case OB_FORMAT_YUYV:
      return "YUYV";
    case OB_FORMAT_YUY2:
      return "YUY2";
    case OB_FORMAT_UYVY:
      return "UYVY";
    case OB_FORMAT_NV12:
      return "NV12";
    case OB_FORMAT_NV21:
      return "NV21";
    case OB_FORMAT_RGB:
      return "RGB";
    case OB_FORMAT_BGR:
      return "BGR";
    case OB_FORMAT_BGRA:
      return "BGRA";
    case OB_FORMAT_RGBA:
      return "RGBA";
    default:
      return std::to_string(static_cast<int>(format));
  }
}

OBConvertFormat convertFormatFor(OBFormat format)
{
  switch (format) {
    case OB_FORMAT_MJPG:
      return FORMAT_MJPG_TO_BGR;
    case OB_FORMAT_YUYV:
    case OB_FORMAT_YUY2:
      return FORMAT_YUYV_TO_BGR;
    case OB_FORMAT_I420:
      return FORMAT_I420_TO_RGB;
    case OB_FORMAT_NV12:
      return FORMAT_NV12_TO_RGB;
    case OB_FORMAT_NV21:
      return FORMAT_NV21_TO_RGB;
    case OB_FORMAT_UYVY:
      return FORMAT_UYVY_TO_RGB;
    case OB_FORMAT_RGB:
      return FORMAT_RGB_TO_BGR;
    default:
      throw std::runtime_error("Unsupported Orbbec color format: " + formatName(format));
  }
}

builtin_interfaces::msg::Time stampFromSystemUs(uint64_t stamp_us, const rclcpp::Time & fallback)
{
  if (stamp_us == 0) {
    return fallback;
  }
  builtin_interfaces::msg::Time stamp;
  stamp.sec = static_cast<int32_t>(stamp_us / 1000000ULL);
  stamp.nanosec = static_cast<uint32_t>((stamp_us % 1000000ULL) * 1000ULL);
  return stamp;
}

std::string normalizeImuSource(std::string source)
{
  std::transform(source.begin(), source.end(), source.begin(), [](unsigned char c) {
    return static_cast<char>(std::tolower(c));
  });
  std::replace(source.begin(), source.end(), '-', '_');
  return source;
}

bool isCubeSerialImuSource(const std::string & source)
{
  return source == "cube_serial" || source == "serial" || source == "external";
}

std::string profileSummary(const std::shared_ptr<ob::VideoStreamProfile> & profile)
{
  if (!profile) {
    return "--";
  }
  std::ostringstream oss;
  oss << profile->getWidth() << "x" << profile->getHeight() << "@" << profile->getFps() << " "
      << formatName(profile->getFormat());
  return oss.str();
}

}  // namespace

class RealSensorNode : public rclcpp::Node
{
public:
  RealSensorNode()
  : Node("real_sensor_node")
  {
    image_topic_ = declare_parameter<std::string>("image_topic", "/camera/image_raw");
    camera_info_topic_ = declare_parameter<std::string>(
      "camera_info_topic", "/camera/camera_info");
    imu_topic_ = declare_parameter<std::string>("imu_topic", "/imu");
    camera_frame_ = declare_parameter<std::string>("camera_frame", "camera_link");
    imu_frame_ = declare_parameter<std::string>("imu_frame", "imu_link");
    output_encoding_ = declare_parameter<std::string>("output_encoding", "mono8");
    std::transform(
      output_encoding_.begin(), output_encoding_.end(), output_encoding_.begin(),
      [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    if (output_encoding_ != sensor_msgs::image_encodings::MONO8 &&
      output_encoding_ != sensor_msgs::image_encodings::BGR8)
    {
      throw std::runtime_error("output_encoding must be mono8 or bgr8");
    }
    imu_source_ = normalizeImuSource(declare_parameter<std::string>("imu_source", "cube_serial"));
    if (!isCubeSerialImuSource(imu_source_)) {
      throw std::runtime_error(
        "real_sensor_node only publishes Cube-mounted serial IMU data on /imu for this "
        "calibration workflow. Orbbec/camera built-in IMU is camera-mounted and must not "
        "be used as the Cube IMU. Set imu_source:=cube_serial and connect /dev/ttyACM0.");
    }

    color_width_ = declare_parameter<int>("color_width", 1280);
    color_height_ = declare_parameter<int>("color_height", 720);
    color_fps_ = declare_parameter<int>("color_fps", 15);
    frame_timeout_ms_ = declare_parameter<int>("frame_timeout_ms", 1000);
    image_qos_depth_ = declare_parameter<int>("image_qos_depth", 30);
    imu_qos_depth_ = declare_parameter<int>("imu_qos_depth", 1000);

    SerialImuConfig imu_config;
    imu_config.enabled = declare_parameter<bool>("serial_imu_enabled", true);
    imu_config.port = declare_parameter<std::string>("serial_port", "/dev/ttyACM0");
    imu_config.baudrate = declare_parameter<int>("serial_baudrate", 2000000);
    imu_config.protocol_mode = declare_parameter<std::string>("serial_protocol_mode", "v2.1");
    imu_config.imu_rate_hz = declare_parameter<double>("serial_imu_rate_hz", 500.0);
    imu_config.startup_command = declare_parameter<std::string>("serial_startup_command", "uartout");
    serial_imu_ = std::make_unique<SerialImuReader>(imu_config);

    image_pub_ = create_publisher<sensor_msgs::msg::Image>(
      image_topic_, sensorQos(image_qos_depth_));
    camera_info_pub_ = create_publisher<sensor_msgs::msg::CameraInfo>(camera_info_topic_, 10);

    startCamera();
    startSerialImu();

    poll_timer_ = create_wall_timer(std::chrono::milliseconds(1), [this]() { pollOnce(); });
  }

  ~RealSensorNode() override
  {
    if (serial_imu_) {
      serial_imu_->stop();
    }
    if (pipeline_) {
      pipeline_->stop();
      pipeline_.reset();
    }
  }

private:
  static rclcpp::QoS sensorQos(int depth)
  {
    auto qos = rclcpp::QoS(rclcpp::KeepLast(static_cast<size_t>(std::max(10, depth))));
    qos.best_effort();
    qos.durability_volatile();
    return qos;
  }

  std::shared_ptr<ob::VideoStreamProfile> selectVideoProfile(
    const std::shared_ptr<ob::Sensor> & sensor, int width, int height, int fps) const
  {
    auto profiles = sensor->getStreamProfileList();
    if (!profiles || profiles->getCount() == 0) {
      throw std::runtime_error("Orbbec color sensor has no stream profiles");
    }

    try {
      return profiles->getVideoStreamProfile(width, height, OB_FORMAT_ANY, fps);
    } catch (const ob::Error &) {
    }
    try {
      return profiles->getVideoStreamProfile(width, height, OB_FORMAT_ANY, OB_FPS_ANY);
    } catch (const ob::Error &) {
    }
    try {
      return profiles->getVideoStreamProfile(OB_WIDTH_ANY, OB_HEIGHT_ANY, OB_FORMAT_ANY, fps);
    } catch (const ob::Error &) {
    }
    for (uint32_t i = 0; i < profiles->getCount(); ++i) {
      auto profile = profiles->getProfile(i);
      if (profile && profile->is<ob::VideoStreamProfile>()) {
        return profile->as<ob::VideoStreamProfile>();
      }
    }
    throw std::runtime_error("No usable Orbbec color stream profile found");
  }

  void startCamera()
  {
    pipeline_ = std::make_shared<ob::Pipeline>();
    auto device = pipeline_->getDevice();
    if (!device) {
      throw std::runtime_error("No Orbbec Mini335/Gemini335 device available");
    }
    auto color_sensor = device->getSensor(OB_SENSOR_COLOR);
    if (!color_sensor) {
      throw std::runtime_error("Connected Orbbec device has no color sensor");
    }

    auto config = std::make_shared<ob::Config>();
    color_profile_ = selectVideoProfile(color_sensor, color_width_, color_height_, color_fps_);
    config->enableStream(color_profile_);
    pipeline_->start(config);

    RCLCPP_INFO(
      get_logger(),
      "real_sensor_node camera started: color=%s, output=%s, image=%s, camera_info=%s, "
      "image_qos_depth=%d",
      profileSummary(color_profile_).c_str(), output_encoding_.c_str(), image_topic_.c_str(),
      camera_info_topic_.c_str(), image_qos_depth_);
  }

  void startSerialImu()
  {
    if (!serial_imu_) {
      return;
    }
    try {
      serial_imu_->start();
      if (serial_imu_->isStarted()) {
        imu_pub_ = create_publisher<sensor_msgs::msg::Imu>(imu_topic_, sensorQos(imu_qos_depth_));
        RCLCPP_INFO(
          get_logger(),
          "real_sensor_node Cube serial IMU started: source=%s, %s -> %s, imu_qos_depth=%d",
          imu_source_.c_str(), serial_imu_->statusLine().c_str(), imu_topic_.c_str(),
          imu_qos_depth_);
      }
    } catch (const std::exception & exc) {
      RCLCPP_WARN(
        get_logger(), "Serial IMU did not start: %s. Camera topics will still be published.",
        exc.what());
    }
  }

  cv::Mat colorFrameToBgr(const std::shared_ptr<ob::ColorFrame> & frame) const
  {
    if (!frame) {
      return {};
    }
    std::shared_ptr<ob::ColorFrame> bgr_frame = frame;
    if (frame->getFormat() != OB_FORMAT_BGR) {
      ob::FormatConvertFilter convert_filter;
      convert_filter.setFormatConvertType(convertFormatFor(frame->getFormat()));
      auto converted = convert_filter.process(frame);
      if (!converted) {
        throw std::runtime_error("Failed to convert Orbbec color frame to BGR");
      }
      bgr_frame = converted->as<ob::ColorFrame>();
    }

    const int width = static_cast<int>(bgr_frame->getWidth());
    const int height = static_cast<int>(bgr_frame->getHeight());
    cv::Mat out(height, width, CV_8UC3);
    std::memcpy(out.data, bgr_frame->getData(), static_cast<std::size_t>(width * height * 3));

    const auto source_format = frame->getFormat();
    if (source_format == OB_FORMAT_I420 || source_format == OB_FORMAT_NV12 ||
      source_format == OB_FORMAT_NV21 || source_format == OB_FORMAT_UYVY)
    {
      cv::cvtColor(out, out, cv::COLOR_RGB2BGR);
    }
    return out;
  }

  sensor_msgs::msg::CameraInfo makeCameraInfo(
    const builtin_interfaces::msg::Time & stamp, int width, int height) const
  {
    sensor_msgs::msg::CameraInfo info;
    info.header.stamp = stamp;
    info.header.frame_id = camera_frame_;
    info.width = static_cast<uint32_t>(width);
    info.height = static_cast<uint32_t>(height);
    info.distortion_model = "plumb_bob";

    try {
      const auto params = pipeline_->getCameraParam();
      const auto & k = params.rgbIntrinsic;
      const auto & d = params.rgbDistortion;
      info.width = static_cast<uint32_t>(k.width);
      info.height = static_cast<uint32_t>(k.height);
      info.d = {d.k1, d.k2, d.p1, d.p2, d.k3};
      info.k = {
        k.fx, 0.0, k.cx,
        0.0, k.fy, k.cy,
        0.0, 0.0, 1.0};
      info.r = {
        1.0, 0.0, 0.0,
        0.0, 1.0, 0.0,
        0.0, 0.0, 1.0};
      info.p = {
        k.fx, 0.0, k.cx, 0.0,
        0.0, k.fy, k.cy, 0.0,
        0.0, 0.0, 1.0, 0.0};
    } catch (const std::exception & exc) {
      RCLCPP_WARN_THROTTLE(
        get_logger(), *get_clock(), 5000, "Using fallback CameraInfo: %s", exc.what());
      const double fx = 600.0;
      const double fy = 600.0;
      const double cx = static_cast<double>(width) * 0.5;
      const double cy = static_cast<double>(height) * 0.5;
      info.d = {0.0, 0.0, 0.0, 0.0, 0.0};
      info.k = {
        fx, 0.0, cx,
        0.0, fy, cy,
        0.0, 0.0, 1.0};
      info.r = {
        1.0, 0.0, 0.0,
        0.0, 1.0, 0.0,
        0.0, 0.0, 1.0};
      info.p = {
        fx, 0.0, cx, 0.0,
        0.0, fy, cy, 0.0,
        0.0, 0.0, 1.0, 0.0};
    }
    return info;
  }

  void publishSerialImu()
  {
    if (!serial_imu_ || !serial_imu_->isStarted() || !imu_pub_) {
      return;
    }

    const auto samples = serial_imu_->drainSamples();
    for (const auto & sample : samples) {
      if (sample.type == ImuSensorType::Accel) {
        latest_accel_ = sample.value;
        latest_accel_valid_ = true;
        latest_accel_stamp_us_ = sample.system_timestamp_us;
        continue;
      }
      if (sample.type != ImuSensorType::Gyro || !latest_accel_valid_) {
        continue;
      }

      sensor_msgs::msg::Imu imu;
      imu.header.stamp = stampFromSystemUs(sample.system_timestamp_us, now());
      imu.header.frame_id = imu_frame_;
      imu.orientation_covariance[0] = -1.0;
      imu.angular_velocity.x = sample.value.x();
      imu.angular_velocity.y = sample.value.y();
      imu.angular_velocity.z = sample.value.z();
      imu.linear_acceleration.x = latest_accel_.x();
      imu.linear_acceleration.y = latest_accel_.y();
      imu.linear_acceleration.z = latest_accel_.z();
      imu.angular_velocity_covariance[0] = 1e-5;
      imu.angular_velocity_covariance[4] = 1e-5;
      imu.angular_velocity_covariance[8] = 1e-5;
      imu.linear_acceleration_covariance[0] = 1e-3;
      imu.linear_acceleration_covariance[4] = 1e-3;
      imu.linear_acceleration_covariance[8] = 1e-3;
      imu_pub_->publish(imu);
    }
  }

  void pollOnce()
  {
    publishSerialImu();

    try {
      auto frame_set = pipeline_->waitForFrameset(static_cast<uint32_t>(frame_timeout_ms_));
      if (!frame_set) {
        return;
      }
      auto color = frame_set->getColorFrame();
      if (!color) {
        return;
      }

      cv::Mat image = colorFrameToBgr(color);
      if (image.empty()) {
        return;
      }
      if (output_encoding_ == sensor_msgs::image_encodings::MONO8) {
        cv::cvtColor(image, image, cv::COLOR_BGR2GRAY);
      }

      const uint64_t stamp_us = color->getSystemTimeStampUs() != 0 ?
        color->getSystemTimeStampUs() : color->getTimeStampUs();
      std_msgs::msg::Header header;
      header.stamp = stampFromSystemUs(stamp_us, now());
      header.frame_id = camera_frame_;

      auto image_msg = cv_bridge::CvImage(header, output_encoding_, image).toImageMsg();
      image_pub_->publish(*image_msg);
      camera_info_pub_->publish(makeCameraInfo(header.stamp, image.cols, image.rows));
    } catch (const std::exception & exc) {
      RCLCPP_WARN_THROTTLE(
        get_logger(), *get_clock(), 2000, "real_sensor_node poll failed: %s", exc.what());
    }
  }

  std::string image_topic_;
  std::string camera_info_topic_;
  std::string imu_topic_;
  std::string camera_frame_;
  std::string imu_frame_;
  std::string output_encoding_ {sensor_msgs::image_encodings::MONO8};
  std::string imu_source_ {"cube_serial"};
  int color_width_ {1280};
  int color_height_ {720};
  int color_fps_ {15};
  int frame_timeout_ms_ {1000};
  int image_qos_depth_ {30};
  int imu_qos_depth_ {1000};

  rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr image_pub_;
  rclcpp::Publisher<sensor_msgs::msg::CameraInfo>::SharedPtr camera_info_pub_;
  rclcpp::Publisher<sensor_msgs::msg::Imu>::SharedPtr imu_pub_;
  rclcpp::TimerBase::SharedPtr poll_timer_;

  std::shared_ptr<ob::Pipeline> pipeline_;
  std::shared_ptr<ob::VideoStreamProfile> color_profile_;
  std::unique_ptr<SerialImuReader> serial_imu_;
  Eigen::Vector3d latest_accel_ {Eigen::Vector3d::Zero()};
  bool latest_accel_valid_ {false};
  uint64_t latest_accel_stamp_us_ {0};
};

}  // namespace cube_imu_calibration

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<cube_imu_calibration::RealSensorNode>());
  rclcpp::shutdown();
  return 0;
}
