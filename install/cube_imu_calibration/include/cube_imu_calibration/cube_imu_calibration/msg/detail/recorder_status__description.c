// generated from rosidl_generator_c/resource/idl__description.c.em
// with input from cube_imu_calibration:msg/RecorderStatus.idl
// generated code does not contain a copyright notice

#include "cube_imu_calibration/msg/detail/recorder_status__functions.h"

ROSIDL_GENERATOR_C_PUBLIC_cube_imu_calibration
const rosidl_type_hash_t *
cube_imu_calibration__msg__RecorderStatus__get_type_hash(
  const rosidl_message_type_support_t * type_support)
{
  (void)type_support;
  static rosidl_type_hash_t hash = {1, {
      0xd9, 0xbf, 0x9d, 0xaf, 0xe4, 0xcf, 0xf7, 0xe2,
      0x89, 0xde, 0x24, 0x5e, 0x4e, 0x31, 0x5b, 0xd8,
      0xe7, 0xd8, 0xc1, 0x4e, 0x60, 0xb6, 0x6a, 0xa9,
      0xb1, 0x72, 0xc9, 0xd5, 0xed, 0xfb, 0xe9, 0x7f,
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

static char cube_imu_calibration__msg__RecorderStatus__TYPE_NAME[] = "cube_imu_calibration/msg/RecorderStatus";
static char builtin_interfaces__msg__Time__TYPE_NAME[] = "builtin_interfaces/msg/Time";
static char std_msgs__msg__Header__TYPE_NAME[] = "std_msgs/msg/Header";

// Define type names, field names, and default values
static char cube_imu_calibration__msg__RecorderStatus__FIELD_NAME__header[] = "header";
static char cube_imu_calibration__msg__RecorderStatus__FIELD_NAME__recording[] = "recording";
static char cube_imu_calibration__msg__RecorderStatus__FIELD_NAME__bag_path[] = "bag_path";
static char cube_imu_calibration__msg__RecorderStatus__FIELD_NAME__image_count[] = "image_count";
static char cube_imu_calibration__msg__RecorderStatus__FIELD_NAME__imu_count[] = "imu_count";
static char cube_imu_calibration__msg__RecorderStatus__FIELD_NAME__camera_info_count[] = "camera_info_count";
static char cube_imu_calibration__msg__RecorderStatus__FIELD_NAME__cube_pose_count[] = "cube_pose_count";
static char cube_imu_calibration__msg__RecorderStatus__FIELD_NAME__cube_pose_status_count[] = "cube_pose_status_count";
static char cube_imu_calibration__msg__RecorderStatus__FIELD_NAME__image_publishers[] = "image_publishers";
static char cube_imu_calibration__msg__RecorderStatus__FIELD_NAME__imu_publishers[] = "imu_publishers";
static char cube_imu_calibration__msg__RecorderStatus__FIELD_NAME__camera_info_publishers[] = "camera_info_publishers";
static char cube_imu_calibration__msg__RecorderStatus__FIELD_NAME__cube_pose_publishers[] = "cube_pose_publishers";
static char cube_imu_calibration__msg__RecorderStatus__FIELD_NAME__cube_pose_status_publishers[] = "cube_pose_status_publishers";
static char cube_imu_calibration__msg__RecorderStatus__FIELD_NAME__elapsed_sec[] = "elapsed_sec";
static char cube_imu_calibration__msg__RecorderStatus__FIELD_NAME__target_duration_sec[] = "target_duration_sec";
static char cube_imu_calibration__msg__RecorderStatus__FIELD_NAME__progress_ratio[] = "progress_ratio";

static rosidl_runtime_c__type_description__Field cube_imu_calibration__msg__RecorderStatus__FIELDS[] = {
  {
    {cube_imu_calibration__msg__RecorderStatus__FIELD_NAME__header, 6, 6},
    {
      rosidl_runtime_c__type_description__FieldType__FIELD_TYPE_NESTED_TYPE,
      0,
      0,
      {std_msgs__msg__Header__TYPE_NAME, 19, 19},
    },
    {NULL, 0, 0},
  },
  {
    {cube_imu_calibration__msg__RecorderStatus__FIELD_NAME__recording, 9, 9},
    {
      rosidl_runtime_c__type_description__FieldType__FIELD_TYPE_BOOLEAN,
      0,
      0,
      {NULL, 0, 0},
    },
    {NULL, 0, 0},
  },
  {
    {cube_imu_calibration__msg__RecorderStatus__FIELD_NAME__bag_path, 8, 8},
    {
      rosidl_runtime_c__type_description__FieldType__FIELD_TYPE_STRING,
      0,
      0,
      {NULL, 0, 0},
    },
    {NULL, 0, 0},
  },
  {
    {cube_imu_calibration__msg__RecorderStatus__FIELD_NAME__image_count, 11, 11},
    {
      rosidl_runtime_c__type_description__FieldType__FIELD_TYPE_UINT64,
      0,
      0,
      {NULL, 0, 0},
    },
    {NULL, 0, 0},
  },
  {
    {cube_imu_calibration__msg__RecorderStatus__FIELD_NAME__imu_count, 9, 9},
    {
      rosidl_runtime_c__type_description__FieldType__FIELD_TYPE_UINT64,
      0,
      0,
      {NULL, 0, 0},
    },
    {NULL, 0, 0},
  },
  {
    {cube_imu_calibration__msg__RecorderStatus__FIELD_NAME__camera_info_count, 17, 17},
    {
      rosidl_runtime_c__type_description__FieldType__FIELD_TYPE_UINT64,
      0,
      0,
      {NULL, 0, 0},
    },
    {NULL, 0, 0},
  },
  {
    {cube_imu_calibration__msg__RecorderStatus__FIELD_NAME__cube_pose_count, 15, 15},
    {
      rosidl_runtime_c__type_description__FieldType__FIELD_TYPE_UINT64,
      0,
      0,
      {NULL, 0, 0},
    },
    {NULL, 0, 0},
  },
  {
    {cube_imu_calibration__msg__RecorderStatus__FIELD_NAME__cube_pose_status_count, 22, 22},
    {
      rosidl_runtime_c__type_description__FieldType__FIELD_TYPE_UINT64,
      0,
      0,
      {NULL, 0, 0},
    },
    {NULL, 0, 0},
  },
  {
    {cube_imu_calibration__msg__RecorderStatus__FIELD_NAME__image_publishers, 16, 16},
    {
      rosidl_runtime_c__type_description__FieldType__FIELD_TYPE_UINT32,
      0,
      0,
      {NULL, 0, 0},
    },
    {NULL, 0, 0},
  },
  {
    {cube_imu_calibration__msg__RecorderStatus__FIELD_NAME__imu_publishers, 14, 14},
    {
      rosidl_runtime_c__type_description__FieldType__FIELD_TYPE_UINT32,
      0,
      0,
      {NULL, 0, 0},
    },
    {NULL, 0, 0},
  },
  {
    {cube_imu_calibration__msg__RecorderStatus__FIELD_NAME__camera_info_publishers, 22, 22},
    {
      rosidl_runtime_c__type_description__FieldType__FIELD_TYPE_UINT32,
      0,
      0,
      {NULL, 0, 0},
    },
    {NULL, 0, 0},
  },
  {
    {cube_imu_calibration__msg__RecorderStatus__FIELD_NAME__cube_pose_publishers, 20, 20},
    {
      rosidl_runtime_c__type_description__FieldType__FIELD_TYPE_UINT32,
      0,
      0,
      {NULL, 0, 0},
    },
    {NULL, 0, 0},
  },
  {
    {cube_imu_calibration__msg__RecorderStatus__FIELD_NAME__cube_pose_status_publishers, 27, 27},
    {
      rosidl_runtime_c__type_description__FieldType__FIELD_TYPE_UINT32,
      0,
      0,
      {NULL, 0, 0},
    },
    {NULL, 0, 0},
  },
  {
    {cube_imu_calibration__msg__RecorderStatus__FIELD_NAME__elapsed_sec, 11, 11},
    {
      rosidl_runtime_c__type_description__FieldType__FIELD_TYPE_DOUBLE,
      0,
      0,
      {NULL, 0, 0},
    },
    {NULL, 0, 0},
  },
  {
    {cube_imu_calibration__msg__RecorderStatus__FIELD_NAME__target_duration_sec, 19, 19},
    {
      rosidl_runtime_c__type_description__FieldType__FIELD_TYPE_DOUBLE,
      0,
      0,
      {NULL, 0, 0},
    },
    {NULL, 0, 0},
  },
  {
    {cube_imu_calibration__msg__RecorderStatus__FIELD_NAME__progress_ratio, 14, 14},
    {
      rosidl_runtime_c__type_description__FieldType__FIELD_TYPE_DOUBLE,
      0,
      0,
      {NULL, 0, 0},
    },
    {NULL, 0, 0},
  },
};

static rosidl_runtime_c__type_description__IndividualTypeDescription cube_imu_calibration__msg__RecorderStatus__REFERENCED_TYPE_DESCRIPTIONS[] = {
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
cube_imu_calibration__msg__RecorderStatus__get_type_description(
  const rosidl_message_type_support_t * type_support)
{
  (void)type_support;
  static bool constructed = false;
  static const rosidl_runtime_c__type_description__TypeDescription description = {
    {
      {cube_imu_calibration__msg__RecorderStatus__TYPE_NAME, 39, 39},
      {cube_imu_calibration__msg__RecorderStatus__FIELDS, 16, 16},
    },
    {cube_imu_calibration__msg__RecorderStatus__REFERENCED_TYPE_DESCRIPTIONS, 2, 2},
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
  "bool recording\n"
  "string bag_path\n"
  "\n"
  "uint64 image_count\n"
  "uint64 imu_count\n"
  "uint64 camera_info_count\n"
  "uint64 cube_pose_count\n"
  "uint64 cube_pose_status_count\n"
  "\n"
  "uint32 image_publishers\n"
  "uint32 imu_publishers\n"
  "uint32 camera_info_publishers\n"
  "uint32 cube_pose_publishers\n"
  "uint32 cube_pose_status_publishers\n"
  "\n"
  "# Recording progress. target_duration_sec <= 0 means manual/unknown-length recording.\n"
  "float64 elapsed_sec\n"
  "float64 target_duration_sec\n"
  "float64 progress_ratio";

static char msg_encoding[] = "msg";

// Define all individual source functions

const rosidl_runtime_c__type_description__TypeSource *
cube_imu_calibration__msg__RecorderStatus__get_individual_type_description_source(
  const rosidl_message_type_support_t * type_support)
{
  (void)type_support;
  static const rosidl_runtime_c__type_description__TypeSource source = {
    {cube_imu_calibration__msg__RecorderStatus__TYPE_NAME, 39, 39},
    {msg_encoding, 3, 3},
    {toplevel_type_raw_source, 468, 468},
  };
  return &source;
}

const rosidl_runtime_c__type_description__TypeSource__Sequence *
cube_imu_calibration__msg__RecorderStatus__get_type_description_sources(
  const rosidl_message_type_support_t * type_support)
{
  (void)type_support;
  static rosidl_runtime_c__type_description__TypeSource sources[3];
  static const rosidl_runtime_c__type_description__TypeSource__Sequence source_sequence = {sources, 3, 3};
  static bool constructed = false;
  if (!constructed) {
    sources[0] = *cube_imu_calibration__msg__RecorderStatus__get_individual_type_description_source(NULL),
    sources[1] = *builtin_interfaces__msg__Time__get_individual_type_description_source(NULL);
    sources[2] = *std_msgs__msg__Header__get_individual_type_description_source(NULL);
    constructed = true;
  }
  return &source_sequence;
}
