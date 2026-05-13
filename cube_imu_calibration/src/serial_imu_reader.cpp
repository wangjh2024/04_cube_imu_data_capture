#include "serial_imu_reader.hpp"

#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

#include <algorithm>
#include <array>
#include <cerrno>
#include <chrono>
#include <cctype>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <utility>

namespace cube_imu_calibration
{

namespace
{

constexpr uint8_t kBlockDataImu6 = 0x02;
constexpr uint8_t kBlockDataImu9 = 0x09;
constexpr std::size_t kMaxFrameLen = 4096;
constexpr double kGravity = 9.80665;

std::string normalizedProtocolMode(std::string mode)
{
  std::transform(
    mode.begin(), mode.end(), mode.begin(),
    [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
  if (mode == "protocol_v2") {
    return "v2";
  }
  if (mode == "protocol_v2_1" || mode == "protocol_v21") {
    return "v2.1";
  }
  if (mode == "protocol_v3") {
    return "v3";
  }
  return mode;
}

uint64_t nowSystemUs()
{
  using clock = std::chrono::system_clock;
  return static_cast<uint64_t>(
    std::chrono::duration_cast<std::chrono::microseconds>(clock::now().time_since_epoch())
      .count());
}

int16_t readI16Le(const uint8_t * p)
{
  return static_cast<int16_t>(static_cast<uint16_t>(p[0]) | (static_cast<uint16_t>(p[1]) << 8));
}

uint16_t readU16Le(const uint8_t * p)
{
  return static_cast<uint16_t>(p[0]) | (static_cast<uint16_t>(p[1]) << 8);
}

uint32_t readU24Le(const uint8_t * p)
{
  return static_cast<uint32_t>(p[0]) | (static_cast<uint32_t>(p[1]) << 8) |
         (static_cast<uint32_t>(p[2]) << 16);
}

uint64_t readU48Le(const uint8_t * p)
{
  uint64_t value = 0;
  for (int i = 0; i < 6; ++i) {
    value |= static_cast<uint64_t>(p[i]) << (8 * i);
  }
  return value;
}

std::vector<uint8_t> makeCrc8Table()
{
  std::vector<uint8_t> table(256);
  for (int value = 0; value < 256; ++value) {
    uint8_t crc = static_cast<uint8_t>(value);
    for (int bit = 0; bit < 8; ++bit) {
      crc = (crc & 0x80) ? static_cast<uint8_t>((crc << 1) ^ 0x07) :
        static_cast<uint8_t>(crc << 1);
    }
    table[static_cast<std::size_t>(value)] = crc;
  }
  return table;
}

uint8_t crc8(const std::vector<uint8_t> & data)
{
  static const std::vector<uint8_t> table = makeCrc8Table();
  uint8_t crc = 0;
  for (uint8_t byte : data) {
    crc = table[static_cast<std::size_t>((crc ^ byte) & 0xff)];
  }
  return crc;
}

speed_t baudToTermios(int baudrate)
{
  switch (baudrate) {
    case 9600:
      return B9600;
    case 57600:
      return B57600;
    case 115200:
      return B115200;
    case 230400:
      return B230400;
    case 460800:
      return B460800;
    case 921600:
      return B921600;
#ifdef B1000000
    case 1000000:
      return B1000000;
#endif
#ifdef B2000000
    case 2000000:
      return B2000000;
#endif
    default:
      throw std::runtime_error("Unsupported serial baudrate: " + std::to_string(baudrate));
  }
}

std::vector<std::string> enumerateSerialPorts()
{
  std::vector<std::string> ports;
  const std::filesystem::path dev("/dev");
  std::error_code ec;
  if (!std::filesystem::exists(dev, ec)) {
    return ports;
  }
  for (std::filesystem::directory_iterator it(dev, ec), end; !ec && it != end; it.increment(ec)) {
    const std::string path = it->path().string();
    const std::string name = it->path().filename().string();
    if (name.rfind("ttyACM", 0) == 0 || name.rfind("ttyUSB", 0) == 0) {
      ports.push_back(path);
    }
  }
  std::sort(ports.begin(), ports.end());
  return ports;
}

}  // namespace

SerialImuReader::SerialImuReader(SerialImuConfig config)
: config_(std::move(config)),
  protocol_mode_(normalizedProtocolMode(config_.protocol_mode)),
  sample_interval_us_(static_cast<uint64_t>(
      std::max(1.0, std::round(1000000.0 / std::max(1.0, config_.imu_rate_hz)))))
{
}

SerialImuReader::~SerialImuReader()
{
  stop();
}

bool SerialImuReader::isStarted() const
{
  return started_;
}

std::string SerialImuReader::resolvePort() const
{
  if (!config_.port.empty() && config_.port != "auto") {
    return config_.port;
  }
  const auto ports = enumerateSerialPorts();
  return ports.empty() ? std::string() : ports.front();
}

void SerialImuReader::start()
{
  if (started_ || !config_.enabled) {
    return;
  }
  port_ = resolvePort();
  if (port_.empty()) {
    throw std::runtime_error("No /dev/ttyACM* or /dev/ttyUSB* serial IMU port was found.");
  }
  if (!openPort(port_)) {
    throw std::runtime_error("Failed to open serial IMU port: " + port_);
  }
  stop_requested_.store(false);
  started_ = true;
  reader_thread_ = std::thread(&SerialImuReader::readLoop, this);
}

bool SerialImuReader::openPort(const std::string & port)
{
  fd_ = ::open(port.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
  if (fd_ < 0) {
    std::cerr << "Failed to open " << port << ": " << std::strerror(errno) << std::endl;
    return false;
  }

  try {
    configurePort();
    sendStartupCommand();
  } catch (const std::exception & exc) {
    std::cerr << "Failed to configure " << port << ": " << exc.what() << std::endl;
    stop();
    return false;
  }
  return true;
}

void SerialImuReader::configurePort()
{
  termios tty {};
  if (tcgetattr(fd_, &tty) != 0) {
    throw std::runtime_error(std::strerror(errno));
  }
  cfmakeraw(&tty);
  const speed_t baud = baudToTermios(config_.baudrate);
  cfsetispeed(&tty, baud);
  cfsetospeed(&tty, baud);
  tty.c_cflag |= static_cast<tcflag_t>(CLOCAL | CREAD);
  tty.c_cflag &= static_cast<tcflag_t>(~CRTSCTS);
  tty.c_cc[VMIN] = 0;
  tty.c_cc[VTIME] = 0;
  if (tcsetattr(fd_, TCSANOW, &tty) != 0) {
    throw std::runtime_error(std::strerror(errno));
  }
  tcflush(fd_, TCIOFLUSH);
}

void SerialImuReader::sendStartupCommand()
{
  if (config_.startup_command.empty()) {
    return;
  }
  std::this_thread::sleep_for(std::chrono::milliseconds(200));
  const std::vector<std::string> payloads = {
    config_.startup_command,
    config_.startup_command + "\r",
    config_.startup_command + "\n",
    config_.startup_command + "\r\n",
  };
  for (const auto & payload : payloads) {
    const ssize_t written = ::write(fd_, payload.data(), payload.size());
    (void)written;
    tcdrain(fd_);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
  tcflush(fd_, TCIFLUSH);
}

void SerialImuReader::stop()
{
  stop_requested_.store(true);
  if (reader_thread_.joinable()) {
    reader_thread_.join();
  }
  {
    std::lock_guard<std::mutex> lock(mutex_);
    if (fd_ >= 0) {
      ::close(fd_);
      fd_ = -1;
    }
    buffer_.clear();
    samples_.clear();
  }
  started_ = false;
}

void SerialImuReader::readLoop()
{
  while (!stop_requested_.load()) {
    {
      std::lock_guard<std::mutex> lock(mutex_);
      readAvailable();
      parseBuffer();
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
}

void SerialImuReader::readAvailable()
{
  if (fd_ < 0) {
    return;
  }
  std::array<uint8_t, 8192> tmp {};
  while (true) {
    const ssize_t n = ::read(fd_, tmp.data(), tmp.size());
    if (n > 0) {
      buffer_.insert(buffer_.end(), tmp.begin(), tmp.begin() + n);
      continue;
    }
    if (n < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
      std::cerr << "Serial IMU read failed on " << port_ << ": " << std::strerror(errno) <<
        std::endl;
    }
    break;
  }
}

std::vector<ImuSample> SerialImuReader::drainSamples()
{
  std::lock_guard<std::mutex> lock(mutex_);
  std::vector<ImuSample> out;
  out.swap(samples_);
  return out;
}

void SerialImuReader::parseBuffer()
{
  const std::size_t header_len = protocol_mode_ == "v3" ? 12 : 8;
  const std::size_t crc_index = protocol_mode_ == "v3" ? 11 : 7;

  while (buffer_.size() >= header_len) {
    std::size_t magic_pos = std::string::npos;
    for (std::size_t i = 0; i + 1 < buffer_.size(); ++i) {
      const bool magic_v21 = buffer_[i] == 0x5A && buffer_[i + 1] == 0xA5;
      const bool magic_v2 = buffer_[i] == 0x5B && buffer_[i + 1] == 0xB5;
      if ((protocol_mode_ == "v2" && magic_v2) ||
        (protocol_mode_ != "v2" && (magic_v21 || magic_v2)))
      {
        magic_pos = i;
        break;
      }
    }
    if (magic_pos == std::string::npos) {
      buffer_.clear();
      return;
    }
    if (magic_pos > 0) {
      buffer_.erase(buffer_.begin(), buffer_.begin() + static_cast<std::ptrdiff_t>(magic_pos));
      if (buffer_.size() < header_len) {
        return;
      }
    }

    const uint16_t total_length = readU16Le(buffer_.data() + 2);
    if (total_length < header_len || total_length > kMaxFrameLen) {
      buffer_.erase(buffer_.begin(), buffer_.begin() + 2);
      continue;
    }
    if (buffer_.size() < total_length) {
      return;
    }

    std::vector<uint8_t> frame(buffer_.begin(), buffer_.begin() + total_length);
    buffer_.erase(buffer_.begin(), buffer_.begin() + total_length);

    const uint8_t expected_crc = frame[crc_index];
    frame[crc_index] = 0;
    if (crc8(frame) != expected_crc) {
      buffer_.insert(buffer_.begin(), frame.begin() + 2, frame.end());
      continue;
    }
    frame[crc_index] = expected_crc;
    parseFrame(frame);
  }
}

void SerialImuReader::parseFrame(const std::vector<uint8_t> & frame)
{
  const bool v3 = protocol_mode_ == "v3";
  const std::size_t header_len = v3 ? 12 : 8;
  if (frame.size() < header_len) {
    return;
  }
  if (v3 && frame[4] != 3) {
    return;
  }

  const uint64_t frame_timestamp = v3 ? readU48Le(frame.data() + 5) :
    readU24Le(frame.data() + 4);
  std::size_t offset = header_len;
  while (offset + 4 <= frame.size()) {
    const uint8_t block_type = frame[offset];
    const uint8_t payload_len = frame[offset + 1];
    const uint16_t block_timestamp = readU16Le(frame.data() + offset + 2);
    const std::size_t payload_start = offset + 4;
    const std::size_t payload_end = payload_start + payload_len;
    if (payload_end > frame.size()) {
      break;
    }

    if (block_type == kBlockDataImu6 || block_type == kBlockDataImu9) {
      const uint64_t base_timestamp_us = (frame_timestamp << 16) | block_timestamp;
      parseImuPayload(
        frame.data() + payload_start, payload_len, base_timestamp_us,
        block_type == kBlockDataImu9);
    }
    offset = payload_end;
  }
}

void SerialImuReader::parseImuPayload(
  const uint8_t * payload, std::size_t payload_len, uint64_t base_timestamp_us, bool has_mag)
{
  const std::size_t stride = has_mag ? 18 : 12;
  const std::size_t usable = (payload_len / stride) * stride;
  const std::size_t sample_count = usable / stride;
  const uint64_t batch_end_us = nowSystemUs();
  const uint64_t batch_span_us = sample_count > 0 ?
    static_cast<uint64_t>(sample_count - 1) * sample_interval_us_ : 0;
  const uint64_t batch_start_us = batch_end_us > batch_span_us ?
    batch_end_us - batch_span_us : batch_end_us;

  for (std::size_t offset = 0; offset < usable; offset += stride) {
    const int16_t ax = readI16Le(payload + offset);
    const int16_t ay = readI16Le(payload + offset + 2);
    const int16_t az = readI16Le(payload + offset + 4);
    const int16_t gx = readI16Le(payload + offset + 6);
    const int16_t gy = readI16Le(payload + offset + 8);
    const int16_t gz = readI16Le(payload + offset + 10);

    const std::size_t sample_offset = offset / stride;
    const uint64_t stamp_us =
      batch_start_us + static_cast<uint64_t>(sample_offset) * sample_interval_us_;
    const uint64_t device_stamp_us =
      base_timestamp_us + static_cast<uint64_t>(sample_offset) * sample_interval_us_;
    const double stamp_s = static_cast<double>(stamp_us) * 1e-6;

    ImuSample accel;
    accel.type = ImuSensorType::Accel;
    accel.index = sample_index_;
    accel.timestamp_us = device_stamp_us;
    accel.system_timestamp_us = stamp_us;
    accel.timestamp_s = stamp_s;
    accel.has_raw_value = true;
    accel.raw_value = Eigen::Vector3i(ax, ay, az);
    accel.value = Eigen::Vector3d(ax, ay, az) * (4.0 * kGravity / 32768.0);

    ImuSample gyro;
    gyro.type = ImuSensorType::Gyro;
    gyro.index = sample_index_;
    gyro.timestamp_us = device_stamp_us;
    gyro.system_timestamp_us = stamp_us;
    gyro.timestamp_s = stamp_s;
    gyro.has_raw_value = true;
    gyro.raw_value = Eigen::Vector3i(gx, gy, gz);
    gyro.value = Eigen::Vector3d(gx, gy, gz) * (2000.0 * M_PI / (180.0 * 32768.0));

    samples_.push_back(accel);
    samples_.push_back(gyro);
    ++sample_index_;
  }
}

std::string SerialImuReader::statusLine() const
{
  std::lock_guard<std::mutex> lock(mutex_);
  return "port=" + port_ + " baud=" + std::to_string(config_.baudrate) +
         " protocol=" + protocol_mode_;
}

std::string SerialImuReader::portName() const
{
  std::lock_guard<std::mutex> lock(mutex_);
  return port_;
}

}  // namespace cube_imu_calibration
