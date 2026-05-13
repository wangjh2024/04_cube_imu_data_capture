#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <deque>
#include <memory>
#include <mutex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include <Eigen/Core>
#include <libobsensor/ObSensor.hpp>
#include <opencv2/imgproc.hpp>

#include "serial_imu_reader.hpp"

extern "C" {

struct DirectSensorConfigC
{
  int color_width;
  int color_height;
  int color_fps;
  int frame_timeout_ms;
  int output_encoding;  // 0=mono8, 1=bgr8
  int serial_enabled;
  const char * serial_port;
  int serial_baudrate;
  const char * serial_protocol_mode;
  double serial_imu_rate_hz;
  const char * serial_startup_command;
};

struct DirectImageMetaC
{
  uint64_t seq;
  double stamp_s;
  uint64_t stamp_us;
  int width;
  int height;
  int channels;
  int step;
  int encoding;  // 0=mono8, 1=bgr8
  int camera_info_valid;
  double K[9];
  double D[5];
  double capture_rate_hz;
};

struct DirectImuSampleC
{
  uint64_t index;
  double stamp_s;
  double ax;
  double ay;
  double az;
  double gx;
  double gy;
  double gz;
};

struct DirectSensorStatsC
{
  int started;
  int camera_online;
  int imu_online;
  int actual_color_width;
  int actual_color_height;
  int actual_color_fps;
  uint64_t image_count;
  uint64_t imu_count;
  uint64_t dropped_imu_count;
  double image_rate_hz;
  double imu_rate_hz;
};

}  // extern "C"

namespace cube_imu_calibration
{
namespace
{

constexpr std::size_t kMaxQueuedImuPackets = 20000;
constexpr double kStatusWindowSec = 3.0;

double nowSteadySeconds()
{
  using clock = std::chrono::steady_clock;
  return std::chrono::duration<double>(clock::now().time_since_epoch()).count();
}

std::string safeString(const char * value, const std::string & fallback = {})
{
  if (value == nullptr || std::strlen(value) == 0) {
    return fallback;
  }
  return std::string(value);
}

double rateFromTimes(const std::deque<double> & times)
{
  if (times.size() < 2) {
    return 0.0;
  }
  const double span = times.back() - times.front();
  if (span <= 1e-9) {
    return 0.0;
  }
  return static_cast<double>(times.size() - 1) / span;
}

void trimTimes(std::deque<double> & times, double now)
{
  while (!times.empty() && now - times.front() > kStatusWindowSec) {
    times.pop_front();
  }
}

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

uint64_t stampFromFrameUs(const std::shared_ptr<ob::ColorFrame> & frame)
{
  if (!frame) {
    return 0;
  }
  const uint64_t system_stamp = frame->getSystemTimeStampUs();
  return system_stamp != 0 ? system_stamp : frame->getTimeStampUs();
}

class DirectSensorRuntime
{
public:
  DirectSensorRuntime() = default;
  ~DirectSensorRuntime()
  {
    stop();
  }

  int start(const DirectSensorConfigC & config)
  {
    std::lock_guard<std::mutex> lock(lifecycle_mutex_);
    if (started_.load()) {
      return 1;
    }
    config_ = config;
    output_encoding_ = config.output_encoding == 1 ? 1 : 0;
    frame_timeout_ms_ = std::max(1, config.frame_timeout_ms);
    expected_imu_rate_hz_ = config.serial_imu_rate_hz > 0.0 ? config.serial_imu_rate_hz : 500.0;

    try {
      startCamera(config);
      startSerialImu(config);
      stop_requested_.store(false);
      started_.store(true);
      camera_thread_ = std::thread(&DirectSensorRuntime::cameraLoop, this);
      imu_thread_ = std::thread(&DirectSensorRuntime::imuLoop, this);
      setStatus("direct sensor started: " + cameraStatusLine());
      return 1;
    } catch (const std::exception & exc) {
      stopUnlocked();
      setStatus(std::string("direct sensor start failed: ") + exc.what());
      return 0;
    }
  }

  void stop()
  {
    std::lock_guard<std::mutex> lock(lifecycle_mutex_);
    stopUnlocked();
  }

  bool isStarted() const
  {
    return started_.load();
  }

  std::size_t copyLatestImage(DirectImageMetaC * meta, uint8_t * dst, std::size_t capacity) const
  {
    std::lock_guard<std::mutex> lock(image_mutex_);
    if (meta != nullptr) {
      *meta = image_meta_;
    }
    const std::size_t required = image_data_.size();
    if (dst == nullptr || capacity < required || required == 0) {
      return required;
    }
    std::memcpy(dst, image_data_.data(), required);
    return required;
  }

