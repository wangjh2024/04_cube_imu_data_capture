std_msgs/Header header

# Pose of cube_link expressed in the configured camera frame.
geometry_msgs/Vector3 translation
geometry_msgs/Quaternion orientation

# Row-major 4x4 homogeneous transform T_Cam_Cube.
float64[16] transform
