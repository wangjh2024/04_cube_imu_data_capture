cmake_minimum_required(VERSION 3.16)
project(cube_imu_calibration)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(std_msgs REQUIRED)
find_package(std_srvs REQUIRED)
find_package(cv_bridge REQUIRED)
find_package(rosbag2_cpp REQUIRED)
find_package(rosbag2_storage REQUIRED)
find_package(rosidl_default_generators REQUIRED)
find_package(OpenCV REQUIRED COMPONENTS core imgproc aruco)
find_package(Eigen3 REQUIRED)
find_package(Threads REQUIRED)

set(DEFAULT_ORBBEC_SDK_ROOT
  "/home/wangjh/code/01历史代码/00历史代码/orbslam3_light/hardware/camera/orbbec_gemini_335/linux/third_party/pyorbbecsdk/sdk"
)
set(ORBBEC_SDK_ROOT "${DEFAULT_ORBBEC_SDK_ROOT}" CACHE PATH "Orbbec SDK root containing include/ and lib/")
set(ORBBEC_INCLUDE_DIR "${ORBBEC_SDK_ROOT}/include" CACHE PATH "Orbbec SDK include directory")
set(ORBBEC_LIB_DIR "${ORBBEC_SDK_ROOT}/lib/linux_x64" CACHE PATH "Orbbec SDK library directory")
set(ORBBEC_SDK_LIBRARY "${ORBBEC_LIB_DIR}/libOrbbecSDK.so.2.7.6" CACHE FILEPATH "Orbbec SDK shared library")

rosidl_generate_interfaces(${PROJECT_NAME}
  "msg/CubePose.msg"
  "msg/CubePoseStatus.msg"
  "msg/RecorderStatus.msg"
  DEPENDENCIES std_msgs geometry_msgs
)

rosidl_get_typesupport_target(cpp_typesupport_target ${PROJECT_NAME} "rosidl_typesupport_cpp")

add_executable(data_recorder_node src/data_recorder_node.cpp)
ament_target_dependencies(data_recorder_node
  rclcpp
  sensor_msgs
  std_msgs
  std_srvs
  rosbag2_cpp
  rosbag2_storage
)
target_link_libraries(data_recorder_node "${cpp_typesupport_target}")

set(extra_runtime_targets "")
if(EXISTS "${ORBBEC_INCLUDE_DIR}/libobsensor/ObSensor.hpp" AND EXISTS "${ORBBEC_SDK_LIBRARY}")
  add_library(cube_imu_direct_sensor SHARED
    src/direct_sensor_reader.cpp
    src/serial_imu_reader.cpp
  )
  target_include_directories(cube_imu_direct_sensor PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${ORBBEC_INCLUDE_DIR}"
    ${OpenCV_INCLUDE_DIRS}
  )
  target_link_libraries(cube_imu_direct_sensor
    Eigen3::Eigen
    ${OpenCV_LIBS}
    "${ORBBEC_SDK_LIBRARY}"
    Threads::Threads
  )
  set_target_properties(cube_imu_direct_sensor PROPERTIES
    BUILD_RPATH "${ORBBEC_LIB_DIR}"
    INSTALL_RPATH "${ORBBEC_LIB_DIR}"
  )
  list(APPEND extra_runtime_targets cube_imu_direct_sensor)

  add_executable(real_sensor_node
    src/real_sensor_node.cpp
    src/serial_imu_reader.cpp
  )
  ament_target_dependencies(real_sensor_node
    rclcpp
    sensor_msgs
    std_msgs
    cv_bridge
  )
  target_include_directories(real_sensor_node PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${ORBBEC_INCLUDE_DIR}"
    ${OpenCV_INCLUDE_DIRS}
  )
  target_link_libraries(real_sensor_node
    Eigen3::Eigen
    ${OpenCV_LIBS}
    "${ORBBEC_SDK_LIBRARY}"
    Threads::Threads
  )
  set_target_properties(real_sensor_node PROPERTIES
    BUILD_RPATH "${ORBBEC_LIB_DIR}"
    INSTALL_RPATH "${ORBBEC_LIB_DIR}"
  )
  list(APPEND extra_runtime_targets real_sensor_node)
else()
  message(WARNING "Orbbec SDK not found; real_sensor_node will not be built. Set ORBBEC_SDK_ROOT if needed.")
endif()

install(TARGETS
  data_recorder_node
  ${extra_runtime_targets}
  DESTINATION lib/${PROJECT_NAME}
)

install(PROGRAMS
  src/scripts/calibration_gui.py
  src/scripts/kalibr_workflow_check.py
  src/scripts/solve_extrinsics.py
  src/scripts/pose_imu_calibrator.py
  DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY
  launch
  config
  DESTINATION share/${PROJECT_NAME}
)

install(FILES
  README_JAZZY.txt
  DESTINATION share/${PROJECT_NAME}
)

ament_export_dependencies(rosidl_default_runtime)
ament_package()
