// generated from rosidl_generator_c/resource/idl__description.c.em
// with input from cube_imu_calibration:msg/CubePoseStatus.idl
// generated code does not contain a copyright notice

#include "cube_imu_calibration/msg/detail/cube_pose_status__functions.h"

ROSIDL_GENERATOR_C_PUBLIC_cube_imu_calibration
const rosidl_type_hash_t *
cube_imu_calibration__msg__CubePoseStatus__get_type_hash(
  const rosidl_message_type_support_t * type_support)
{
  (void)type_support;
  static rosidl_type_hash_t hash = {1, {
      0x9d, 0x66, 0x7b, 0x55, 0xfc, 0xf1, 0xd1, 0xf7,
      0x91, 0x56, 0x0f, 0xe6, 0xc0, 0x54, 0x8a, 0x80,
      0xee, 0xb3, 0x51, 0xdc, 0x60, 0xcf, 0xe6, 0x70,
      0x59, 0x3e, 0x34, 0x3c, 0x17, 0xeb, 0x83, 0xc1,
    }};
  return &hash;
}

#include <assert.h>
#include <string.h>

// Include directives for referenced types
#include "builtin_interfaces/msg/detail/time__functions.h"
#include "std_msgs/msg/detail/header__functions.h"

// Hashes for external referenced types
#ifndef NDEBUG
static const rosidl_type_hash_t builtin_interfaces__msg__Time__EXPECTED_HASH = {1, {
    0xb1, 0x06, 0x23, 0x5e, 0x25, 0xa4, 0xc5, 0xed,
    0x35, 0x09, 0x8a, 0xa0, 0xa6, 0x1a, 0x3e, 0xe9,
    0xc9, 0xb1, 0x8d, 0x19, 0x7f, 0x39, 0x8b, 0x0e,
    0x42, 0x06, 0xce, 0xa9, 0xac, 0xf9, 0xc1, 0x97,
  }};
static const rosidl_type_hash_t std_msgs__msg__Header__EXPECTED_HASH = {1, {
    0xf4, 0x9f, 0xb3, 0xae, 0x2c, 0xf0, 0x70, 0xf7,
    0x93, 0x64, 0x5f, 0xf7, 0x49, 0x68, 0x3a, 0xc6,
    0xb0, 0x62, 0x03, 0xe4, 0x1c, 0x89, 0x1e, 0x17,
    0x70, 0x1b, 0x1c, 0xb5, 0x97, 0xce, 0x6a, 0x01,
  }};
#endif

static char cube_imu_calibration__msg__CubePoseStatus__TYPE_NAME[] = "cube_imu_calibration/msg/CubePoseStatus";
static char builtin_interfaces__msg__Time__TYPE_NAME[] = "builtin_interfaces/msg/Time";
static char std_msgs__msg__Header__TYPE_NAME[] = "std_msgs/msg/Header";

// Define type names, field names, and default values
static char cube_imu_calibration__msg__CubePoseStatus__FIELD_NAME__header[] = "header";
static char cube_imu_calibration__msg__CubePoseStatus__FIELD_NAME__pose_valid[] = "pose_valid";
static char cube_imu_calibration__msg__CubePoseStatus__FIELD_NAME__tag_count[] = "tag_count";
static char cube_imu_calibration__msg__CubePoseStatus__FIELD_NAME__tag_ids[] = "tag_ids";
static char cube_imu_calibration__msg__CubePoseStatus__FIELD_NAME__reproj_error[] = "reproj_error";
static char cube_imu_calibration__msg__CubePoseStatus__FIELD_NAME__message[] = "message";

static rosidl_runtime_c__type_description__Field cube_imu_calibration__msg__CubePoseStatus__FIELDS[] = {
  {
    {cube_imu_calibration__msg__CubePoseStatus__FIELD_NAME__header, 6, 6},
    {
      rosidl_runtime_c__type_description__FieldType__FIELD_TYPE_NESTED_TYPE,
      0,
      0,
      {std_msgs__msg__Header__TYPE_NAME, 19, 19},
    },
    {NULL, 0, 0},
  },
  {
    {cube_imu_calibration__msg__CubePoseStatus__FIELD_NAME__pose_valid, 10, 10},
    {
      rosidl_runtime_c__type_description__FieldType__FIELD_TYPE_BOOLEAN,
      0,
      0,
      {NULL, 0, 0},
    },
    {NULL, 0, 0},
  },
  {
    {cube_imu_calibration__msg__CubePoseStatus__FIELD_NAME__tag_count, 9, 9},
    {
      rosidl_runtime_c__type_description__FieldType__FIELD_TYPE_UINT32,
      0,
      0,
      {NULL, 0, 0},
    },
    {NULL, 0, 0},
  },
  {
    {cube_imu_calibration__msg__CubePoseStatus__FIELD_NAME__tag_ids, 7, 7},
    {
      rosidl_runtime_c__type_description__FieldType__FIELD_TYPE_INT32_UNBOUNDED_SEQUENCE,
      0,
      0,
      {NULL, 0, 0},
    },
    {NULL, 0, 0},
  },
  {
    {cube_imu_calibration__msg__CubePoseStatus__FIELD_NAME__reproj_error, 12, 12},
    {
      rosidl_runtime_c__type_description__FieldType__FIELD_TYPE_DOUBLE,
      0,
      0,
      {NULL, 0, 0},
    },
    {NULL, 0, 0},
  },
  {
    {cube_imu_calibration__msg__CubePoseStatus__FIELD_NAME__message, 7, 7},
    {
      rosidl_runtime_c__type_description__FieldType__FIELD_TYPE_STRING,
      0,
      0,
      {NULL, 0, 0},
    },
    {NULL, 0, 0},
  },
};

