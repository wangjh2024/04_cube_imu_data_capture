#include <algorithm>
#include <array>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <map>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <vector>

#include "cv_bridge/cv_bridge.hpp"
#include "geometry_msgs/msg/transform_stamped.hpp"
#include "opencv2/aruco.hpp"
#include "opencv2/calib3d.hpp"
#include "opencv2/imgproc.hpp"
#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/image_encodings.hpp"
#include "sensor_msgs/msg/camera_info.hpp"
#include "sensor_msgs/msg/image.hpp"
#include "tf2/LinearMath/Matrix3x3.h"
#include "tf2/LinearMath/Quaternion.h"
#include "tf2_ros/transform_broadcaster.h"

#include "cube_imu_calibration/msg/cube_pose.hpp"

namespace cube_imu_calibration
{

namespace
{

cv::Matx44d identity44()
{
  return cv::Matx44d::eye();
}

cv::Matx44d makeTransform(const cv::Matx33d & rotation, const cv::Vec3d & translation)
{
  cv::Matx44d transform = identity44();
  for (int row = 0; row < 3; ++row) {
    for (int col = 0; col < 3; ++col) {
      transform(row, col) = rotation(row, col);
    }
    transform(row, 3) = translation(row);
  }
  return transform;
}

cv::Matx33d rotationFromQuaternionXyzw(const std::vector<double> & q_xyzw)
{
  tf2::Quaternion q(q_xyzw[0], q_xyzw[1], q_xyzw[2], q_xyzw[3]);
  if (q.length2() < 1e-16) {
    q = tf2::Quaternion(0.0, 0.0, 0.0, 1.0);
  }
  q.normalize();

  tf2::Matrix3x3 tf_rotation(q);
  cv::Matx33d rotation;
  for (int row = 0; row < 3; ++row) {
    for (int col = 0; col < 3; ++col) {
      rotation(row, col) = tf_rotation[row][col];
    }
  }
  return rotation;
}

tf2::Quaternion quaternionFromRotation(const cv::Matx44d & transform)
{
  tf2::Matrix3x3 tf_rotation(
    transform(0, 0), transform(0, 1), transform(0, 2),
    transform(1, 0), transform(1, 1), transform(1, 2),
    transform(2, 0), transform(2, 1), transform(2, 2));
  tf2::Quaternion q;
  tf_rotation.getRotation(q);
  if (q.length2() < 1e-16) {
    q = tf2::Quaternion(0.0, 0.0, 0.0, 1.0);
  }
  q.normalize();
  return q;
}

std::string idsToString(const std::vector<int> & ids)
{
  std::ostringstream oss;
  for (size_t i = 0; i < ids.size(); ++i) {
    if (i > 0) {
      oss << ",";
    }
    oss << ids[i];
  }
  return oss.str();
}

}  // namespace

class AprilTagPoseNode : public rclcpp::Node
{
public:
  AprilTagPoseNode()
  : Node("apriltag_pose_node"),
    tf_broadcaster_(std::make_unique<tf2_ros::TransformBroadcaster>(*this))
  {
    image_topic_ = declare_parameter<std::string>("image_topic", "/head_camera/image_raw");
    camera_info_topic_ = declare_parameter<std::string>(
      "camera_info_topic", "/head_camera/camera_info");
    use_camera_info_ = declare_parameter<bool>("use_camera_info", true);
    marker_size_ = declare_parameter<double>("marker_size", 0.08);
    target_tag_id_ = declare_parameter<int>("target_tag_id", -1);
    dictionary_name_ = declare_parameter<std::string>("dictionary", "DICT_APRILTAG_36h11");
    camera_frame_ = declare_parameter<std::string>("camera_frame", "camera_link");
    cube_frame_ = declare_parameter<std::string>("cube_frame", "cube_link");
    publish_tf_ = declare_parameter<bool>("publish_tf", true);
    log_every_n_detections_ = declare_parameter<int>("log_every_n_detections", 30);
    pose_csv_path_ = declare_parameter<std::string>("pose_csv_path", "");
    pose_csv_append_ = declare_parameter<bool>("pose_csv_append", false);

    if (marker_size_ <= 0.0) {
      RCLCPP_WARN(get_logger(), "marker_size must be positive; using 0.08 m");
      marker_size_ = 0.08;
    }
    if (log_every_n_detections_ <= 0) {
      log_every_n_detections_ = 30;
    }

    loadCameraParameters();
    loadTagToCubeTransform();
    configureDetector();
    openPoseCsv();

    pose_pub_ = create_publisher<cube_imu_calibration::msg::CubePose>("/cube_pose", 10);

    image_sub_ = create_subscription<sensor_msgs::msg::Image>(
      image_topic_, rclcpp::SensorDataQoS(),
      [this](sensor_msgs::msg::Image::ConstSharedPtr msg) { imageCallback(msg); });

    if (use_camera_info_) {
      camera_info_sub_ = create_subscription<sensor_msgs::msg::CameraInfo>(
        camera_info_topic_, rclcpp::QoS(10),
        [this](sensor_msgs::msg::CameraInfo::ConstSharedPtr msg) { cameraInfoCallback(msg); });
    }

    RCLCPP_INFO(get_logger(), "Subscribed image topic: %s", image_topic_.c_str());
    if (use_camera_info_) {
      RCLCPP_INFO(get_logger(), "Subscribed camera info topic: %s", camera_info_topic_.c_str());
    }
    RCLCPP_INFO(
      get_logger(), "AprilTag dictionary=%s, marker_size=%.6f m, target_tag_id=%d",
      dictionary_name_.c_str(), marker_size_, target_tag_id_);
    RCLCPP_INFO(
      get_logger(), "Publishing T_Cam_Cube on /cube_pose and TF %s -> %s",
      camera_frame_.c_str(), cube_frame_.c_str());
  }

private:
  void configureDetector()
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

