#[cfg(feature = "serde")]
use serde::{Deserialize, Serialize};


#[link(name = "cube_imu_calibration__rosidl_typesupport_c")]
extern "C" {
    fn rosidl_typesupport_c__get_message_type_support_handle__cube_imu_calibration__msg__CubePose() -> *const std::ffi::c_void;
}

#[link(name = "cube_imu_calibration__rosidl_generator_c")]
extern "C" {
    fn cube_imu_calibration__msg__CubePose__init(msg: *mut CubePose) -> bool;
    fn cube_imu_calibration__msg__CubePose__Sequence__init(seq: *mut rosidl_runtime_rs::Sequence<CubePose>, size: usize) -> bool;
    fn cube_imu_calibration__msg__CubePose__Sequence__fini(seq: *mut rosidl_runtime_rs::Sequence<CubePose>);
    fn cube_imu_calibration__msg__CubePose__Sequence__copy(in_seq: &rosidl_runtime_rs::Sequence<CubePose>, out_seq: *mut rosidl_runtime_rs::Sequence<CubePose>) -> bool;
}

// Corresponds to cube_imu_calibration__msg__CubePose
#[cfg_attr(feature = "serde", derive(Deserialize, Serialize))]


// This struct is not documented.
#[allow(missing_docs)]

#[repr(C)]
#[derive(Clone, Debug, PartialEq, PartialOrd)]
pub struct CubePose {

    // This member is not documented.
    #[allow(missing_docs)]
    pub header: std_msgs::msg::rmw::Header,

    /// Pose of cube_link expressed in the configured camera frame.
    pub translation: geometry_msgs::msg::rmw::Vector3,


    // This member is not documented.
    #[allow(missing_docs)]
    pub orientation: geometry_msgs::msg::rmw::Quaternion,

    /// Row-major 4x4 homogeneous transform T_Cam_Cube.
    pub transform: [f64; 16],

}



impl Default for CubePose {
  fn default() -> Self {
    unsafe {
      let mut msg = std::mem::zeroed();
      if !cube_imu_calibration__msg__CubePose__init(&mut msg as *mut _) {
        panic!("Call to cube_imu_calibration__msg__CubePose__init() failed");
      }
      msg
    }
  }
}

impl rosidl_runtime_rs::SequenceAlloc for CubePose {
  fn sequence_init(seq: &mut rosidl_runtime_rs::Sequence<Self>, size: usize) -> bool {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { cube_imu_calibration__msg__CubePose__Sequence__init(seq as *mut _, size) }
  }
  fn sequence_fini(seq: &mut rosidl_runtime_rs::Sequence<Self>) {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { cube_imu_calibration__msg__CubePose__Sequence__fini(seq as *mut _) }
  }
  fn sequence_copy(in_seq: &rosidl_runtime_rs::Sequence<Self>, out_seq: &mut rosidl_runtime_rs::Sequence<Self>) -> bool {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { cube_imu_calibration__msg__CubePose__Sequence__copy(in_seq, out_seq as *mut _) }
  }
}

impl rosidl_runtime_rs::Message for CubePose {
  type RmwMsg = Self;
  fn into_rmw_message(msg_cow: std::borrow::Cow<'_, Self>) -> std::borrow::Cow<'_, Self::RmwMsg> { msg_cow }
  fn from_rmw_message(msg: Self::RmwMsg) -> Self { msg }
}

impl rosidl_runtime_rs::RmwMessage for CubePose where Self: Sized {
  const TYPE_NAME: &'static str = "cube_imu_calibration/msg/CubePose";
  fn get_type_support() -> *const std::ffi::c_void {
    // SAFETY: No preconditions for this function.
    unsafe { rosidl_typesupport_c__get_message_type_support_handle__cube_imu_calibration__msg__CubePose() }
  }
}


#[link(name = "cube_imu_calibration__rosidl_typesupport_c")]
extern "C" {
    fn rosidl_typesupport_c__get_message_type_support_handle__cube_imu_calibration__msg__CubePoseStatus() -> *const std::ffi::c_void;
}

#[link(name = "cube_imu_calibration__rosidl_generator_c")]
extern "C" {
    fn cube_imu_calibration__msg__CubePoseStatus__init(msg: *mut CubePoseStatus) -> bool;
    fn cube_imu_calibration__msg__CubePoseStatus__Sequence__init(seq: *mut rosidl_runtime_rs::Sequence<CubePoseStatus>, size: usize) -> bool;
    fn cube_imu_calibration__msg__CubePoseStatus__Sequence__fini(seq: *mut rosidl_runtime_rs::Sequence<CubePoseStatus>);
    fn cube_imu_calibration__msg__CubePoseStatus__Sequence__copy(in_seq: &rosidl_runtime_rs::Sequence<CubePoseStatus>, out_seq: *mut rosidl_runtime_rs::Sequence<CubePoseStatus>) -> bool;
}

// Corresponds to cube_imu_calibration__msg__CubePoseStatus
#[cfg_attr(feature = "serde", derive(Deserialize, Serialize))]


// This struct is not documented.
#[allow(missing_docs)]

#[repr(C)]
#[derive(Clone, Debug, PartialEq, PartialOrd)]
pub struct CubePoseStatus {

    // This member is not documented.
    #[allow(missing_docs)]
    pub header: std_msgs::msg::rmw::Header,

    /// Quality report for the Cube pose estimated from AprilTag detections.
    pub pose_valid: bool,


    // This member is not documented.
    #[allow(missing_docs)]
    pub tag_count: u32,


    // This member is not documented.
    #[allow(missing_docs)]
    pub tag_ids: rosidl_runtime_rs::Sequence<i32>,


    // This member is not documented.
    #[allow(missing_docs)]
    pub reproj_error: f64,