static rosidl_runtime_c__type_description__IndividualTypeDescription cube_imu_calibration__msg__CubePoseStatus__REFERENCED_TYPE_DESCRIPTIONS[] = {
  {
    {builtin_interfaces__msg__Time__TYPE_NAME, 27, 27},
    {NULL, 0, 0},
  },
  {
    {std_msgs__msg__Header__TYPE_NAME, 19, 19},
    {NULL, 0, 0},
  },
};

const rosidl_runtime_c__type_description__TypeDescription *
cube_imu_calibration__msg__CubePoseStatus__get_type_description(
  const rosidl_message_type_support_t * type_support)
{
  (void)type_support;
  static bool constructed = false;
  static const rosidl_runtime_c__type_description__TypeDescription description = {
    {
      {cube_imu_calibration__msg__CubePoseStatus__TYPE_NAME, 39, 39},
      {cube_imu_calibration__msg__CubePoseStatus__FIELDS, 6, 6},
    },
    {cube_imu_calibration__msg__CubePoseStatus__REFERENCED_TYPE_DESCRIPTIONS, 2, 2},
  };
  if (!constructed) {
    assert(0 == memcmp(&builtin_interfaces__msg__Time__EXPECTED_HASH, builtin_interfaces__msg__Time__get_type_hash(NULL), sizeof(rosidl_type_hash_t)));
    description.referenced_type_descriptions.data[0].fields = builtin_interfaces__msg__Time__get_type_description(NULL)->type_description.fields;
    assert(0 == memcmp(&std_msgs__msg__Header__EXPECTED_HASH, std_msgs__msg__Header__get_type_hash(NULL), sizeof(rosidl_type_hash_t)));
    description.referenced_type_descriptions.data[1].fields = std_msgs__msg__Header__get_type_description(NULL)->type_description.fields;
    constructed = true;
  }
  return &description;
}

static char toplevel_type_raw_source[] =
  "std_msgs/Header header\n"
  "\n"
  "# Quality report for the Cube pose estimated from AprilTag detections.\n"
  "bool pose_valid\n"
  "uint32 tag_count\n"
  "int32[] tag_ids\n"
  "float64 reproj_error\n"
  "string message";

static char msg_encoding[] = "msg";

// Define all individual source functions

const rosidl_runtime_c__type_description__TypeSource *
cube_imu_calibration__msg__CubePoseStatus__get_individual_type_description_source(
  const rosidl_message_type_support_t * type_support)
{
  (void)type_support;
  static const rosidl_runtime_c__type_description__TypeSource source = {
    {cube_imu_calibration__msg__CubePoseStatus__TYPE_NAME, 39, 39},
    {msg_encoding, 3, 3},
    {toplevel_type_raw_source, 180, 180},
  };
  return &source;
}

const rosidl_runtime_c__type_description__TypeSource__Sequence *
cube_imu_calibration__msg__CubePoseStatus__get_type_description_sources(
  const rosidl_message_type_support_t * type_support)
{
  (void)type_support;
  static rosidl_runtime_c__type_description__TypeSource sources[3];
  static const rosidl_runtime_c__type_description__TypeSource__Sequence source_sequence = {sources, 3, 3};
  static bool constructed = false;
  if (!constructed) {
    sources[0] = *cube_imu_calibration__msg__CubePoseStatus__get_individual_type_description_source(NULL),
    sources[1] = *builtin_interfaces__msg__Time__get_individual_type_description_source(NULL);
    sources[2] = *std_msgs__msg__Header__get_individual_type_description_source(NULL);
    constructed = true;
  }
  return &source_sequence;
}