    detector_params_ = cv::aruco::DetectorParameters::create();
    detector_params_->cornerRefinementMethod = cv::aruco::CORNER_REFINE_APRILTAG;
    detector_params_->cornerRefinementWinSize = 5;
    detector_params_->cornerRefinementMaxIterations = 50;
    detector_params_->cornerRefinementMinAccuracy = 0.01;

    estimate_params_ = cv::aruco::EstimateParameters::create();
    estimate_params_->pattern = cv::aruco::CCW_center;
    estimate_params_->solvePnPMethod = cv::SOLVEPNP_ITERATIVE;
  }

  void loadCameraParameters()
  {
    const std::vector<double> camera_matrix_param = declare_parameter<std::vector<double>>(
      "camera_matrix",
      {600.0, 0.0, 320.0, 0.0, 600.0, 240.0, 0.0, 0.0, 1.0});
    const std::vector<double> dist_coeffs_param = declare_parameter<std::vector<double>>(
      "dist_coeffs", {0.0, 0.0, 0.0, 0.0, 0.0});

    if (camera_matrix_param.size() != 9) {
      RCLCPP_ERROR(
        get_logger(), "camera_matrix must contain 9 values; AprilTag pose is disabled");
      camera_ready_ = false;
      return;
    }

    std::lock_guard<std::mutex> lock(camera_mutex_);
    camera_matrix_ = cv::Mat(3, 3, CV_64F);
    for (int idx = 0; idx < 9; ++idx) {
      camera_matrix_.at<double>(idx / 3, idx % 3) = camera_matrix_param[idx];
    }

    dist_coeffs_ = cv::Mat(1, static_cast<int>(dist_coeffs_param.size()), CV_64F);
    for (size_t idx = 0; idx < dist_coeffs_param.size(); ++idx) {
      dist_coeffs_.at<double>(0, static_cast<int>(idx)) = dist_coeffs_param[idx];
    }

    camera_ready_ = true;
    RCLCPP_INFO(
      get_logger(),
      "Loaded fallback camera intrinsics fx=%.3f fy=%.3f cx=%.3f cy=%.3f",
      camera_matrix_.at<double>(0, 0), camera_matrix_.at<double>(1, 1),
      camera_matrix_.at<double>(0, 2), camera_matrix_.at<double>(1, 2));
  }