    // This member is not documented.
    #[allow(missing_docs)]
    pub message: rosidl_runtime_rs::String,

}



impl Default for CubePoseStatus {
  fn default() -> Self {
    unsafe {
      let mut msg = std::mem::zeroed();
      if !cube_imu_calibration__msg__CubePoseStatus__init(&mut msg as *mut _) {
        panic!("Call to cube_imu_calibration__msg__CubePoseStatus__init() failed");
      }
      msg
    }
  }
}

impl rosidl_runtime_rs::SequenceAlloc for CubePoseStatus {
  fn sequence_init(seq: &mut rosidl_runtime_rs::Sequence<Self>, size: usize) -> bool {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { cube_imu_calibration__msg__CubePoseStatus__Sequence__init(seq as *mut _, size) }
  }
  fn sequence_fini(seq: &mut rosidl_runtime_rs::Sequence<Self>) {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { cube_imu_calibration__msg__CubePoseStatus__Sequence__fini(seq as *mut _) }
  }
  fn sequence_copy(in_seq: &rosidl_runtime_rs::Sequence<Self>, out_seq: &mut rosidl_runtime_rs::Sequence<Self>) -> bool {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { cube_imu_calibration__msg__CubePoseStatus__Sequence__copy(in_seq, out_seq as *mut _) }
  }
}

impl rosidl_runtime_rs::Message for CubePoseStatus {
  type RmwMsg = Self;
  fn into_rmw_message(msg_cow: std::borrow::Cow<'_, Self>) -> std::borrow::Cow<'_, Self::RmwMsg> { msg_cow }
  fn from_rmw_message(msg: Self::RmwMsg) -> Self { msg }
}

impl rosidl_runtime_rs::RmwMessage for CubePoseStatus where Self: Sized {
  const TYPE_NAME: &'static str = "cube_imu_calibration/msg/CubePoseStatus";
  fn get_type_support() -> *const std::ffi::c_void {
    // SAFETY: No preconditions for this function.
    unsafe { rosidl_typesupport_c__get_message_type_support_handle__cube_imu_calibration__msg__CubePoseStatus() }
  }
}


#[link(name = "cube_imu_calibration__rosidl_typesupport_c")]
extern "C" {
    fn rosidl_typesupport_c__get_message_type_support_handle__cube_imu_calibration__msg__RecorderStatus() -> *const std::ffi::c_void;
}

#[link(name = "cube_imu_calibration__rosidl_generator_c")]
extern "C" {
    fn cube_imu_calibration__msg__RecorderStatus__init(msg: *mut RecorderStatus) -> bool;
    fn cube_imu_calibration__msg__RecorderStatus__Sequence__init(seq: *mut rosidl_runtime_rs::Sequence<RecorderStatus>, size: usize) -> bool;
    fn cube_imu_calibration__msg__RecorderStatus__Sequence__fini(seq: *mut rosidl_runtime_rs::Sequence<RecorderStatus>);
    fn cube_imu_calibration__msg__RecorderStatus__Sequence__copy(in_seq: &rosidl_runtime_rs::Sequence<RecorderStatus>, out_seq: *mut rosidl_runtime_rs::Sequence<RecorderStatus>) -> bool;
}

// Corresponds to cube_imu_calibration__msg__RecorderStatus
#[cfg_attr(feature = "serde", derive(Deserialize, Serialize))]


// This struct is not documented.
#[allow(missing_docs)]

#[repr(C)]
#[derive(Clone, Debug, PartialEq, PartialOrd)]
pub struct RecorderStatus {

    // This member is not documented.
    #[allow(missing_docs)]
    pub header: std_msgs::msg::rmw::Header,


    // This member is not documented.
    #[allow(missing_docs)]
    pub recording: bool,


    // This member is not documented.
    #[allow(missing_docs)]
    pub bag_path: rosidl_runtime_rs::String,


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
    unsafe {
      let mut msg = std::mem::zeroed();
      if !cube_imu_calibration__msg__RecorderStatus__init(&mut msg as *mut _) {
        panic!("Call to cube_imu_calibration__msg__RecorderStatus__init() failed");
      }
      msg
    }
  }
}

impl rosidl_runtime_rs::SequenceAlloc for RecorderStatus {
  fn sequence_init(seq: &mut rosidl_runtime_rs::Sequence<Self>, size: usize) -> bool {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { cube_imu_calibration__msg__RecorderStatus__Sequence__init(seq as *mut _, size) }
  }
  fn sequence_fini(seq: &mut rosidl_runtime_rs::Sequence<Self>) {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { cube_imu_calibration__msg__RecorderStatus__Sequence__fini(seq as *mut _) }
  }
  fn sequence_copy(in_seq: &rosidl_runtime_rs::Sequence<Self>, out_seq: &mut rosidl_runtime_rs::Sequence<Self>) -> bool {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { cube_imu_calibration__msg__RecorderStatus__Sequence__copy(in_seq, out_seq as *mut _) }
  }
}

impl rosidl_runtime_rs::Message for RecorderStatus {
  type RmwMsg = Self;
  fn into_rmw_message(msg_cow: std::borrow::Cow<'_, Self>) -> std::borrow::Cow<'_, Self::RmwMsg> { msg_cow }
  fn from_rmw_message(msg: Self::RmwMsg) -> Self { msg }
}

impl rosidl_runtime_rs::RmwMessage for RecorderStatus where Self: Sized {
  const TYPE_NAME: &'static str = "cube_imu_calibration/msg/RecorderStatus";
  fn get_type_support() -> *const std::ffi::c_void {
    // SAFETY: No preconditions for this function.
    unsafe { rosidl_typesupport_c__get_message_type_support_handle__cube_imu_calibration__msg__RecorderStatus() }
  }
}


