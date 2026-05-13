#[cfg(feature = "serde")]
use serde::{Deserialize, Serialize};



// Corresponds to cube_imu_calibration__msg__CubePose

// This struct is not documented.
#[allow(missing_docs)]

#[cfg_attr(feature = "serde", derive(Deserialize, Serialize))]
#[derive(Clone, Debug, PartialEq, PartialOrd)]
pub struct CubePose {

    // This member is not documented.
    #[allow(missing_docs)]
    pub header: std_msgs::msg::Header,

    /// Pose of cube_link expressed in the configured camera frame.
    pub translation: geometry_msgs::msg::Vector3,


    // This member is not documented.
    #[allow(missing_docs)]
    pub orientation: geometry_msgs::msg::Quaternion,

    /// Row-major 4x4 homogeneous transform T_Cam_Cube.
    pub transform: [f64; 16],

}



impl Default for CubePose {
  fn default() -> Self {
    <Self as rosidl_runtime_rs::Message>::from_rmw_message(super::msg::rmw::CubePose::default())
  }
}

impl rosidl_runtime_rs::Message for CubePose {
  type RmwMsg = super::msg::rmw::CubePose;

  fn into_rmw_message(msg_cow: std::borrow::Cow<'_, Self>) -> std::borrow::Cow<'_, Self::RmwMsg> {
    match msg_cow {
      std::borrow::Cow::Owned(msg) => std::borrow::Cow::Owned(Self::RmwMsg {
        header: std_msgs::msg::Header::into_rmw_message(std::borrow::Cow::Owned(msg.header)).into_owned(),
        translation: geometry_msgs::msg::Vector3::into_rmw_message(std::borrow::Cow::Owned(msg.translation)).into_owned(),
        orientation: geometry_msgs::msg::Quaternion::into_rmw_message(std::borrow::Cow::Owned(msg.orientation)).into_owned(),
        transform: msg.transform,
      }),
      std::borrow::Cow::Borrowed(msg) => std::borrow::Cow::Owned(Self::RmwMsg {
        header: std_msgs::msg::Header::into_rmw_message(std::borrow::Cow::Borrowed(&msg.header)).into_owned(),
        translation: geometry_msgs::msg::Vector3::into_rmw_message(std::borrow::Cow::Borrowed(&msg.translation)).into_owned(),
        orientation: geometry_msgs::msg::Quaternion::into_rmw_message(std::borrow::Cow::Borrowed(&msg.orientation)).into_owned(),
        transform: msg.transform,
      })
    }
  }

  fn from_rmw_message(msg: Self::RmwMsg) -> Self {
    Self {
      header: std_msgs::msg::Header::from_rmw_message(msg.header),
      translation: geometry_msgs::msg::Vector3::from_rmw_message(msg.translation),
      orientation: geometry_msgs::msg::Quaternion::from_rmw_message(msg.orientation),
      transform: msg.transform,
    }
  }
}


// Corresponds to cube_imu_calibration__msg__CubePoseStatus

// This struct is not documented.
#[allow(missing_docs)]

#[cfg_attr(feature = "serde", derive(Deserialize, Serialize))]
#[derive(Clone, Debug, PartialEq, PartialOrd)]
pub struct CubePoseStatus {

    // This member is not documented.
    #[allow(missing_docs)]
    pub header: std_msgs::msg::Header,

    /// Quality report for the Cube pose estimated from AprilTag detections.
    pub pose_valid: bool,


    // This member is not documented.
    #[allow(missing_docs)]
    pub tag_count: u32,


    // This member is not documented.
    #[allow(missing_docs)]
    pub tag_ids: Vec<i32>,


    // This member is not documented.
    #[allow(missing_docs)]
    pub reproj_error: f64,


    // This member is not documented.
    #[allow(missing_docs)]
    pub message: std::string::String,

}



impl Default for CubePoseStatus {
  fn default() -> Self {
    <Self as rosidl_runtime_rs::Message>::from_rmw_message(super::msg::rmw::CubePoseStatus::default())
  }
}

impl rosidl_runtime_rs::Message for CubePoseStatus {
  type RmwMsg = super::msg::rmw::CubePoseStatus;

  fn into_rmw_message(msg_cow: std::borrow::Cow<'_, Self>) -> std::borrow::Cow<'_, Self::RmwMsg> {
    match msg_cow {
      std::borrow::Cow::Owned(msg) => std::borrow::Cow::Owned(Self::RmwMsg {
        header: std_msgs::msg::Header::into_rmw_message(std::borrow::Cow::Owned(msg.header)).into_owned(),
        pose_valid: msg.pose_valid,
        tag_count: msg.tag_count,
        tag_ids: msg.tag_ids.into(),
        reproj_error: msg.reproj_error,
        message: msg.message.as_str().into(),
      }),
      std::borrow::Cow::Borrowed(msg) => std::borrow::Cow::Owned(Self::RmwMsg {
        header: std_msgs::msg::Header::into_rmw_message(std::borrow::Cow::Borrowed(&msg.header)).into_owned(),
      pose_valid: msg.pose_valid,
      tag_count: msg.tag_count,
        tag_ids: msg.tag_ids.as_slice().into(),
      reproj_error: msg.reproj_error,
        message: msg.message.as_str().into(),
      })
    }
  }

  fn from_rmw_message(msg: Self::RmwMsg) -> Self {
    Self {
      header: std_msgs::msg::Header::from_rmw_message(msg.header),
      pose_valid: msg.pose_valid,
      tag_count: msg.tag_count,
      tag_ids: msg.tag_ids
          .into_iter()
          .collect(),
      reproj_error: msg.reproj_error,
      message: msg.message.to_string(),
    }
  }
}


// Corresponds to cube_imu_calibration__msg__RecorderStatus

// This struct is not documented.
#[allow(missing_docs)]

#[cfg_attr(feature = "serde", derive(Deserialize, Serialize))]
#[derive(Clone, Debug, PartialEq, PartialOrd)]
pub struct RecorderStatus {