  void loadTagToCubeTransform()
  {
    const std::vector<double> translation = declare_parameter<std::vector<double>>(
      "tag_to_cube_translation", {0.0, 0.0, 0.0});
    const std::vector<double> quaternion = declare_parameter<std::vector<double>>(
      "tag_to_cube_quaternion_xyzw", {0.0, 0.0, 0.0, 1.0});

    if (translation.size() != 3 || quaternion.size() != 4) {
      RCLCPP_WARN(
        get_logger(),
        "Invalid T_tag_cube parameter lengths; using identity cube frame at tag center");
      tag_to_cube_transform_ = identity44();
      return;
    }

    const cv::Matx33d rotation = rotationFromQuaternionXyzw(quaternion);
    tag_to_cube_transform_ = makeTransform(
      rotation, cv::Vec3d(translation[0], translation[1], translation[2]));
  }

  void openPoseCsv()
  {
    if (pose_csv_path_.empty()) {
      return;
    }

    try {
      std::filesystem::path path(pose_csv_path_);
      if (path.is_relative()) {
        path = std::filesystem::absolute(path);
      }
      if (!path.parent_path().empty()) {
        std::filesystem::create_directories(path.parent_path());
      }

      const bool write_header = !pose_csv_append_ || !std::filesystem::exists(path) ||
        std::filesystem::file_size(path) == 0;
      const auto mode = pose_csv_append_ ? std::ios::app : std::ios::trunc;
      pose_csv_.open(path, std::ios::out | mode);
      if (!pose_csv_.is_open()) {
        RCLCPP_ERROR(get_logger(), "Failed to open pose CSV: %s", path.string().c_str());
        return;
      }
      pose_csv_path_ = path.string();
      if (write_header) {
        pose_csv_ << "timestamp,tx,ty,tz,qx,qy,qz,qw\n";
      }
      RCLCPP_INFO(get_logger(), "Writing AprilTag/Cube pose CSV: %s", pose_csv_path_.c_str());
    } catch (const std::exception & e) {
      RCLCPP_ERROR(get_logger(), "Failed to prepare pose CSV '%s': %s", pose_csv_path_.c_str(), e.what());
    }
  }

  void writePoseCsv(const builtin_interfaces::msg::Time & stamp, const cv::Matx44d & transform)
  {
    if (!pose_csv_.is_open()) {
      return;
    }

    const tf2::Quaternion q = quaternionFromRotation(transform);
    const double timestamp = static_cast<double>(stamp.sec) +
      static_cast<double>(stamp.nanosec) * 1e-9;
    pose_csv_ << std::fixed << std::setprecision(9)
              << timestamp << ","
              << std::setprecision(12)
              << transform(0, 3) << ","
              << transform(1, 3) << ","
              << transform(2, 3) << ","
              << q.x() << ","
              << q.y() << ","
              << q.z() << ","
              << q.w() << "\n";
    pose_csv_.flush();
  }

  void cameraInfoCallback(const sensor_msgs::msg::CameraInfo::ConstSharedPtr msg)
  {
    if (msg->k[0] == 0.0 || msg->k[4] == 0.0) {
      RCLCPP_WARN_THROTTLE(
        get_logger(), *get_clock(), 5000,
        "Received CameraInfo with invalid focal length; keeping previous intrinsics");
      return;
    }

    std::lock_guard<std::mutex> lock(camera_mutex_);
    camera_matrix_ = cv::Mat(3, 3, CV_64F);
    for (int idx = 0; idx < 9; ++idx) {
      camera_matrix_.at<double>(idx / 3, idx % 3) = msg->k[idx];
    }

    if (msg->d.empty()) {
      dist_coeffs_ = cv::Mat::zeros(1, 5, CV_64F);
    } else {
      dist_coeffs_ = cv::Mat(1, static_cast<int>(msg->d.size()), CV_64F);
      for (size_t idx = 0; idx < msg->d.size(); ++idx) {
        dist_coeffs_.at<double>(0, static_cast<int>(idx)) = msg->d[idx];
      }
    }
    camera_ready_ = true;

    if (!logged_camera_info_) {
      RCLCPP_INFO(
        get_logger(),
        "Using CameraInfo intrinsics fx=%.3f fy=%.3f cx=%.3f cy=%.3f",
        camera_matrix_.at<double>(0, 0), camera_matrix_.at<double>(1, 1),
        camera_matrix_.at<double>(0, 2), camera_matrix_.at<double>(1, 2));
      logged_camera_info_ = true;
    }
  }

