std_msgs/Header header

bool recording
string bag_path

uint64 image_count
uint64 imu_count
uint64 camera_info_count
uint64 cube_pose_count
uint64 cube_pose_status_count

uint32 image_publishers
uint32 imu_publishers
uint32 camera_info_publishers
uint32 cube_pose_publishers
uint32 cube_pose_status_publishers

# Recording progress. target_duration_sec <= 0 means manual/unknown-length recording.
float64 elapsed_sec
float64 target_duration_sec
float64 progress_ratio