  int drainImu(DirectImuSampleC * dst, int max_samples)
  {
    if (dst == nullptr || max_samples <= 0) {
      return 0;
    }
    std::lock_guard<std::mutex> lock(imu_mutex_);
    const int count = std::min<int>(max_samples, static_cast<int>(imu_packets_.size()));
    for (int i = 0; i < count; ++i) {
      dst[i] = imu_packets_.front();
      imu_packets_.pop_front();
    }
    return count;
  }

  DirectSensorStatsC stats() const
  {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    DirectSensorStatsC out {};
    out.started = started_.load() ? 1 : 0;
    out.camera_online = camera_online_ ? 1 : 0;
    out.imu_online = imu_online_ ? 1 : 0;
    out.actual_color_width = actual_color_width_;
    out.actual_color_height = actual_color_height_;
    out.actual_color_fps = actual_color_fps_;
    out.image_count = image_count_;
    out.imu_count = imu_count_;
    out.dropped_imu_count = dropped_imu_count_;
    out.image_rate_hz = rateFromTimes(image_times_);
    out.imu_rate_hz = rateFromTimes(imu_times_);
    return out;
  }

  std::string statusLine() const
  {
    std::lock_guard<std::mutex> lock(status_mutex_);
    return status_line_;
  }

private:
  void stopUnlocked()
  {
    stop_requested_.store(true);
    if (camera_thread_.joinable()) {
      camera_thread_.join();
    }
    if (imu_thread_.joinable()) {
      imu_thread_.join();
    }
    if (serial_imu_) {
      serial_imu_->stop();
      serial_imu_.reset();
    }
    if (pipeline_) {
      try {
        pipeline_->stop();
      } catch (...) {
      }
      pipeline_.reset();
    }
    color_profile_.reset();
    started_.store(false);
    {
      std::lock_guard<std::mutex> lock(stats_mutex_);
      camera_online_ = false;
      imu_online_ = false;
      actual_color_width_ = 0;
      actual_color_height_ = 0;
      actual_color_fps_ = 0;
      image_times_.clear();
      imu_times_.clear();
    }
    {
      std::lock_guard<std::mutex> lock(imu_mutex_);
      imu_packets_.clear();
    }
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

  void startCamera(const DirectSensorConfigC & config)
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

    auto ob_config = std::make_shared<ob::Config>();
    color_profile_ = selectVideoProfile(
      color_sensor, std::max(1, config.color_width), std::max(1, config.color_height),
      std::max(1, config.color_fps));
    ob_config->enableStream(color_profile_);
    pipeline_->start(ob_config);

    {
      std::lock_guard<std::mutex> lock(stats_mutex_);
      camera_online_ = true;
      actual_color_width_ = static_cast<int>(color_profile_->getWidth());
      actual_color_height_ = static_cast<int>(color_profile_->getHeight());
      actual_color_fps_ = static_cast<int>(color_profile_->getFps());
    }
    updateCameraInfo();
  }

  void startSerialImu(const DirectSensorConfigC & config)
  {
    SerialImuConfig imu_config;
    imu_config.enabled = config.serial_enabled != 0;
    imu_config.port = safeString(config.serial_port, "/dev/ttyACM0");
    imu_config.baudrate = config.serial_baudrate > 0 ? config.serial_baudrate : 2000000;
    imu_config.protocol_mode = safeString(config.serial_protocol_mode, "v2.1");
    imu_config.imu_rate_hz = config.serial_imu_rate_hz > 0.0 ? config.serial_imu_rate_hz : 500.0;
    imu_config.startup_command = safeString(config.serial_startup_command, "uartout");
    serial_imu_ = std::make_unique<SerialImuReader>(imu_config);
    try {
      serial_imu_->start();
    } catch (const std::exception & exc) {
      setStatus(std::string("serial IMU start failed: ") + exc.what());
    }
    {
      std::lock_guard<std::mutex> lock(stats_mutex_);
      imu_online_ = serial_imu_ && serial_imu_->isStarted();
    }
  }