  bool getCameraParameters(cv::Mat & camera_matrix, cv::Mat & dist_coeffs)
  {
    std::lock_guard<std::mutex> lock(camera_mutex_);
    if (!camera_ready_) {
      return false;
    }
    camera_matrix = camera_matrix_.clone();
    dist_coeffs = dist_coeffs_.clone();
    return true;
  }

  bool selectTarget(
    const std::vector<int> & ids,
    const std::vector<std::vector<cv::Point2f>> & corners,
    std::vector<std::vector<cv::Point2f>> & selected_corners,
    int & selected_id)
  {
    if (ids.empty()) {
      return false;
    }

    size_t selected_index = 0;
    if (target_tag_id_ >= 0) {
      const auto id_iter = std::find(ids.begin(), ids.end(), target_tag_id_);
      if (id_iter == ids.end()) {
        RCLCPP_WARN_THROTTLE(
          get_logger(), *get_clock(), 2000,
          "Target AprilTag id=%d not found; detected ids=[%s]",
          target_tag_id_, idsToString(ids).c_str());
        return false;
      }
      selected_index = static_cast<size_t>(std::distance(ids.begin(), id_iter));
    }

    selected_id = ids[selected_index];
    selected_corners.clear();
    selected_corners.push_back(corners[selected_index]);
    return true;
  }

  cv::Matx44d estimateCamCubeTransform(
    const std::vector<std::vector<cv::Point2f>> & selected_corners,
    const cv::Mat & camera_matrix,
    const cv::Mat & dist_coeffs)
  {
    std::vector<cv::Vec3d> rvecs;
    std::vector<cv::Vec3d> tvecs;

    // OpenCV returns T_Cam_Tag: p_cam = R_cam_tag * p_tag + t_cam_tag.
    cv::aruco::estimatePoseSingleMarkers(
      selected_corners, static_cast<float>(marker_size_),
      camera_matrix, dist_coeffs, rvecs, tvecs, cv::noArray(), estimate_params_);

    cv::Mat rotation_mat;
    cv::Rodrigues(rvecs.front(), rotation_mat);

    cv::Matx33d rotation;
    for (int row = 0; row < 3; ++row) {
      for (int col = 0; col < 3; ++col) {
        rotation(row, col) = rotation_mat.at<double>(row, col);
      }
    }

    const cv::Matx44d cam_tag_transform = makeTransform(rotation, tvecs.front());

    // User supplies T_Tag_Cube. Composition gives T_Cam_Cube:
    // p_cam = T_Cam_Tag * T_Tag_Cube * p_cube.
    return cam_tag_transform * tag_to_cube_transform_;
  }

  void publishPose(
    const builtin_interfaces::msg::Time & stamp,
    const cv::Matx44d & cam_cube_transform)
  {
    const tf2::Quaternion q = quaternionFromRotation(cam_cube_transform);

    cube_imu_calibration::msg::CubePose pose_msg;
    pose_msg.header.stamp = stamp;
    pose_msg.header.frame_id = camera_frame_;
    pose_msg.translation.x = cam_cube_transform(0, 3);
    pose_msg.translation.y = cam_cube_transform(1, 3);
    pose_msg.translation.z = cam_cube_transform(2, 3);
    pose_msg.orientation.x = q.x();
    pose_msg.orientation.y = q.y();
    pose_msg.orientation.z = q.z();
    pose_msg.orientation.w = q.w();

    for (int row = 0; row < 4; ++row) {
      for (int col = 0; col < 4; ++col) {
        pose_msg.transform[static_cast<size_t>(row * 4 + col)] = cam_cube_transform(row, col);
      }
    }
    pose_pub_->publish(pose_msg);
    writePoseCsv(stamp, cam_cube_transform);

    if (publish_tf_) {
      geometry_msgs::msg::TransformStamped tf_msg;
      tf_msg.header = pose_msg.header;
      tf_msg.child_frame_id = cube_frame_;
      tf_msg.transform.translation.x = pose_msg.translation.x;
      tf_msg.transform.translation.y = pose_msg.translation.y;
      tf_msg.transform.translation.z = pose_msg.translation.z;
      tf_msg.transform.rotation = pose_msg.orientation;
      tf_broadcaster_->sendTransform(tf_msg);
    }
  }