    // This member is not documented.
    #[allow(missing_docs)]
    pub header: std_msgs::msg::Header,


    // This member is not documented.
    #[allow(missing_docs)]
    pub recording: bool,


    // This member is not documented.
    #[allow(missing_docs)]
    pub bag_path: std::string::String,


    // This member is not documented.
    #[allow(missing_docs)]
    pub image_count: u64,


    // This member is not documented.
    #[allow(missing_docs)]
    pub imu_count: u64,


    // This member is not documented.
    #[allow(missing_docs)]
    pub camera_info_count: u64,


    // This member is not documented.
    #[allow(missing_docs)]
    pub cube_pose_count: u64,


    // This member is not documented.
    #[allow(missing_docs)]
    pub cube_pose_status_count: u64,


    // This member is not documented.
    #[allow(missing_docs)]
    pub image_publishers: u32,


    // This member is not documented.
    #[allow(missing_docs)]
    pub imu_publishers: u32,


    // This member is not documented.
    #[allow(missing_docs)]
    pub camera_info_publishers: u32,


    // This member is not documented.
    #[allow(missing_docs)]
    pub cube_pose_publishers: u32,


    // This member is not documented.
    #[allow(missing_docs)]
    pub cube_pose_status_publishers: u32,

    /// Recording progress. target_duration_sec <= 0 means manual/unknown-length recording.
    pub elapsed_sec: f64,


    // This member is not documented.
    #[allow(missing_docs)]
    pub target_duration_sec: f64,


    // This member is not documented.
    #[allow(missing_docs)]
    pub progress_ratio: f64,

}



impl Default for RecorderStatus {
  fn default() -> Self {
    <Self as rosidl_runtime_rs::Message>::from_rmw_message(super::msg::rmw::RecorderStatus::default())
  }
}

impl rosidl_runtime_rs::Message for RecorderStatus {
  type RmwMsg = super::msg::rmw::RecorderStatus;

  fn into_rmw_message(msg_cow: std::borrow::Cow<'_, Self>) -> std::borrow::Cow<'_, Self::RmwMsg> {
    match msg_cow {
      std::borrow::Cow::Owned(msg) => std::borrow::Cow::Owned(Self::RmwMsg {
        header: std_msgs::msg::Header::into_rmw_message(std::borrow::Cow::Owned(msg.header)).into_owned(),
        recording: msg.recording,
        bag_path: msg.bag_path.as_str().into(),
        image_count: msg.image_count,
        imu_count: msg.imu_count,
        camera_info_count: msg.camera_info_count,
        cube_pose_count: msg.cube_pose_count,
        cube_pose_status_count: msg.cube_pose_status_count,
        image_publishers: msg.image_publishers,
        imu_publishers: msg.imu_publishers,
        camera_info_publishers: msg.camera_info_publishers,
        cube_pose_publishers: msg.cube_pose_publishers,
        cube_pose_status_publishers: msg.cube_pose_status_publishers,
        elapsed_sec: msg.elapsed_sec,
        target_duration_sec: msg.target_duration_sec,
        progress_ratio: msg.progress_ratio,
      }),
      std::borrow::Cow::Borrowed(msg) => std::borrow::Cow::Owned(Self::RmwMsg {
        header: std_msgs::msg::Header::into_rmw_message(std::borrow::Cow::Borrowed(&msg.header)).into_owned(),
      recording: msg.recording,
        bag_path: msg.bag_path.as_str().into(),
      image_count: msg.image_count,
      imu_count: msg.imu_count,
      camera_info_count: msg.camera_info_count,
      cube_pose_count: msg.cube_pose_count,
      cube_pose_status_count: msg.cube_pose_status_count,
      image_publishers: msg.image_publishers,
      imu_publishers: msg.imu_publishers,
      camera_info_publishers: msg.camera_info_publishers,
      cube_pose_publishers: msg.cube_pose_publishers,
      cube_pose_status_publishers: msg.cube_pose_status_publishers,
      elapsed_sec: msg.elapsed_sec,
      target_duration_sec: msg.target_duration_sec,
      progress_ratio: msg.progress_ratio,
      })
    }
  }

  fn from_rmw_message(msg: Self::RmwMsg) -> Self {
    Self {
      header: std_msgs::msg::Header::from_rmw_message(msg.header),
      recording: msg.recording,
      bag_path: msg.bag_path.to_string(),
      image_count: msg.image_count,
      imu_count: msg.imu_count,
      camera_info_count: msg.camera_info_count,
      cube_pose_count: msg.cube_pose_count,
      cube_pose_status_count: msg.cube_pose_status_count,
      image_publishers: msg.image_publishers,
      imu_publishers: msg.imu_publishers,
      camera_info_publishers: msg.camera_info_publishers,
      cube_pose_publishers: msg.cube_pose_publishers,
      cube_pose_status_publishers: msg.cube_pose_status_publishers,
      elapsed_sec: msg.elapsed_sec,
      target_duration_sec: msg.target_duration_sec,
      progress_ratio: msg.progress_ratio,
    }
  }
}