  void updateCameraInfo()
  {
    DirectImageMetaC local {};
    {
      std::lock_guard<std::mutex> lock(image_mutex_);
      local = image_meta_;
    }
    try {
      const auto params = pipeline_->getCameraParam();
      const auto & k = params.rgbIntrinsic;
      const auto & d = params.rgbDistortion;
      local.camera_info_valid = 1;
      local.K[0] = k.fx;
      local.K[1] = 0.0;
      local.K[2] = k.cx;
      local.K[3] = 0.0;
      local.K[4] = k.fy;
      local.K[5] = k.cy;
      local.K[6] = 0.0;
      local.K[7] = 0.0;
      local.K[8] = 1.0;
      local.D[0] = d.k1;
      local.D[1] = d.k2;
      local.D[2] = d.p1;
      local.D[3] = d.p2;
      local.D[4] = d.k3;
    } catch (const std::exception &) {
      local.camera_info_valid = 0;
    }
    {
      std::lock_guard<std::mutex> lock(image_mutex_);
      for (int i = 0; i < 9; ++i) {
        image_meta_.K[i] = local.K[i];
      }
      for (int i = 0; i < 5; ++i) {
        image_meta_.D[i] = local.D[i];
      }
      image_meta_.camera_info_valid = local.camera_info_valid;
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

  void cameraLoop()
  {
    while (!stop_requested_.load()) {
      try {
        auto frame_set = pipeline_->waitForFrameset(static_cast<uint32_t>(frame_timeout_ms_));
        if (!frame_set) {
          continue;
        }
        auto color = frame_set->getColorFrame();
        if (!color) {
          continue;
        }

        cv::Mat image = colorFrameToBgr(color);
        if (image.empty()) {
          continue;
        }
        if (output_encoding_ == 0) {
          cv::cvtColor(image, image, cv::COLOR_BGR2GRAY);
        }
        if (!image.isContinuous()) {
          image = image.clone();
        }

        DirectImageMetaC meta {};
        {
          std::lock_guard<std::mutex> lock(image_mutex_);
          meta = image_meta_;
          meta.seq = image_meta_.seq + 1;
        }
        meta.stamp_us = stampFromFrameUs(color);
        meta.stamp_s = static_cast<double>(meta.stamp_us) * 1e-6;
        if (meta.stamp_s <= 0.0) {
          using clock = std::chrono::system_clock;
          meta.stamp_s = std::chrono::duration<double>(clock::now().time_since_epoch()).count();
          meta.stamp_us = static_cast<uint64_t>(meta.stamp_s * 1e6);
        }
        meta.width = image.cols;
        meta.height = image.rows;
        meta.channels = image.channels();
        meta.step = static_cast<int>(image.step);
        meta.encoding = output_encoding_;

        double capture_rate_hz = 0.0;
        {
          std::lock_guard<std::mutex> lock(stats_mutex_);
          ++image_count_;
          const double now = nowSteadySeconds();
          image_times_.push_back(now);
          trimTimes(image_times_, now);
          capture_rate_hz = rateFromTimes(image_times_);
        }
        meta.capture_rate_hz = capture_rate_hz;

        const auto size = static_cast<std::size_t>(image.total() * image.elemSize());
        {
          std::lock_guard<std::mutex> lock(image_mutex_);
          image_meta_ = meta;
          image_data_.assign(image.data, image.data + size);
        }
      } catch (const std::exception & exc) {
        setStatus(std::string("direct camera poll failed: ") + exc.what());
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
      }
    }
  }

  void imuLoop()
  {
    while (!stop_requested_.load()) {
      try {
        drainSerialImu();
      } catch (const std::exception & exc) {
        setStatus(std::string("direct IMU drain failed: ") + exc.what());
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
  }

  void drainSerialImu()
  {
    if (!serial_imu_ || !serial_imu_->isStarted()) {
      return;
    }

    const auto samples = serial_imu_->drainSamples();
    std::vector<DirectImuSampleC> packets;
    packets.reserve(samples.size() / 2);
    for (const auto & sample : samples) {
      if (sample.type == ImuSensorType::Accel) {
        latest_accel_ = sample.value;
        latest_accel_valid_ = true;
        continue;
      }
      if (sample.type != ImuSensorType::Gyro || !latest_accel_valid_) {
        continue;
      }

      DirectImuSampleC packet {};
      packet.index = sample.index;
      packet.stamp_s = sample.system_timestamp_us > 0 ?
        static_cast<double>(sample.system_timestamp_us) * 1e-6 : sample.timestamp_s;
      packet.ax = latest_accel_.x();
      packet.ay = latest_accel_.y();
      packet.az = latest_accel_.z();
      packet.gx = sample.value.x();
      packet.gy = sample.value.y();
      packet.gz = sample.value.z();
      packets.push_back(packet);
    }

    if (packets.empty()) {
      return;
    }
    {
      std::lock_guard<std::mutex> lock(imu_mutex_);
      for (const auto & packet : packets) {
        imu_packets_.push_back(packet);
        while (imu_packets_.size() > kMaxQueuedImuPackets) {
          imu_packets_.pop_front();
          ++dropped_imu_count_;
        }
      }
    }
    {
      std::lock_guard<std::mutex> lock(stats_mutex_);
      const double now = nowSteadySeconds();
      for (std::size_t i = 0; i < packets.size(); ++i) {
        imu_times_.push_back(now - static_cast<double>(packets.size() - 1 - i) *
          (1.0 / std::max(1.0, expected_imu_rate_hz_)));
      }
      trimTimes(imu_times_, now);
      imu_count_ += packets.size();
      imu_online_ = true;
    }
  }

  std::string cameraStatusLine() const
  {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    std::ostringstream oss;
    oss << "color=" << actual_color_width_ << "x" << actual_color_height_ << "@"
        << actual_color_fps_ << " " << (output_encoding_ == 0 ? "mono8" : "bgr8");
    if (serial_imu_) {
      oss << " imu=" << serial_imu_->statusLine();
    }
    return oss.str();
  }

  void setStatus(const std::string & value)
  {
    std::lock_guard<std::mutex> lock(status_mutex_);
    status_line_ = value;
  }

  DirectSensorConfigC config_ {};
  int output_encoding_ {0};
  int frame_timeout_ms_ {1000};
  double expected_imu_rate_hz_ {500.0};

  std::atomic_bool started_ {false};
  std::atomic_bool stop_requested_ {false};
  mutable std::mutex lifecycle_mutex_;
  std::thread camera_thread_;
  std::thread imu_thread_;

  std::shared_ptr<ob::Pipeline> pipeline_;
  std::shared_ptr<ob::VideoStreamProfile> color_profile_;
  std::unique_ptr<SerialImuReader> serial_imu_;

  mutable std::mutex image_mutex_;
  DirectImageMetaC image_meta_ {};
  std::vector<uint8_t> image_data_;

  mutable std::mutex imu_mutex_;
  std::deque<DirectImuSampleC> imu_packets_;
  Eigen::Vector3d latest_accel_ {Eigen::Vector3d::Zero()};
  bool latest_accel_valid_ {false};

  mutable std::mutex stats_mutex_;
  bool camera_online_ {false};
  bool imu_online_ {false};
  int actual_color_width_ {0};
  int actual_color_height_ {0};
  int actual_color_fps_ {0};
  uint64_t image_count_ {0};
  uint64_t imu_count_ {0};
  uint64_t dropped_imu_count_ {0};
  std::deque<double> image_times_;
  std::deque<double> imu_times_;

  mutable std::mutex status_mutex_;
  std::string status_line_ {"direct sensor stopped"};
};

}  // namespace
}  // namespace cube_imu_calibration

extern "C" {

void * direct_sensor_create()
{
  return new cube_imu_calibration::DirectSensorRuntime();
}

void direct_sensor_destroy(void * handle)
{
  delete static_cast<cube_imu_calibration::DirectSensorRuntime *>(handle);
}

int direct_sensor_start(void * handle, const DirectSensorConfigC * config)
{
  if (handle == nullptr || config == nullptr) {
    return 0;
  }
  return static_cast<cube_imu_calibration::DirectSensorRuntime *>(handle)->start(*config);
}

void direct_sensor_stop(void * handle)
{
  if (handle == nullptr) {
    return;
  }
  static_cast<cube_imu_calibration::DirectSensorRuntime *>(handle)->stop();
}

int direct_sensor_is_started(void * handle)
{
  if (handle == nullptr) {
    return 0;
  }
  return static_cast<cube_imu_calibration::DirectSensorRuntime *>(handle)->isStarted() ? 1 : 0;
}

std::size_t direct_sensor_copy_latest_image(
  void * handle, DirectImageMetaC * meta, uint8_t * dst, std::size_t capacity)
{
  if (handle == nullptr) {
    return 0;
  }
  return static_cast<cube_imu_calibration::DirectSensorRuntime *>(handle)->copyLatestImage(
    meta, dst, capacity);
}

int direct_sensor_drain_imu(void * handle, DirectImuSampleC * dst, int max_samples)
{
  if (handle == nullptr) {
    return 0;
  }
  return static_cast<cube_imu_calibration::DirectSensorRuntime *>(handle)->drainImu(
    dst, max_samples);
}

int direct_sensor_get_stats(void * handle, DirectSensorStatsC * stats)
{
  if (handle == nullptr || stats == nullptr) {
    return 0;
  }
  *stats = static_cast<cube_imu_calibration::DirectSensorRuntime *>(handle)->stats();
  return 1;
}

int direct_sensor_get_status(void * handle, char * dst, std::size_t capacity)
{
  if (handle == nullptr || dst == nullptr || capacity == 0) {
    return 0;
  }
  const std::string status =
    static_cast<cube_imu_calibration::DirectSensorRuntime *>(handle)->statusLine();
  const std::size_t n = std::min<std::size_t>(capacity - 1, status.size());
  std::memcpy(dst, status.data(), n);
  dst[n] = '\0';
  return static_cast<int>(n);
}

}  // extern "C"