  void imageCallback(const sensor_msgs::msg::Image::ConstSharedPtr msg)
  {
    cv::Mat camera_matrix;
    cv::Mat dist_coeffs;
    if (!getCameraParameters(camera_matrix, dist_coeffs)) {
      RCLCPP_WARN_THROTTLE(
        get_logger(), *get_clock(), 5000,
        "Camera intrinsics are not ready; waiting for parameters or CameraInfo topic");
      return;
    }

    cv_bridge::CvImagePtr cv_image;
    try {
      cv_image = cv_bridge::toCvCopy(msg, sensor_msgs::image_encodings::MONO8);
    } catch (const cv_bridge::Exception & e) {
      RCLCPP_ERROR_THROTTLE(
        get_logger(), *get_clock(), 5000,
        "cv_bridge image conversion failed: %s", e.what());
      return;
    }

    std::vector<int> ids;
    std::vector<std::vector<cv::Point2f>> corners;
    std::vector<std::vector<cv::Point2f>> rejected;
    cv::aruco::detectMarkers(cv_image->image, dictionary_, corners, ids, detector_params_, rejected);

    if (ids.empty()) {
      RCLCPP_INFO_THROTTLE(
        get_logger(), *get_clock(), 3000,
        "No AprilTag detected in current image");
      return;
    }

    int selected_id = -1;
    std::vector<std::vector<cv::Point2f>> selected_corners;
    if (!selectTarget(ids, corners, selected_corners, selected_id)) {
      return;
    }

    try {
      const cv::Matx44d cam_cube_transform =
        estimateCamCubeTransform(selected_corners, camera_matrix, dist_coeffs);
      publishPose(msg->header.stamp, cam_cube_transform);

      ++detection_count_;
      if (detection_count_ % static_cast<uint64_t>(log_every_n_detections_) == 1) {
        RCLCPP_INFO(
          get_logger(),
          "Detected AprilTag id=%d, T_Cam_Cube t=[%.6f %.6f %.6f], q_xyzw=[%.6f %.6f %.6f %.6f]",
          selected_id,
          cam_cube_transform(0, 3), cam_cube_transform(1, 3), cam_cube_transform(2, 3),
          quaternionFromRotation(cam_cube_transform).x(),
          quaternionFromRotation(cam_cube_transform).y(),
          quaternionFromRotation(cam_cube_transform).z(),
          quaternionFromRotation(cam_cube_transform).w());
      }
    } catch (const cv::Exception & e) {
      RCLCPP_ERROR_THROTTLE(
        get_logger(), *get_clock(), 5000,
        "AprilTag pose estimation failed: %s", e.what());
    }
  }

  std::string image_topic_;
  std::string camera_info_topic_;
  bool use_camera_info_ {true};
  double marker_size_ {0.08};
  int target_tag_id_ {-1};
  std::string dictionary_name_;
  std::string camera_frame_;
  std::string cube_frame_;
  bool publish_tf_ {true};
  int log_every_n_detections_ {30};
  std::string pose_csv_path_;
  bool pose_csv_append_ {false};

  cv::Ptr<cv::aruco::Dictionary> dictionary_;
  cv::Ptr<cv::aruco::DetectorParameters> detector_params_;
  cv::Ptr<cv::aruco::EstimateParameters> estimate_params_;
  cv::Matx44d tag_to_cube_transform_ {identity44()};

  std::mutex camera_mutex_;
  cv::Mat camera_matrix_;
  cv::Mat dist_coeffs_;
  bool camera_ready_ {false};
  bool logged_camera_info_ {false};

  uint64_t detection_count_ {0};
  std::ofstream pose_csv_;

  rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr image_sub_;
  rclcpp::Subscription<sensor_msgs::msg::CameraInfo>::SharedPtr camera_info_sub_;
  rclcpp::Publisher<cube_imu_calibration::msg::CubePose>::SharedPtr pose_pub_;
  std::unique_ptr<tf2_ros::TransformBroadcaster> tf_broadcaster_;
};

}  // namespace cube_imu_calibration

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<cube_imu_calibration::AprilTagPoseNode>());
  rclcpp::shutdown();
  return 0;
}
