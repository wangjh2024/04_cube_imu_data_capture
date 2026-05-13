#pragma once

#include <atomic>
#include <cstdint>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include <Eigen/Core>

namespace cube_imu_calibration
{

enum class ImuSensorType
{
  Accel,
  Gyro,
};

struct ImuSample
{
  ImuSensorType type {ImuSensorType::Accel};
  uint64_t index {0};
  uint64_t timestamp_us {0};
  uint64_t system_timestamp_us {0};
  double timestamp_s {0.0};
  Eigen::Vector3d value {Eigen::Vector3d::Zero()};
  bool has_raw_value {false};
  Eigen::Vector3i raw_value {Eigen::Vector3i::Zero()};
};

struct SerialImuConfig
{
  bool enabled {true};
  std::string display_name;
  std::string port {"auto"};
  int baudrate {2000000};
  std::string protocol_mode {"v2.1"};
  double imu_rate_hz {500.0};
  std::string startup_command {"uartout"};
};

class SerialImuReader
{
public:
  explicit SerialImuReader(SerialImuConfig config);
  ~SerialImuReader();

  void start();
  void stop();
  bool isStarted() const;
  std::vector<ImuSample> drainSamples();
  std::string statusLine() const;
  std::string portName() const;

private:
  std::string resolvePort() const;
  bool openPort(const std::string & port);
  void configurePort();
  void sendStartupCommand();
  void readLoop();
  void readAvailable();
  void parseBuffer();
  void parseFrame(const std::vector<uint8_t> & frame);
  void parseImuPayload(
    const uint8_t * payload, std::size_t payload_len, uint64_t base_timestamp_us,
    bool has_mag);

  SerialImuConfig config_;
  int fd_ {-1};
  bool started_ {false};
  std::string port_;
  std::string protocol_mode_;
  uint64_t sample_interval_us_ {2000};
  uint64_t sample_index_ {0};
  std::vector<uint8_t> buffer_;
  std::vector<ImuSample> samples_;
  std::atomic_bool stop_requested_ {false};
  std::thread reader_thread_;
  mutable std::mutex mutex_;
};

}  // namespace cube_imu_calibration
