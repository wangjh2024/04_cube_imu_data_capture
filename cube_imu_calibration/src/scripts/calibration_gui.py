#!/usr/bin/python3

import os
import sys
import time
import glob
import grp
import ctypes
import queue
import threading
from collections import deque

import numpy as np
import rclpy
from PyQt5.QtCore import QPointF, QRect, QRectF, QProcess, Qt, QTimer
from PyQt5.QtGui import QColor, QFont, QFontMetrics, QImage, QPainter, QPainterPath, QPen, QPixmap
from PyQt5.QtWidgets import (
    QApplication,
    QDoubleSpinBox,
    QGridLayout,
    QGroupBox,
    QHBoxLayout,
    QLabel,
    QLineEdit,
    QProgressBar,
    QPushButton,
    QFrame,
    QScrollArea,
    QSizePolicy,
    QSplitter,
    QVBoxLayout,
    QWidget,
)
try:
    import cv2
except ImportError:  # pragma: no cover - surfaced in the GUI at runtime.
    cv2 = None

from geometry_msgs.msg import TransformStamped
from rclpy.qos import DurabilityPolicy, QoSProfile, qos_profile_sensor_data
from rclpy.utilities import remove_ros_args
from sensor_msgs.msg import CameraInfo, Image, Imu
from std_srvs.srv import Trigger
from tf2_ros import TransformBroadcaster

from cube_imu_calibration.msg import CubePose, CubePoseStatus, RecorderStatus


GRAVITY_M_S2 = 9.80665
RAD_TO_DEG = 180.0 / np.pi
CUBE_FACE_IDS = frozenset((0, 1, 2, 3, 4))
CUBE_LAYOUT_NAME = "left_hand_5face"
CUBE_LAYOUT_LABEL = "Left Hand Cube"
CUBE_LAYOUT_ALIASES = {
    "left",
    "left_hand",
    "left_hand_5face",
    "left-hand",
    "left-hand-5face",
}
MULTI_TAG_CONSISTENCY_WARN_DEG = 8.0
MULTI_TAG_CONSISTENCY_WARN_M = 0.010
MIN_RECORD_IMAGE_RATE_HZ = 15.0
MIN_RECORD_CUBE_POSE_RATE_HZ = 8.0
MIN_RECORD_IMU_RATE_HZ = 200.0
RATE_READY_TOLERANCE_HZ = 0.25
ACTION_PHASES = (
    ("静止", "带 Tag 的 IMU 模块放在画面中央，保持静止", 10.0),
    ("绕 X 轴", "手持 IMU 模块绕自身 X 轴慢速往返旋转，不要甩动，不要出画", 20.0),
    ("绕 Y 轴", "手持 IMU 模块绕自身 Y 轴慢速往返旋转，至少保持一个 Tag 可见", 20.0),
    ("绕 Z 轴", "手持 IMU 模块绕自身 Z 轴慢速往返旋转，避免空白面正对相机", 20.0),
    ("多面过渡+小平移", "经过 ID0-ID1/2/3/4 边界，尽量双 Tag 同时可见，加入 5-10cm 小平移", 30.0),
    ("8字+三轴组合", "手持 IMU 模块小范围 8 字和平移 5-15cm，同时做三轴慢速组合旋转", 50.0),
)
PHASE_PRECHECK_RATIO = 0.25
PHASE_PRECHECK_MAX_SEC = 3.0
POSE_CHECK_MIN_EVAL_SEC = 0.25
POSE_STILL_GYRO_WARN_DEG_S = 12.0
POSE_GRAVITY_MIN_G = 0.70
POSE_GRAVITY_MAX_G = 1.30
POSE_AXIS_VERTICAL_MIN = 0.82
POSE_REPROJ_SOFT_WARN_PX = 2.0
POSE_REPROJ_BLOCK_PX = 4.0


class DirectSensorConfigC(ctypes.Structure):
    _fields_ = [
        ("color_width", ctypes.c_int),
        ("color_height", ctypes.c_int),
        ("color_fps", ctypes.c_int),
        ("frame_timeout_ms", ctypes.c_int),
        ("output_encoding", ctypes.c_int),
        ("serial_enabled", ctypes.c_int),
        ("serial_port", ctypes.c_char_p),
        ("serial_baudrate", ctypes.c_int),
        ("serial_protocol_mode", ctypes.c_char_p),
        ("serial_imu_rate_hz", ctypes.c_double),
        ("serial_startup_command", ctypes.c_char_p),
    ]


class DirectImageMetaC(ctypes.Structure):
    _fields_ = [
        ("seq", ctypes.c_uint64),
        ("stamp_s", ctypes.c_double),
        ("stamp_us", ctypes.c_uint64),
        ("width", ctypes.c_int),
        ("height", ctypes.c_int),
        ("channels", ctypes.c_int),
        ("step", ctypes.c_int),
        ("encoding", ctypes.c_int),
        ("camera_info_valid", ctypes.c_int),
        ("K", ctypes.c_double * 9),
        ("D", ctypes.c_double * 5),
        ("capture_rate_hz", ctypes.c_double),
    ]


class DirectImuSampleC(ctypes.Structure):
    _fields_ = [
        ("index", ctypes.c_uint64),
        ("stamp_s", ctypes.c_double),
        ("ax", ctypes.c_double),
        ("ay", ctypes.c_double),
        ("az", ctypes.c_double),
        ("gx", ctypes.c_double),
        ("gy", ctypes.c_double),
        ("gz", ctypes.c_double),
    ]


class DirectSensorStatsC(ctypes.Structure):
    _fields_ = [
        ("started", ctypes.c_int),
        ("camera_online", ctypes.c_int),
        ("imu_online", ctypes.c_int),
        ("actual_color_width", ctypes.c_int),
        ("actual_color_height", ctypes.c_int),
        ("actual_color_fps", ctypes.c_int),
        ("image_count", ctypes.c_uint64),
        ("imu_count", ctypes.c_uint64),
        ("dropped_imu_count", ctypes.c_uint64),
        ("image_rate_hz", ctypes.c_double),
        ("imu_rate_hz", ctypes.c_double),
    ]


class DirectSensorClient:
    def __init__(self) -> None:
        self.lib = None
        self.handle = None
        self.error = ""
        self.last_seq = 0
        self._load_library()

    def _candidate_library_paths(self) -> list[str]:
        script_dir = os.path.dirname(os.path.abspath(__file__))
        workspace_root = os.path.abspath(os.path.join(script_dir, "../../.."))
        candidates = []
        if os.environ.get("CUBE_IMU_DIRECT_LIB"):
            candidates.append(os.environ["CUBE_IMU_DIRECT_LIB"])
        candidates.extend(
            [
                os.path.join(script_dir, "libcube_imu_direct_sensor.so"),
                os.path.join(
                    workspace_root,
                    "install/cube_imu_calibration/lib/cube_imu_calibration/libcube_imu_direct_sensor.so",
                ),
                os.path.join(workspace_root, "build/cube_imu_calibration/libcube_imu_direct_sensor.so"),
            ]
        )
        return candidates

    def _load_library(self) -> None:
        for path in self._candidate_library_paths():
            if not path or not os.path.exists(path):
                continue
            try:
                self.lib = ctypes.CDLL(path)
                self._configure_api()
                self.handle = self.lib.direct_sensor_create()
                self.error = ""
                return
            except Exception as exc:  # noqa: BLE001 - surfaced in GUI status.
                self.error = f"加载直连采集库失败 {path}: {exc}"
        if not self.error:
            self.error = "未找到 libcube_imu_direct_sensor.so，请先 colcon build"

    def _configure_api(self) -> None:
        self.lib.direct_sensor_create.argtypes = []
        self.lib.direct_sensor_create.restype = ctypes.c_void_p
        self.lib.direct_sensor_destroy.argtypes = [ctypes.c_void_p]
        self.lib.direct_sensor_destroy.restype = None
        self.lib.direct_sensor_start.argtypes = [
            ctypes.c_void_p,
            ctypes.POINTER(DirectSensorConfigC),
        ]
        self.lib.direct_sensor_start.restype = ctypes.c_int
        self.lib.direct_sensor_stop.argtypes = [ctypes.c_void_p]
        self.lib.direct_sensor_stop.restype = None
        self.lib.direct_sensor_is_started.argtypes = [ctypes.c_void_p]
        self.lib.direct_sensor_is_started.restype = ctypes.c_int
        self.lib.direct_sensor_copy_latest_image.argtypes = [
            ctypes.c_void_p,
            ctypes.POINTER(DirectImageMetaC),
            ctypes.POINTER(ctypes.c_uint8),
            ctypes.c_size_t,
        ]
        self.lib.direct_sensor_copy_latest_image.restype = ctypes.c_size_t
        self.lib.direct_sensor_drain_imu.argtypes = [
            ctypes.c_void_p,
            ctypes.POINTER(DirectImuSampleC),
            ctypes.c_int,
        ]
        self.lib.direct_sensor_drain_imu.restype = ctypes.c_int
        self.lib.direct_sensor_get_stats.argtypes = [
            ctypes.c_void_p,
            ctypes.POINTER(DirectSensorStatsC),
        ]
        self.lib.direct_sensor_get_stats.restype = ctypes.c_int
        self.lib.direct_sensor_get_status.argtypes = [
            ctypes.c_void_p,
            ctypes.c_char_p,
            ctypes.c_size_t,
        ]
        self.lib.direct_sensor_get_status.restype = ctypes.c_int

    def available(self) -> bool:
        return self.lib is not None and self.handle is not None

    def is_started(self) -> bool:
        return bool(self.available() and self.lib.direct_sensor_is_started(self.handle))

    def start(
        self,
        *,
        color_width: int,
        color_height: int,
        color_fps: int,
        frame_timeout_ms: int,
        output_encoding: str,
        serial_port: str,
        serial_baudrate: int,
        serial_protocol_mode: str,
        serial_imu_rate_hz: float,
        serial_startup_command: str,
    ) -> bool:
        if not self.available():
            return False
        encoding_id = 1 if str(output_encoding).lower() == "bgr8" else 0
        cfg = DirectSensorConfigC(
            int(color_width),
            int(color_height),
            int(color_fps),
            int(frame_timeout_ms),
            int(encoding_id),
            1,
            str(serial_port or "auto").encode("utf-8"),
            int(serial_baudrate),
            str(serial_protocol_mode or "v2.1").encode("utf-8"),
            float(serial_imu_rate_hz),
            str(serial_startup_command or "").encode("utf-8"),
        )
        ok = bool(self.lib.direct_sensor_start(self.handle, ctypes.byref(cfg)))
        if not ok:
            self.error = self.status_text()
        else:
            self.error = ""
        return ok

    def stop(self) -> None:
        if self.available():
            self.lib.direct_sensor_stop(self.handle)

    def close(self) -> None:
        if self.available():
            self.lib.direct_sensor_destroy(self.handle)
        self.handle = None
        self.lib = None

    def stats(self) -> DirectSensorStatsC:
        stats = DirectSensorStatsC()
        if self.available():
            self.lib.direct_sensor_get_stats(self.handle, ctypes.byref(stats))
        return stats

    def status_text(self) -> str:
        if not self.available():
            return self.error
        buffer = ctypes.create_string_buffer(1024)
        self.lib.direct_sensor_get_status(self.handle, buffer, len(buffer))
        return buffer.value.decode("utf-8", errors="replace")

    def copy_latest_image(self) -> tuple[DirectImageMetaC, bytes] | None:
        if not self.available() or not self.is_started():
            return None
        meta = DirectImageMetaC()
        required = int(self.lib.direct_sensor_copy_latest_image(self.handle, ctypes.byref(meta), None, 0))
        if required <= 0 or int(meta.seq) == int(self.last_seq):
            return None
        buffer = (ctypes.c_uint8 * required)()
        copied = int(
            self.lib.direct_sensor_copy_latest_image(
                self.handle,
                ctypes.byref(meta),
                buffer,
                required,
            )
        )
        if copied != required:
            self.error = f"直连图像复制失败 required={required} copied={copied}"
            return None
        self.last_seq = int(meta.seq)
        return meta, bytes(buffer)

    def drain_imu(self, max_samples: int = 8192) -> list[DirectImuSampleC]:
        if not self.available() or not self.is_started():
            return []
        out: list[DirectImuSampleC] = []
        while True:
            buffer = (DirectImuSampleC * max_samples)()
            count = int(self.lib.direct_sensor_drain_imu(self.handle, buffer, max_samples))
            if count <= 0:
                break
            out.extend(buffer[i] for i in range(count))
            if count < max_samples:
                break
        return out


class DirectRecorderStatus:
    def __init__(self) -> None:
        self.recording = False
        self.bag_path = ""
        self.elapsed_sec = 0.0
        self.target_duration_sec = 0.0
        self.image_count = 0
        self.imu_count = 0
        self.camera_info_count = 0
        self.cube_pose_count = 0
        self.cube_pose_status_count = 0
        self.image_publishers = 0
        self.imu_publishers = 0
        self.camera_info_publishers = 0
        self.cube_pose_publishers = 1
        self.cube_pose_status_publishers = 1


class DirectRecorder:
    def __init__(self) -> None:
        self.recording = False
        self.path = ""
        self.target_duration_sec = 0.0
        self.start_wall = 0.0
        self.last_elapsed_sec = 0.0
        self.image_count = 0
        self.imu_count = 0
        self.camera_info_count = 0
        self.cube_pose_count = 0
        self.cube_pose_status_count = 0
        self.dropped_queue_items = 0
        self._queue = None
        self._thread = None

    @staticmethod
    def _unique_output_path(base_path: str) -> str:
        expanded = os.path.abspath(os.path.expanduser(base_path or "output_bag"))
        stamp = time.strftime("%Y%m%d_%H%M%S")
        candidate = f"{expanded}_{stamp}"
        suffix = 1
        while os.path.exists(candidate):
            candidate = f"{expanded}_{stamp}_{suffix:02d}"
            suffix += 1
        return candidate

    def start(
        self,
        base_path: str,
        target_duration_sec: float,
        camera_matrix: np.ndarray,
        dist_coeffs: np.ndarray,
    ) -> str:
        if self.recording:
            return self.path
        self.path = self._unique_output_path(base_path)
        os.makedirs(os.path.join(self.path, "images"), exist_ok=True)
        self.target_duration_sec = float(target_duration_sec)
        self.start_wall = time.monotonic()
        self.last_elapsed_sec = 0.0
        self.image_count = 0
        self.imu_count = 0
        self.camera_info_count = 1
        self.cube_pose_count = 0
        self.cube_pose_status_count = 0
        self.dropped_queue_items = 0
        self._write_camera_info(camera_matrix, dist_coeffs)
        self._write_metadata()
        self._queue = queue.Queue(maxsize=4096)
        self.recording = True
        self._thread = threading.Thread(target=self._writer_loop, daemon=True)
        self._thread.start()
        return self.path

    def stop(self) -> None:
        if not self.recording:
            return
        self.last_elapsed_sec = time.monotonic() - self.start_wall
        self.recording = False
        if self._queue is not None:
            try:
                self._queue.put_nowait(None)
            except queue.Full:
                pass
        if self._thread is not None:
            self._thread.join(timeout=5.0)
        self._thread = None
        self._queue = None

    def _write_camera_info(self, camera_matrix: np.ndarray, dist_coeffs: np.ndarray) -> None:
        k = np.asarray(camera_matrix, dtype=float).reshape(3, 3)
        d = np.asarray(dist_coeffs, dtype=float).reshape(-1)
        with open(os.path.join(self.path, "camera_info.yaml"), "w", encoding="utf-8") as file:
            file.write("camera_matrix:\n")
            file.write("  rows: 3\n  cols: 3\n")
            file.write("  data: [" + ", ".join(f"{value:.12g}" for value in k.reshape(-1)) + "]\n")
            file.write("distortion_coefficients:\n")
            file.write(f"  rows: 1\n  cols: {len(d)}\n")
            file.write("  data: [" + ", ".join(f"{value:.12g}" for value in d) + "]\n")

    def _write_metadata(self) -> None:
        with open(os.path.join(self.path, "metadata.yaml"), "w", encoding="utf-8") as file:
            file.write("format: cube_imu_direct_dataset_v1\n")
            file.write("image_dir: images\n")
            file.write("image_timestamps: image_timestamps.csv\n")
            file.write("imu_csv: imu.csv\n")
            file.write("camera_info: camera_info.yaml\n")
            file.write("pose_csv: tag_pose.csv\n")

    def _writer_loop(self) -> None:
        image_csv_path = os.path.join(self.path, "image_timestamps.csv")
        imu_csv_path = os.path.join(self.path, "imu.csv")
        with open(image_csv_path, "w", encoding="utf-8", buffering=1) as image_csv, open(
            imu_csv_path,
            "w",
            encoding="utf-8",
            buffering=1,
        ) as imu_csv:
            image_csv.write("timestamp,filename,width,height,encoding,seq\n")
            imu_csv.write("timestamp,ax,ay,az,gx,gy,gz,index\n")
            while True:
                item = self._queue.get()
                if item is None:
                    break
                kind = item[0]
                if kind == "image":
                    _, stamp, seq, width, height, encoding, data = item
                    ext = "ppm" if encoding == "bgr8" else "pgm"
                    name = f"images/frame_{seq:06d}.{ext}"
                    path = os.path.join(self.path, name)
                    with open(path, "wb") as image_file:
                        if encoding == "bgr8":
                            image_file.write(f"P6\n{width} {height}\n255\n".encode("ascii"))
                            if cv2 is not None:
                                bgr = np.frombuffer(data, dtype=np.uint8).reshape(height, width, 3)
                                rgb = cv2.cvtColor(bgr, cv2.COLOR_BGR2RGB)
                                image_file.write(rgb.tobytes())
                            else:
                                image_file.write(data)
                        else:
                            image_file.write(f"P5\n{width} {height}\n255\n".encode("ascii"))
                            image_file.write(data)
                    image_csv.write(f"{stamp:.9f},{name},{width},{height},{encoding},{seq}\n")
                elif kind == "imu":
                    for sample in item[1]:
                        imu_csv.write(
                            f"{sample[0]:.9f},{sample[1]:.12g},{sample[2]:.12g},"
                            f"{sample[3]:.12g},{sample[4]:.12g},{sample[5]:.12g},"
                            f"{sample[6]:.12g},{int(sample[7])}\n"
                        )

    def enqueue_image(self, meta: DirectImageMetaC, data: bytes) -> None:
        if not self.recording or self._queue is None:
            return
        encoding = "bgr8" if int(meta.encoding) == 1 else "mono8"
        item = (
            "image",
            float(meta.stamp_s),
            int(meta.seq),
            int(meta.width),
            int(meta.height),
            encoding,
            data,
        )
        try:
            self._queue.put_nowait(item)
            self.image_count += 1
        except queue.Full:
            self.dropped_queue_items += 1

    def enqueue_imu_batch(self, samples: list[tuple]) -> None:
        if not self.recording or self._queue is None or not samples:
            return
        try:
            self._queue.put_nowait(("imu", samples))
            self.imu_count += len(samples)
        except queue.Full:
            self.dropped_queue_items += 1

    def note_pose(self) -> None:
        if self.recording:
            self.cube_pose_count += 1

    def note_pose_status(self) -> None:
        if self.recording:
            self.cube_pose_status_count += 1

    def status(self, sensor_stats: DirectSensorStatsC, camera_info_ready: bool) -> DirectRecorderStatus:
        status = DirectRecorderStatus()
        status.recording = self.recording
        status.bag_path = self.path
        if self.recording:
            status.elapsed_sec = time.monotonic() - self.start_wall
        else:
            status.elapsed_sec = self.last_elapsed_sec
        status.target_duration_sec = self.target_duration_sec
        status.image_count = self.image_count
        status.imu_count = self.imu_count
        status.camera_info_count = self.camera_info_count
        status.cube_pose_count = self.cube_pose_count
        status.cube_pose_status_count = self.cube_pose_status_count
        status.image_publishers = 1 if sensor_stats.image_rate_hz > 0.1 else 0
        status.imu_publishers = 1 if sensor_stats.imu_rate_hz > 0.1 else 0
        status.camera_info_publishers = 1 if camera_info_ready else 0
        return status


def fmt_float(value: float) -> str:
    return f"{value:.6f}"


def fmt_vec(values, precision: int = 3, sign: bool = True) -> str:
    pattern = f"{{:{'+' if sign else ''}.{precision}f}}"
    return "  ".join(pattern.format(float(value)) for value in values)


def stamp_to_float(stamp) -> float:
    return float(stamp.sec) + float(stamp.nanosec) * 1e-9


def normalize_cube_layout(value: str) -> str:
    text = str(value or "").strip().lower().replace(" ", "_").replace("-", "_")
    return CUBE_LAYOUT_NAME if text in {alias.replace("-", "_") for alias in CUBE_LAYOUT_ALIASES} else CUBE_LAYOUT_NAME


def rotation_angle_deg(rotation: np.ndarray) -> float:
    value = (float(np.trace(rotation)) - 1.0) * 0.5
    return float(np.arccos(np.clip(value, -1.0, 1.0)) * RAD_TO_DEG)


def fmt_seconds(value: float) -> str:
    return f"{max(0.0, float(value)):.0f}s"


def rotation_matrix_to_quaternion_xyzw(matrix: np.ndarray) -> tuple[float, float, float, float]:
    """Convert a 3x3 rotation matrix to ROS quaternion order x, y, z, w."""
    trace = float(np.trace(matrix))
    if trace > 0.0:
        s = 0.5 / np.sqrt(trace + 1.0)
        w = 0.25 / s
        x = (matrix[2, 1] - matrix[1, 2]) * s
        y = (matrix[0, 2] - matrix[2, 0]) * s
        z = (matrix[1, 0] - matrix[0, 1]) * s
    else:
        diagonal = np.diag(matrix)
        index = int(np.argmax(diagonal))
        if index == 0:
            s = 2.0 * np.sqrt(max(1.0 + matrix[0, 0] - matrix[1, 1] - matrix[2, 2], 1e-12))
            w = (matrix[2, 1] - matrix[1, 2]) / s
            x = 0.25 * s
            y = (matrix[0, 1] + matrix[1, 0]) / s
            z = (matrix[0, 2] + matrix[2, 0]) / s
        elif index == 1:
            s = 2.0 * np.sqrt(max(1.0 + matrix[1, 1] - matrix[0, 0] - matrix[2, 2], 1e-12))
            w = (matrix[0, 2] - matrix[2, 0]) / s
            x = (matrix[0, 1] + matrix[1, 0]) / s
            y = 0.25 * s
            z = (matrix[1, 2] + matrix[2, 1]) / s
        else:
            s = 2.0 * np.sqrt(max(1.0 + matrix[2, 2] - matrix[0, 0] - matrix[1, 1], 1e-12))
            w = (matrix[1, 0] - matrix[0, 1]) / s
            x = (matrix[0, 2] + matrix[2, 0]) / s
            y = (matrix[1, 2] + matrix[2, 1]) / s
            z = 0.25 * s
    norm = max(float(np.sqrt(x * x + y * y + z * z + w * w)), 1e-12)
    return x / norm, y / norm, z / norm, w / norm


class RosBridge:
    def __init__(self) -> None:
        self.node = rclpy.create_node("calibration_gui")
        self.image_topic = self.node.declare_parameter("image_topic", "/camera/image_raw").value
        self.imu_topic = self.node.declare_parameter("imu_topic", "/imu").value
        self.camera_info_topic = self.node.declare_parameter(
            "camera_info_topic", "/camera/camera_info"
        ).value
        self.cube_pose_topic = self.node.declare_parameter("cube_pose_topic", "/cube_pose").value
        self.cube_pose_status_topic = self.node.declare_parameter(
            "cube_pose_status_topic", "/cube_pose_status"
        ).value
        self.recorder_status_topic = self.node.declare_parameter(
            "recorder_status_topic", "/data_recorder_node/status"
        ).value
        self.start_service_name = self.node.declare_parameter(
            "start_service", "/data_recorder_node/start_recording"
        ).value
        self.stop_service_name = self.node.declare_parameter(
            "stop_service", "/data_recorder_node/stop_recording"
        ).value
        self.bag_path = self.node.declare_parameter("bag_path", "output_bag").value
        self.marker_size = float(self.node.declare_parameter("marker_size", 0.03).value)
        self.cube_visual_size = float(self.node.declare_parameter("cube_visual_size", 0.030).value)
        self.cube_layout = normalize_cube_layout(
            self.node.declare_parameter("cube_layout", CUBE_LAYOUT_NAME).value
        )
        self.target_tag_id = int(self.node.declare_parameter("target_tag_id", -1).value)
        self.dictionary_name = self.node.declare_parameter(
            "dictionary", "DICT_APRILTAG_36h11"
        ).value
        self.record_duration_sec = float(
            self.node.declare_parameter("record_duration_sec", 150.0).value
        )
        self.camera_frame = self.node.declare_parameter("camera_frame", "camera_link").value
        self.cube_frame = self.node.declare_parameter("cube_frame", "cube_link").value
        self.imu_frame = self.node.declare_parameter("imu_frame", "imu_link").value
        self.imu_source = self.node.declare_parameter("imu_source", "cube_serial").value
        self.serial_port = self.node.declare_parameter("serial_port", "/dev/ttyACM0").value
        self.serial_baudrate = int(self.node.declare_parameter("serial_baudrate", 2000000).value)
        self.serial_protocol_mode = self.node.declare_parameter(
            "serial_protocol_mode", "v2.1"
        ).value
        self.serial_imu_rate_hz = float(
            self.node.declare_parameter("serial_imu_rate_hz", 500.0).value
        )
        self.serial_startup_command = self.node.declare_parameter(
            "serial_startup_command", "uartout"
        ).value
        self.color_fps = int(self.node.declare_parameter("color_fps", 15).value)
        self.real_sensor_image_qos_depth = int(
            self.node.declare_parameter("real_sensor_image_qos_depth", 30).value
        )
        self.real_sensor_imu_qos_depth = int(
            self.node.declare_parameter("real_sensor_imu_qos_depth", 1000).value
        )
        self.recorder_image_qos_depth = int(
            self.node.declare_parameter("recorder_image_qos_depth", 120).value
        )
        self.recorder_imu_qos_depth = int(
            self.node.declare_parameter("recorder_imu_qos_depth", 2000).value
        )
        self.recorder_status_qos_depth = int(
            self.node.declare_parameter("recorder_status_qos_depth", 100).value
        )
        self.camera_output_encoding = self.node.declare_parameter(
            "camera_output_encoding", "mono8"
        ).value
        self.image_preview_rate_hz = float(
            self.node.declare_parameter("image_preview_rate_hz", 5.0).value
        )
        self.tag_detection_rate_hz = float(
            self.node.declare_parameter("tag_detection_rate_hz", 15.0).value
        )
        self.tag_detection_scale = float(
            self.node.declare_parameter("tag_detection_scale", 0.5).value
        )
        self.preview_scale = float(self.node.declare_parameter("preview_scale", 1.0).value)
        self.draw_preview_overlay = bool(
            self.node.declare_parameter("draw_preview_overlay", True).value
        )
        self.gui_display_rate_hz = float(
            self.node.declare_parameter("gui_display_rate_hz", 5.0).value
        )
        self.enable_cube_detection = bool(
            self.node.declare_parameter("enable_cube_detection", True).value
        )
        self.pose_csv_path = self.node.declare_parameter("pose_csv_path", "outputs/tag_pose.csv").value
        self.publish_tf = bool(self.node.declare_parameter("publish_tf", True).value)

        self.direct_sensor = DirectSensorClient()
        self.direct_recorder = DirectRecorder()
        self.direct_sensor_status = (
            "直连采集库已加载" if self.direct_sensor.available() else self.direct_sensor.error
        )

        self.last_status = None
        self.last_status_wall = 0.0
        self.last_pose = None
        self.last_pose_wall = 0.0
        self.last_tag_warning = (
            "视频采集模式：显示原始视频并录制图像/IMU/CameraInfo"
            if not self.enable_cube_detection
            else f"等待图像（{CUBE_LAYOUT_LABEL}，ID0=center ID1=top ID2=right ID3=bottom ID4=left）"
        )
        self.last_tag_ids = []
        self.observed_cube_ids = set()
        self.last_detection_tag_count = 0
        self.last_reproj_error = float("nan")
        self.last_multitag_consistency = (float("nan"), float("nan"))
        self.last_overlay_info = (
            "原始视频预览"
            if not self.enable_cube_detection
            else "等待 Cube 可视化"
        )
        self.last_service_message = "等待操作"
        self.last_image = None
        self.last_image_wall = 0.0
        self.last_image_info = "等待图像"
        self.last_image_error = ""
        self.last_image_stamp = 0.0
        self.last_image_stamp_issue = "等待图像时间戳"
        self.last_image_seq = 0
        self.last_image_display_scale = 1.0
        self.last_image_preview_wall = 0.0
        self.last_tag_detection_wall = 0.0
        self.last_camera_info_wall = 0.0
        self.last_camera_info_info = "等待 CameraInfo"
        self.last_camera_info_stamp = 0.0
        self.last_camera_info_stamp_issue = "等待 CameraInfo 时间戳"
        self.image_times = deque(maxlen=120)
        self.cube_pose_times = deque(maxlen=120)
        self.imu_samples = deque(maxlen=1200)
        self.imu_times = deque(maxlen=600)
        self.last_imu_wall = 0.0
        self.last_imu_info = "等待 IMU"
        self.last_imu_stamp = 0.0
        self.last_imu_stamp_issue = "等待 IMU 时间戳"
        self.last_imu_accel_g_info = "-"
        self.last_imu_gyro_deg_info = "-"
        self.last_imu_norm_info = "-"
        self.image_sub = None
        self.imu_sub = None
        self.camera_info_sub = None
        self.camera_matrix = np.array(
            [[600.0, 0.0, 320.0], [0.0, 600.0, 240.0], [0.0, 0.0, 1.0]],
            dtype=np.float64,
        )
        self.dist_coeffs = np.zeros((5, 1), dtype=np.float64)
        self.camera_info_ready = False
        self._aruco_detector = None
        self._aruco_dictionary = None
        self._aruco_signature = None
        self.pose_csv_file = None
        self.pose_csv_open_path = ""

        self.pose_pub = self.node.create_publisher(CubePose, self.cube_pose_topic, 10)
        self.pose_status_pub = self.node.create_publisher(
            CubePoseStatus, self.cube_pose_status_topic, 10
        )
        self.tf_broadcaster = TransformBroadcaster(self.node) if self.publish_tf else None

        self.start_client = self.node.create_client(Trigger, self.start_service_name)
        self.stop_client = self.node.create_client(Trigger, self.stop_service_name)
        self.configure_data_subscriptions()

    def _status_cb(self, msg: RecorderStatus) -> None:
        self.last_status = msg
        self.last_status_wall = time.monotonic()

    def configure_data_subscriptions(self, start_capture: bool = False) -> None:
        """Start direct hardware capture and run visualization/detection locally."""

        self.last_image = None
        self.last_image_wall = 0.0
        self.last_image_info = "等待 Orbbec 直连图像"
        self.last_image_error = ""
        self.last_image_stamp = 0.0
        self.last_image_stamp_issue = "等待图像时间戳"
        self.last_image_seq += 1
        self.last_image_display_scale = 1.0
        self.last_image_preview_wall = 0.0
        self.last_tag_detection_wall = 0.0
        self.last_camera_info_wall = 0.0
        self.last_camera_info_info = "等待 Orbbec SDK 内参"
        self.last_camera_info_stamp = 0.0
        self.last_camera_info_stamp_issue = "等待 CameraInfo 时间戳"
        self.camera_info_ready = False
        self.camera_matrix = np.array(
            [[600.0, 0.0, 320.0], [0.0, 600.0, 240.0], [0.0, 0.0, 1.0]],
            dtype=np.float64,
        )
        self.dist_coeffs = np.zeros((5, 1), dtype=np.float64)
        self.last_pose = None
        self.last_pose_wall = 0.0
        self.last_tag_warning = (
            "视频采集模式：显示原始视频并录制图像/IMU/CameraInfo"
            if not self.enable_cube_detection
            else f"等待图像（{CUBE_LAYOUT_LABEL}）"
        )
        self.last_tag_ids = []
        self.observed_cube_ids.clear()
        self.cube_pose_times.clear()
        self.last_detection_tag_count = 0
        self.last_reproj_error = float("nan")
        self.last_multitag_consistency = (float("nan"), float("nan"))
        self.last_overlay_info = (
            "原始视频预览"
            if not self.enable_cube_detection
            else "等待 Cube 可视化"
        )
        self.image_times.clear()
        self.imu_samples.clear()
        self.imu_times.clear()
        self.last_imu_wall = 0.0
        self.last_imu_info = f"等待串口 IMU {self.serial_port}"
        self.last_imu_stamp = 0.0
        self.last_imu_stamp_issue = "等待 IMU 时间戳"
        self.last_imu_accel_g_info = "-"
        self.last_imu_gyro_deg_info = "-"
        self.last_imu_norm_info = "-"

        if start_capture:
            self.start_direct_sensor()

    def start_direct_sensor(self) -> bool:
        if self.direct_sensor.is_started():
            return True
        ok = self.direct_sensor.start(
            color_width=1280,
            color_height=720,
            color_fps=max(int(self.color_fps), int(MIN_RECORD_IMAGE_RATE_HZ)),
            frame_timeout_ms=1000,
            output_encoding=self.camera_output_encoding,
            serial_port=self.serial_port,
            serial_baudrate=self.serial_baudrate,
            serial_protocol_mode=self.serial_protocol_mode,
            serial_imu_rate_hz=max(float(self.serial_imu_rate_hz), float(MIN_RECORD_IMU_RATE_HZ)),
            serial_startup_command=self.serial_startup_command,
        )
        self.direct_sensor_status = self.direct_sensor.status_text()
        if not ok:
            self.last_image_info = self.direct_sensor_status
            self.last_imu_info = self.direct_sensor_status
        return ok

    def stop_direct_sensor(self) -> None:
        self.direct_sensor.stop()
        self.direct_sensor_status = self.direct_sensor.status_text()

    @staticmethod
    def _stamp_from_float(stamp_s: float):
        msg = Image().header.stamp
        if not np.isfinite(stamp_s) or stamp_s <= 0.0:
            stamp_s = time.time()
        sec = int(stamp_s)
        msg.sec = sec
        msg.nanosec = int(max(0.0, stamp_s - sec) * 1e9)
        return msg

    def _image_msg_from_direct(self, meta: DirectImageMetaC, data: bytes) -> Image:
        msg = Image()
        msg.header.stamp = self._stamp_from_float(float(meta.stamp_s))
        msg.header.frame_id = self.camera_frame
        msg.height = int(meta.height)
        msg.width = int(meta.width)
        msg.encoding = "bgr8" if int(meta.encoding) == 1 else "mono8"
        msg.is_bigendian = 0
        msg.step = int(meta.step)
        msg.data = data
        return msg

    def _camera_info_from_direct(self, meta: DirectImageMetaC) -> None:
        if int(meta.camera_info_valid) <= 0:
            if not self.camera_info_ready:
                self.last_camera_info_info = "Orbbec SDK 暂未返回内参，使用 fallback 内参"
            return
        msg = CameraInfo()
        msg.header.stamp = self._stamp_from_float(float(meta.stamp_s))
        msg.header.frame_id = self.camera_frame
        msg.width = int(meta.width)
        msg.height = int(meta.height)
        msg.distortion_model = "plumb_bob"
        msg.k = [float(value) for value in meta.K]
        msg.d = [float(value) for value in meta.D]
        msg.r = [
            1.0, 0.0, 0.0,
            0.0, 1.0, 0.0,
            0.0, 0.0, 1.0,
        ]
        msg.p = [
            msg.k[0], 0.0, msg.k[2], 0.0,
            0.0, msg.k[4], msg.k[5], 0.0,
            0.0, 0.0, 1.0, 0.0,
        ]
        self._camera_info_cb(msg)

    def _direct_imu_cb(self, sample_msg: DirectImuSampleC) -> tuple:
        now = time.monotonic()
        stamp = float(sample_msg.stamp_s)
        if self.last_imu_stamp > 0.0 and stamp < self.last_imu_stamp - 1e-9:
            issue = f"imu: 时间戳倒退 {self.last_imu_stamp - stamp:.6f}s"
        elif stamp <= 0.0:
            issue = "imu: 时间戳=0"
            stamp = time.time()
        else:
            issue = "ok"
        self.last_imu_stamp = stamp
        self.last_imu_stamp_issue = issue
        accel = np.array([sample_msg.ax, sample_msg.ay, sample_msg.az], dtype=float)
        gyro = np.array([sample_msg.gx, sample_msg.gy, sample_msg.gz], dtype=float)
        accel_g = accel / GRAVITY_M_S2
        gyro_deg_s = gyro * RAD_TO_DEG
        sample = (
            stamp,
            float(accel[0]),
            float(accel[1]),
            float(accel[2]),
            float(gyro[0]),
            float(gyro[1]),
            float(gyro[2]),
            now,
        )
        self.imu_samples.append(sample)
        self.imu_times.append(stamp)
        self.last_imu_wall = now
        self.last_imu_info = (
            f"acc=[{sample[1]:.3f}, {sample[2]:.3f}, {sample[3]:.3f}] m/s^2   "
            f"gyro=[{sample[4]:.3f}, {sample[5]:.3f}, {sample[6]:.3f}] rad/s"
        )
        self.last_imu_accel_g_info = fmt_vec(accel_g, 3)
        self.last_imu_gyro_deg_info = fmt_vec(gyro_deg_s, 2)
        self.last_imu_norm_info = (
            f"|a|={float(np.linalg.norm(accel_g)):.3f} g   "
            f"|w|={float(np.linalg.norm(gyro_deg_s)):.2f} deg/s"
        )
        return (
            stamp,
            float(accel[0]),
            float(accel[1]),
            float(accel[2]),
            float(gyro[0]),
            float(gyro[1]),
            float(gyro[2]),
            int(sample_msg.index),
        )

    def _poll_direct_sensor(self) -> None:
        if not self.direct_sensor.is_started():
            self.last_status = self.direct_recorder.status(self.direct_sensor.stats(), self.camera_info_ready)
            self.last_status_wall = time.monotonic()
            return

        image = self.direct_sensor.copy_latest_image()
        if image is not None:
            meta, data = image
            self._camera_info_from_direct(meta)
            msg = self._image_msg_from_direct(meta, data)
            self._image_cb(msg)
            self.direct_recorder.enqueue_image(meta, data)

        raw_samples = self.direct_sensor.drain_imu()
        if raw_samples:
            batch = [self._direct_imu_cb(sample) for sample in raw_samples]
            self.direct_recorder.enqueue_imu_batch(batch)

        stats = self.direct_sensor.stats()
        self.direct_sensor_status = self.direct_sensor.status_text()
        self.last_status = self.direct_recorder.status(stats, self.camera_info_ready)
        self.last_status_wall = time.monotonic()

    def _camera_info_cb(self, msg: CameraInfo) -> None:
        stamp, issue = self._checked_stamp(
            "CameraInfo", msg.header.stamp, self.last_camera_info_stamp
        )
        self.last_camera_info_stamp = stamp
        self.last_camera_info_stamp_issue = issue
        if len(msg.k) >= 9 and msg.k[0] > 0.0 and msg.k[4] > 0.0:
            self.camera_matrix = np.array(msg.k, dtype=np.float64).reshape(3, 3)
            if msg.d:
                self.dist_coeffs = np.array(msg.d, dtype=np.float64).reshape(-1, 1)
            else:
                self.dist_coeffs = np.zeros((5, 1), dtype=np.float64)
            self.camera_info_ready = True
            self.last_camera_info_wall = time.monotonic()
            self.last_camera_info_info = (
                f"CameraInfo fx={self.camera_matrix[0, 0]:.1f} "
                f"fy={self.camera_matrix[1, 1]:.1f} "
                f"cx={self.camera_matrix[0, 2]:.1f} cy={self.camera_matrix[1, 2]:.1f}"
            )

    @staticmethod
    def _checked_stamp(name: str, stamp_msg, previous_stamp: float) -> tuple[float, str]:
        stamp = stamp_to_float(stamp_msg)
        if stamp <= 0.0:
            return 0.0, f"{name}: header.stamp=0"
        if previous_stamp > 0.0 and stamp < previous_stamp - 1e-9:
            return stamp, f"{name}: header.stamp 倒退 {previous_stamp - stamp:.6f}s"
        return stamp, "ok"

    @staticmethod
    def _rate_due(now: float, last_wall: float, rate_hz: float) -> bool:
        if rate_hz <= 0.0:
            return False
        return last_wall <= 0.0 or now - last_wall >= 1.0 / max(rate_hz, 1e-6)

    @staticmethod
    def _msg_data_buffer(msg: Image):
        try:
            return memoryview(msg.data)
        except TypeError:
            return bytes(msg.data)

    @staticmethod
    def _resize_for_preview(array: np.ndarray, scale: float) -> tuple[np.ndarray, float]:
        if cv2 is None:
            return np.ascontiguousarray(array), 1.0
        if not np.isfinite(scale) or scale <= 0.0 or scale >= 0.999:
            return np.ascontiguousarray(array), 1.0
        scale = max(0.25, min(1.0, float(scale)))
        resized = cv2.resize(array, None, fx=scale, fy=scale, interpolation=cv2.INTER_AREA)
        actual_scale = float(resized.shape[1]) / max(1.0, float(array.shape[1]))
        return np.ascontiguousarray(resized), actual_scale

    @staticmethod
    def _qimage_from_msg(msg: Image, preview_scale: float = 1.0) -> tuple[QImage, float]:
        width = int(msg.width)
        height = int(msg.height)
        step = int(msg.step)
        encoding = msg.encoding.lower()
        data = RosBridge._msg_data_buffer(msg)

        if width <= 0 or height <= 0 or step <= 0:
            raise ValueError("empty image")

        # Keep the conversion local to the GUI so the project does not depend on
        # cv_bridge for display. Preview is downscaled before QImage creation to
        # keep Qt repaint cost away from the capture/detection hot path.
        if encoding == "rgb8":
            raw = np.frombuffer(data, dtype=np.uint8).reshape(height, step)
            rgb = raw[:, : width * 3].reshape(height, width, 3)
            preview, actual_scale = RosBridge._resize_for_preview(rgb, preview_scale)
            qimage = QImage(
                preview.data,
                preview.shape[1],
                preview.shape[0],
                preview.strides[0],
                QImage.Format_RGB888,
            ).copy()
            return qimage, actual_scale
        if encoding == "bgr8":
            raw = np.frombuffer(data, dtype=np.uint8).reshape(height, step)
            bgr = raw[:, : width * 3].reshape(height, width, 3)
            preview, actual_scale = RosBridge._resize_for_preview(bgr, preview_scale)
            bgr_format = getattr(QImage, "Format_BGR888", None)
            if bgr_format is not None:
                qimage = QImage(
                    preview.data,
                    preview.shape[1],
                    preview.shape[0],
                    preview.strides[0],
                    bgr_format,
                ).copy()
            else:
                qimage = QImage(
                    preview.data,
                    preview.shape[1],
                    preview.shape[0],
                    preview.strides[0],
                    QImage.Format_RGB888,
                ).rgbSwapped().copy()
            return qimage, actual_scale
        if encoding in ("mono8", "8uc1"):
            raw = np.frombuffer(data, dtype=np.uint8).reshape(height, step)[:, :width]
            preview, actual_scale = RosBridge._resize_for_preview(raw, preview_scale)
            qimage = QImage(
                preview.data,
                preview.shape[1],
                preview.shape[0],
                preview.strides[0],
                QImage.Format_Grayscale8,
            ).copy()
            return qimage, actual_scale
        if encoding == "rgba8":
            raw = np.frombuffer(data, dtype=np.uint8).reshape(height, step)
            rgba = raw[:, : width * 4].reshape(height, width, 4)
            preview, actual_scale = RosBridge._resize_for_preview(rgba, preview_scale)
            qimage = QImage(
                preview.data,
                preview.shape[1],
                preview.shape[0],
                preview.strides[0],
                QImage.Format_RGBA8888,
            ).copy()
            return qimage, actual_scale
        if encoding == "bgra8":
            raw = np.frombuffer(data, dtype=np.uint8).reshape(height, step)
            bgra = raw[:, : width * 4].reshape(height, width, 4)
            preview, actual_scale = RosBridge._resize_for_preview(bgra, preview_scale)
            image = QImage(
                preview.data,
                preview.shape[1],
                preview.shape[0],
                preview.strides[0],
                QImage.Format_ARGB32,
            ).copy()
            return image.rgbSwapped(), actual_scale
        if encoding in ("mono16", "16uc1"):
            row_stride = step // np.dtype(np.uint16).itemsize
            array = np.frombuffer(data, dtype=np.uint16).reshape(height, row_stride)[:, :width]
            scaled = RosBridge._normalized_gray_array(array, width, height)
            preview, actual_scale = RosBridge._resize_for_preview(scaled, preview_scale)
            qimage = QImage(
                preview.data,
                preview.shape[1],
                preview.shape[0],
                preview.strides[0],
                QImage.Format_Grayscale8,
            ).copy()
            return qimage, actual_scale
        if encoding in ("32fc1", "32fc"):
            row_stride = step // np.dtype(np.float32).itemsize
            array = np.frombuffer(data, dtype=np.float32).reshape(height, row_stride)[:, :width]
            scaled = RosBridge._normalized_gray_array(array, width, height)
            preview, actual_scale = RosBridge._resize_for_preview(scaled, preview_scale)
            qimage = QImage(
                preview.data,
                preview.shape[1],
                preview.shape[0],
                preview.strides[0],
                QImage.Format_Grayscale8,
            ).copy()
            return qimage, actual_scale

        raise ValueError(f"不支持图像编码 {msg.encoding}")

    @staticmethod
    def _normalized_gray_array(array: np.ndarray, width: int, height: int) -> np.ndarray:
        finite = np.isfinite(array)
        if not finite.any():
            scaled = np.zeros((height, width), dtype=np.uint8)
        else:
            valid = array[finite].astype(np.float64)
            low = float(np.percentile(valid, 1.0))
            high = float(np.percentile(valid, 99.0))
            if high <= low:
                scaled = np.zeros((height, width), dtype=np.uint8)
            else:
                source = np.where(finite, array.astype(np.float64), low)
                clipped = np.clip(source, low, high)
                scaled = ((clipped - low) * 255.0 / (high - low)).astype(np.uint8)
        return np.ascontiguousarray(scaled)

    @staticmethod
    def _gray_from_msg(msg: Image) -> np.ndarray | None:
        width = int(msg.width)
        height = int(msg.height)
        step = int(msg.step)
        encoding = msg.encoding.lower()
        data = RosBridge._msg_data_buffer(msg)

        if width <= 0 or height <= 0 or step <= 0:
            return None
        if encoding in ("mono8", "8uc1"):
            return np.frombuffer(data, dtype=np.uint8).reshape(height, step)[:, :width].copy()
        if encoding == "rgb8":
            raw = np.frombuffer(data, dtype=np.uint8).reshape(height, step)
            rgb = raw[:, : width * 3].reshape(height, width, 3)
            if cv2 is not None:
                return cv2.cvtColor(rgb, cv2.COLOR_RGB2GRAY)
            return (0.299 * rgb[:, :, 0] + 0.587 * rgb[:, :, 1] + 0.114 * rgb[:, :, 2]).astype(np.uint8)
        if encoding == "bgr8":
            raw = np.frombuffer(data, dtype=np.uint8).reshape(height, step)
            bgr = raw[:, : width * 3].reshape(height, width, 3)
            if cv2 is not None:
                return cv2.cvtColor(bgr, cv2.COLOR_BGR2GRAY)
            return (0.114 * bgr[:, :, 0] + 0.587 * bgr[:, :, 1] + 0.299 * bgr[:, :, 2]).astype(np.uint8)
        if encoding == "rgba8":
            raw = np.frombuffer(data, dtype=np.uint8).reshape(height, step)
            rgba = raw[:, : width * 4].reshape(height, width, 4)
            if cv2 is not None:
                return cv2.cvtColor(rgba, cv2.COLOR_RGBA2GRAY)
            return (0.299 * rgba[:, :, 0] + 0.587 * rgba[:, :, 1] + 0.114 * rgba[:, :, 2]).astype(np.uint8)
        if encoding == "bgra8":
            raw = np.frombuffer(data, dtype=np.uint8).reshape(height, step)
            bgra = raw[:, : width * 4].reshape(height, width, 4)
            if cv2 is not None:
                return cv2.cvtColor(bgra, cv2.COLOR_BGRA2GRAY)
            return (0.114 * bgra[:, :, 0] + 0.587 * bgra[:, :, 1] + 0.299 * bgra[:, :, 2]).astype(np.uint8)
        if encoding in ("mono16", "16uc1"):
            row_stride = step // np.dtype(np.uint16).itemsize
            array = np.frombuffer(data, dtype=np.uint16).reshape(height, row_stride)[:, :width]
            return RosBridge._normalized_gray_array(array, width, height)
        if encoding in ("32fc1", "32fc"):
            row_stride = step // np.dtype(np.float32).itemsize
            array = np.frombuffer(data, dtype=np.float32).reshape(height, row_stride)[:, :width]
            return RosBridge._normalized_gray_array(array, width, height)
        return None

    def _image_cb(self, msg: Image) -> None:
        try:
            now = time.monotonic()
            stamp, issue = self._checked_stamp("image", msg.header.stamp, self.last_image_stamp)
            self.last_image_stamp = stamp
            self.last_image_stamp_issue = issue
            self.last_image_wall = now
            self.image_times.append(now)
            self.last_image_info = (
                f"{msg.width}x{msg.height}  {msg.encoding}  stamp="
                f"{msg.header.stamp.sec}.{msg.header.stamp.nanosec:09d}"
            )

            preview_due = self.last_image is None or self._rate_due(
                now, self.last_image_preview_wall, self.image_preview_rate_hz
            )
            tag_due = self.enable_cube_detection and self._rate_due(
                now, self.last_tag_detection_wall, self.tag_detection_rate_hz
            )
            if not preview_due and not tag_due:
                return

            display_image = None
            display_scale = 1.0
            if preview_due:
                display_image, display_scale = self._qimage_from_msg(msg, self.preview_scale)
                self.last_image_error = ""
                self.last_image_preview_wall = now
                self.last_image = display_image
                self.last_image_display_scale = display_scale
                self.last_image_seq += 1
            if not self.enable_cube_detection:
                self.last_pose = None
                self.last_pose_wall = 0.0
                self.last_tag_ids = []
                self.last_detection_tag_count = 0
                self.last_reproj_error = float("nan")
                self.last_multitag_consistency = (float("nan"), float("nan"))
                self.last_tag_warning = "视频采集模式：显示原始视频并录制图像/IMU/CameraInfo"
                self.last_overlay_info = "原始视频预览"

            if tag_due:
                self.last_tag_detection_wall = now
                try:
                    gray = self._gray_from_msg(msg)
                    if gray is not None:
                        detect_gray, corner_scale = self._prepare_detection_gray(gray)
                        overlay = self._detect_apriltag(
                            detect_gray,
                            msg,
                            display_image if self.draw_preview_overlay else None,
                            corner_scale,
                            display_scale,
                        )
                        if overlay is not None:
                            self.last_image = overlay
                            self.last_image_display_scale = display_scale
                            self.last_image_seq += 1
                except Exception as exc:  # noqa: BLE001 - keep live preview even if detection fails.
                    self.last_tag_warning = f"AprilTag 检测异常：{exc}"
        except Exception as exc:  # noqa: BLE001 - display conversion errors belong in GUI.
            self.last_image_error = str(exc)
            self.last_image_wall = time.monotonic()

    def _prepare_detection_gray(self, gray: np.ndarray) -> tuple[np.ndarray, float]:
        if cv2 is None:
            return gray, 1.0
        scale = float(self.tag_detection_scale)
        if not np.isfinite(scale) or scale <= 0.0 or scale >= 0.999:
            return gray, 1.0
        scale = max(0.25, min(1.0, scale))
        resized = cv2.resize(gray, None, fx=scale, fy=scale, interpolation=cv2.INTER_AREA)
        return resized, 1.0 / scale

    def _build_aruco_detector(self) -> bool:
        if cv2 is None or not hasattr(cv2, "aruco"):
            self.last_tag_warning = "OpenCV aruco 模块不可用，无法检测 AprilTag"
            return False

        signature = self.dictionary_name
        if self._aruco_signature == signature and self._aruco_dictionary is not None:
            return True

        dictionary_id = getattr(cv2.aruco, self.dictionary_name, None)
        if dictionary_id is None:
            self.last_tag_warning = f"不支持的 AprilTag 字典：{self.dictionary_name}"
            return False

        self._aruco_dictionary = cv2.aruco.getPredefinedDictionary(dictionary_id)
        if hasattr(cv2.aruco, "ArucoDetector"):
            params = cv2.aruco.DetectorParameters()
            self._aruco_detector = cv2.aruco.ArucoDetector(self._aruco_dictionary, params)
        else:
            self._aruco_detector = None
        self._aruco_signature = signature
        return True

    def _publish_pose_status(
        self,
        stamp_msg,
        pose_valid: bool,
        tag_ids: list[int],
        reproj_error: float,
        message: str,
    ) -> None:
        status = CubePoseStatus()
        status.header.stamp = stamp_msg
        status.header.frame_id = self.camera_frame
        status.pose_valid = bool(pose_valid)
        status.tag_count = int(len(tag_ids))
        status.tag_ids = [int(tag_id) for tag_id in tag_ids]
        status.reproj_error = float(reproj_error) if np.isfinite(reproj_error) else -1.0
        status.message = str(message)
        self.pose_status_pub.publish(status)
        self.direct_recorder.note_pose_status()

    def _detect_apriltag(
        self,
        gray: np.ndarray,
        image_msg: Image,
        display_image: QImage | None,
        corner_scale: float = 1.0,
        display_scale: float = 1.0,
    ) -> QImage | None:
        if not self._build_aruco_detector():
            return None
        if self.marker_size <= 0.0:
            self.last_tag_warning = "Tag 边长必须大于 0"
            return None

        try:
            if self._aruco_detector is not None:
                corners, ids, _ = self._aruco_detector.detectMarkers(gray)
            else:
                params = cv2.aruco.DetectorParameters_create()
                corners, ids, _ = cv2.aruco.detectMarkers(
                    gray,
                    self._aruco_dictionary,
                    parameters=params,
                )
        except Exception as exc:  # noqa: BLE001 - detector errors should be visible in GUI.
            self.last_tag_warning = f"AprilTag 检测异常：{exc}"
            return None

        if ids is None or len(ids) == 0:
            self.last_tag_ids = []
            self.last_detection_tag_count = 0
            self.last_reproj_error = float("nan")
            self.last_multitag_consistency = (float("nan"), float("nan"))
            self.last_tag_warning = "未识别到 AprilTag：请把 Cube 放回画面中央"
            self._publish_pose_status(
                image_msg.header.stamp,
                False,
                [],
                float("nan"),
                self.last_tag_warning,
            )
            return None

        flat_ids = [int(tag_id) for tag_id in np.asarray(ids).reshape(-1)]
        self.last_tag_ids = flat_ids
        if abs(float(corner_scale) - 1.0) > 1e-6:
            corners = [
                np.asarray(tag_corners, dtype=np.float32) * float(corner_scale)
                for tag_corners in corners
            ]

        candidate_indices = self._candidate_tag_indices(flat_ids, corners)
        if not candidate_indices:
            self.last_detection_tag_count = 0
            self.last_reproj_error = float("nan")
            self.last_multitag_consistency = (float("nan"), float("nan"))
            self._publish_pose_status(
                image_msg.header.stamp,
                False,
                flat_ids,
                float("nan"),
                self.last_tag_warning,
            )
            return None

        object_points_by_tag = []
        image_points_by_tag = []
        used_tag_ids = []
        used_face_names = []
        for candidate_index in candidate_indices:
            tag_id = flat_ids[candidate_index]
            tag_object_points, face_name = self._tag_object_points_in_cube(tag_id)
            if tag_object_points is None:
                continue
            tag_image_points = np.asarray(corners[candidate_index], dtype=np.float64).reshape(4, 2)
            object_points_by_tag.append(tag_object_points)
            image_points_by_tag.append(tag_image_points)
            used_tag_ids.append(tag_id)
            used_face_names.append(face_name)

        if not object_points_by_tag:
            self.last_detection_tag_count = 0
            self.last_reproj_error = float("nan")
            self.last_multitag_consistency = (float("nan"), float("nan"))
            self.last_tag_warning = "没有可用于 Cube PnP 的 AprilTag 角点"
            self._publish_pose_status(
                image_msg.header.stamp,
                False,
                flat_ids,
                float("nan"),
                self.last_tag_warning,
            )
            return None

        object_points = np.vstack(object_points_by_tag).astype(np.float64)
        image_points = np.vstack(image_points_by_tag).astype(np.float64)
        seed_pose = self._estimate_cube_pose_from_single_tag(
            used_tag_ids[0],
            image_points_by_tag[0],
        )
        if seed_pose is None:
            self.last_detection_tag_count = 0
            self.last_reproj_error = float("nan")
            self.last_multitag_consistency = (float("nan"), float("nan"))
            self.last_tag_warning = "单 Tag 初值解算失败"
            self._publish_pose_status(
                image_msg.header.stamp,
                False,
                used_tag_ids,
                float("nan"),
                self.last_tag_warning,
            )
            return None

        cube_rotation, cube_tvec = seed_pose
        cube_rvec, _ = cv2.Rodrigues(cube_rotation)
        cube_rvec = np.asarray(cube_rvec, dtype=np.float64).reshape(3)
        cube_tvec = np.asarray(cube_tvec, dtype=np.float64).reshape(3)
        max_single_rot_deg, max_single_pos_m = self._single_tag_pose_consistency(
            used_tag_ids,
            image_points_by_tag,
        )
        self.last_multitag_consistency = (max_single_rot_deg, max_single_pos_m)
        if len(used_tag_ids) > 1:
            try:
                success, refined_rvec, refined_tvec = cv2.solvePnP(
                    object_points,
                    image_points,
                    self.camera_matrix,
                    self.dist_coeffs,
                    cube_rvec.reshape(3, 1),
                    cube_tvec.reshape(3, 1),
                    True,
                    cv2.SOLVEPNP_ITERATIVE,
                )
                if not success:
                    self.last_detection_tag_count = 0
                    self.last_reproj_error = float("nan")
                    self.last_multitag_consistency = (float("nan"), float("nan"))
                    self.last_tag_warning = "Cube 联合 PnP 失败"
                    self._publish_pose_status(
                        image_msg.header.stamp,
                        False,
                        used_tag_ids,
                        float("nan"),
                        self.last_tag_warning,
                    )
                    return None
                cube_rvec = np.asarray(refined_rvec, dtype=np.float64).reshape(3)
                cube_tvec = np.asarray(refined_tvec, dtype=np.float64).reshape(3)
                if hasattr(cv2, "solvePnPRefineLM"):
                    cube_rvec, cube_tvec = cv2.solvePnPRefineLM(
                        object_points,
                        image_points,
                        self.camera_matrix,
                        self.dist_coeffs,
                        cube_rvec.reshape(3, 1),
                        cube_tvec.reshape(3, 1),
                    )
                    cube_rvec = np.asarray(cube_rvec, dtype=np.float64).reshape(3)
                    cube_tvec = np.asarray(cube_tvec, dtype=np.float64).reshape(3)
                cube_rotation, _ = cv2.Rodrigues(cube_rvec)
                cube_rotation = np.asarray(cube_rotation, dtype=np.float64).reshape(3, 3)
            except Exception as exc:  # noqa: BLE001 - pose errors should be visible in GUI.
                self.last_detection_tag_count = 0
                self.last_reproj_error = float("nan")
                self.last_multitag_consistency = (float("nan"), float("nan"))
                self.last_tag_warning = f"Cube 联合 PnP 异常：{exc}"
                self._publish_pose_status(
                    image_msg.header.stamp,
                    False,
                    used_tag_ids,
                    float("nan"),
                    self.last_tag_warning,
                )
                return None
        reproj_error = self._reprojection_rmse(object_points, image_points, cube_rvec, cube_tvec)

        transform = np.eye(4, dtype=np.float64)
        transform[:3, :3] = cube_rotation
        transform[:3, 3] = cube_tvec
        qx, qy, qz, qw = rotation_matrix_to_quaternion_xyzw(cube_rotation)

        pose = CubePose()
        pose.header.stamp = image_msg.header.stamp
        pose.header.frame_id = self.camera_frame
        pose.translation.x = float(cube_tvec[0])
        pose.translation.y = float(cube_tvec[1])
        pose.translation.z = float(cube_tvec[2])
        pose.orientation.x = float(qx)
        pose.orientation.y = float(qy)
        pose.orientation.z = float(qz)
        pose.orientation.w = float(qw)
        pose.transform = [float(value) for value in transform.reshape(-1)]
        self.pose_pub.publish(pose)
        self.direct_recorder.note_pose()
        self.last_pose = pose
        self.last_pose_wall = time.monotonic()
        self.cube_pose_times.append(self.last_pose_wall)
        self.last_detection_tag_count = len(used_tag_ids)
        self.last_reproj_error = reproj_error
        self.observed_cube_ids.update(tag_id for tag_id in used_tag_ids if tag_id in CUBE_FACE_IDS)
        target_text = ";".join(f"{tag_id}/{face}" for tag_id, face in zip(used_tag_ids, used_face_names))
        camera_info_text = "CameraInfo" if self.camera_info_ready else "fallback内参"
        observed = ",".join(str(tag_id) for tag_id in sorted(self.observed_cube_ids)) or "-"
        consistency_text = self._format_multitag_consistency(max_single_rot_deg, max_single_pos_m)
        self.last_tag_warning = (
            f"Cube PnP: {CUBE_LAYOUT_LABEL}，tags={target_text}，count={len(used_tag_ids)}，"
            f"reproj={reproj_error:.2f}px{consistency_text}，已见ID={observed}，"
            f"Cube边长={self.cube_visual_size:.3f}m，使用 {camera_info_text}"
        )
        self._publish_pose_status(
            image_msg.header.stamp,
            True,
            used_tag_ids,
            reproj_error,
            self.last_tag_warning,
        )
        self._publish_tf(pose)
        self._append_pose_csv(pose, len(used_tag_ids), reproj_error, used_tag_ids)
        if display_image is None:
            return None
        return self._draw_cube_overlay(
            display_image,
            image_points_by_tag,
            cube_rvec,
            cube_tvec,
            used_tag_ids,
            used_face_names,
            reproj_error,
            display_scale,
        )

    def _candidate_tag_indices(self, flat_ids: list[int], corners) -> list[int]:
        if self.target_tag_id >= 0:
            try:
                selected_index = flat_ids.index(self.target_tag_id)
            except ValueError:
                self.last_tag_warning = (
                    f"识别到 tag {flat_ids}，但未找到目标 tag_id={self.target_tag_id}"
                )
                return []
            return [selected_index]

        cube_indices = [index for index, tag_id in enumerate(flat_ids) if tag_id in CUBE_FACE_IDS]
        if not cube_indices:
            self.last_tag_warning = f"识别到 tag {flat_ids}，但 Cube 模板需要 ID 0-4"
            return []
        return sorted(cube_indices, key=lambda index: self._tag_area(corners[index]), reverse=True)

    @staticmethod
    def _tag_area(corners) -> float:
        points = np.asarray(corners, dtype=np.float32).reshape(4, 2)
        return float(cv2.contourArea(points)) if cv2 is not None else 0.0

    def _tag_local_corner_points(self) -> np.ndarray:
        half = float(self.marker_size) * 0.5
        return np.array(
            [
                [-half, half, 0.0],
                [half, half, 0.0],
                [half, -half, 0.0],
                [-half, -half, 0.0],
            ],
            dtype=np.float64,
        )

    def _tag_object_points_in_cube(self, tag_id: int) -> tuple[np.ndarray | None, str]:
        tag_points = self._tag_local_corner_points()
        face_transform = self._cube_face_transform(tag_id)
        if face_transform is None:
            if self.target_tag_id >= 0 and int(tag_id) == self.target_tag_id:
                return tag_points, f"id{tag_id}"
            return None, f"id{tag_id}"
        face_name, cube_from_tag_rotation, cube_from_tag_translation = face_transform
        cube_points = tag_points @ cube_from_tag_rotation.T + cube_from_tag_translation
        return cube_points.astype(np.float64), face_name

    def _estimate_cube_pose_from_single_tag(
        self,
        tag_id: int,
        tag_image_points: np.ndarray,
    ) -> tuple[np.ndarray, np.ndarray] | None:
        corners = [np.asarray(tag_image_points, dtype=np.float32).reshape(1, 4, 2)]
        try:
            if hasattr(cv2.aruco, "estimatePoseSingleMarkers"):
                rvecs, tvecs, _ = cv2.aruco.estimatePoseSingleMarkers(
                    corners,
                    float(self.marker_size),
                    self.camera_matrix,
                    self.dist_coeffs,
                )
                tag_rvec = np.asarray(rvecs[0], dtype=np.float64).reshape(3)
                tag_tvec = np.asarray(tvecs[0], dtype=np.float64).reshape(3)
            else:
                success, tag_rvec, tag_tvec = cv2.solvePnP(
                    self._tag_local_corner_points(),
                    np.asarray(tag_image_points, dtype=np.float64).reshape(4, 2),
                    self.camera_matrix,
                    self.dist_coeffs,
                    flags=cv2.SOLVEPNP_IPPE_SQUARE,
                )
                if not success:
                    return None
                tag_rvec = np.asarray(tag_rvec, dtype=np.float64).reshape(3)
                tag_tvec = np.asarray(tag_tvec, dtype=np.float64).reshape(3)
            tag_rotation, _ = cv2.Rodrigues(tag_rvec)
            cube_rotation, cube_translation, _ = self._tag_pose_to_cube_pose(
                tag_id,
                np.asarray(tag_rotation, dtype=np.float64).reshape(3, 3),
                tag_tvec,
            )
            return cube_rotation, cube_translation
        except Exception as exc:  # noqa: BLE001 - keep live GUI running and surface nearby warning.
            self.last_tag_warning = f"单 Tag 初值解算异常：{exc}"
            return None

    def _tag_pose_to_cube_pose(
        self,
        tag_id: int,
        tag_rotation: np.ndarray,
        tag_translation: np.ndarray,
    ) -> tuple[np.ndarray, np.ndarray, str]:
        face_transform = self._cube_face_transform(tag_id)
        if face_transform is None:
            return tag_rotation, tag_translation, f"id{tag_id}"
        face_name, cube_from_tag_rotation, cube_from_tag_translation = face_transform
        cube_rotation = tag_rotation @ cube_from_tag_rotation.T
        u, _, vt = np.linalg.svd(cube_rotation)
        cube_rotation = u @ vt
        if np.linalg.det(cube_rotation) < 0.0:
            u[:, -1] *= -1.0
            cube_rotation = u @ vt
        cube_translation = tag_translation - cube_rotation @ cube_from_tag_translation
        return cube_rotation, cube_translation, face_name

    def _single_tag_pose_consistency(
        self,
        tag_ids: list[int],
        image_points_by_tag: list[np.ndarray],
    ) -> tuple[float, float]:
        if len(tag_ids) < 2:
            return float("nan"), float("nan")

        poses = []
        for tag_id, image_points in zip(tag_ids, image_points_by_tag):
            pose = self._estimate_cube_pose_from_single_tag(tag_id, image_points)
            if pose is None:
                continue
            poses.append((tag_id, pose[0], pose[1]))
        if len(poses) < 2:
            return float("nan"), float("nan")

        _, reference_rotation, reference_translation = poses[0]
        max_rotation_deg = 0.0
        max_translation_m = 0.0
        for _, rotation, translation in poses[1:]:
            delta_rotation = reference_rotation.T @ rotation
            max_rotation_deg = max(max_rotation_deg, rotation_angle_deg(delta_rotation))
            max_translation_m = max(
                max_translation_m,
                float(np.linalg.norm(translation - reference_translation)),
            )
        return max_rotation_deg, max_translation_m

    @staticmethod
    def _format_multitag_consistency(max_rotation_deg: float, max_translation_m: float) -> str:
        if not np.isfinite(max_rotation_deg) or not np.isfinite(max_translation_m):
            return ""
        text = f"，单Tag一致性={max_rotation_deg:.1f}deg/{max_translation_m * 1000.0:.1f}mm"
        if (
            max_rotation_deg > MULTI_TAG_CONSISTENCY_WARN_DEG
            or max_translation_m > MULTI_TAG_CONSISTENCY_WARN_M
        ):
            text += "，多面几何需检查"
        return text

    def _cube_face_transform(self, tag_id: int) -> tuple[str, np.ndarray, np.ndarray] | None:
        cube_size = self.cube_visual_size if self.cube_visual_size > 0.0 else self.marker_size
        if cube_size <= 0.0:
            return None
        half = float(cube_size) * 0.5
        # This is the AprilTag 36h11 "Left Hand Cube" 5-face template:
        # Center=ID0, Top=ID1, Right=ID2, Bottom=ID3, Left=ID4.
        # The transforms map OpenCV AprilTag local marker axes into cube_link at
        # the physical cube center. Keep this synced with 07process_data.
        faces = {
            0: (
                "center",
                np.array([[1.0, 0.0, 0.0], [0.0, -1.0, 0.0], [0.0, 0.0, -1.0]], dtype=np.float64),
                np.array([0.0, 0.0, -half], dtype=np.float64),
            ),
            1: (
                "top",
                np.array([[1.0, 0.0, 0.0], [0.0, 0.0, -1.0], [0.0, 1.0, 0.0]], dtype=np.float64),
                np.array([0.0, -half, 0.0], dtype=np.float64),
            ),
            2: (
                "right",
                np.array([[0.0, 0.0, 1.0], [0.0, -1.0, 0.0], [1.0, 0.0, 0.0]], dtype=np.float64),
                np.array([half, 0.0, 0.0], dtype=np.float64),
            ),
            3: (
                "bottom",
                np.array([[1.0, 0.0, 0.0], [0.0, 0.0, 1.0], [0.0, -1.0, 0.0]], dtype=np.float64),
                np.array([0.0, half, 0.0], dtype=np.float64),
            ),
            4: (
                "left",
                np.array([[0.0, 0.0, -1.0], [0.0, -1.0, 0.0], [-1.0, 0.0, 0.0]], dtype=np.float64),
                np.array([-half, 0.0, 0.0], dtype=np.float64),
            ),
        }
        return faces.get(int(tag_id))

    def _reprojection_rmse(
        self,
        object_points: np.ndarray,
        image_points: np.ndarray,
        rvec: np.ndarray,
        tvec: np.ndarray,
    ) -> float:
        projected = self._project_points(object_points, rvec, tvec)
        residual = projected - np.asarray(image_points, dtype=np.float64).reshape(-1, 2)
        return float(np.sqrt(np.mean(np.sum(residual * residual, axis=1))))

    def _project_points(
        self,
        object_points: np.ndarray,
        rvec: np.ndarray,
        tvec: np.ndarray,
    ) -> np.ndarray:
        projected, _ = cv2.projectPoints(
            np.asarray(object_points, dtype=np.float64),
            np.asarray(rvec, dtype=np.float64).reshape(3, 1),
            np.asarray(tvec, dtype=np.float64).reshape(3, 1),
            self.camera_matrix,
            self.dist_coeffs,
        )
        return projected.reshape(-1, 2)

    @staticmethod
    def _valid_pixel_points(points: np.ndarray) -> bool:
        return bool(np.all(np.isfinite(points)) and points.size >= 4)

    def _draw_cube_overlay(
        self,
        image: QImage,
        tag_corners_by_tag: list[np.ndarray],
        rvec: np.ndarray,
        tvec: np.ndarray,
        tag_ids: list[int],
        face_names: list[str],
        reproj_error: float,
        display_scale: float = 1.0,
    ) -> QImage:
        overlay = image.convertToFormat(QImage.Format_RGB888).copy()
        painter = QPainter(overlay)
        painter.setRenderHint(QPainter.Antialiasing)
        scale = float(display_scale) if np.isfinite(display_scale) and display_scale > 0.0 else 1.0

        def draw_polyline(points: np.ndarray, color: str, width: float, closed: bool = False, dash: bool = False) -> None:
            if not self._valid_pixel_points(points):
                return
            pen = QPen(QColor(color), width)
            pen.setCapStyle(Qt.RoundCap)
            pen.setJoinStyle(Qt.RoundJoin)
            if dash:
                pen.setStyle(Qt.DashLine)
            painter.setPen(pen)
            count = len(points)
            limit = count if closed else count - 1
            for index in range(limit):
                a = points[index]
                b = points[(index + 1) % count]
                painter.drawLine(QPointF(float(a[0]), float(a[1])), QPointF(float(b[0]), float(b[1])))

        def draw_axis_arrow(origin: np.ndarray, point: np.ndarray, color: str, name: str) -> None:
            start = QPointF(float(origin[0]), float(origin[1]))
            end = QPointF(float(point[0]), float(point[1]))
            painter.setPen(QPen(QColor(color), 3.0, Qt.SolidLine, Qt.RoundCap, Qt.RoundJoin))
            painter.drawLine(start, end)

            vector = np.array([end.x() - start.x(), end.y() - start.y()], dtype=float)
            norm = float(np.linalg.norm(vector))
            if norm > 1e-6:
                direction = vector / norm
                normal = np.array([-direction[1], direction[0]])
                size = 10.0
                p1 = np.array([end.x(), end.y()]) - direction * size + normal * size * 0.45
                p2 = np.array([end.x(), end.y()]) - direction * size - normal * size * 0.45
                head = QPainterPath()
                head.moveTo(end)
                head.lineTo(QPointF(float(p1[0]), float(p1[1])))
                head.lineTo(QPointF(float(p2[0]), float(p2[1])))
                head.closeSubpath()
                painter.fillPath(head, QColor(color))

            painter.setPen(QColor(color))
            painter.setFont(QFont("Sans Serif", 11, QFont.Bold))
            painter.drawText(QPointF(float(point[0]) + 5.0, float(point[1]) - 5.0), name)

        corners_2d_by_tag = [
            np.asarray(tag_corners, dtype=np.float64).reshape(-1, 2) * scale
            for tag_corners in tag_corners_by_tag
        ]
        for corners_2d in corners_2d_by_tag:
            draw_polyline(corners_2d, "#12b76a", 3.0, closed=True)

        cube_size = self.cube_visual_size if self.cube_visual_size > 0.0 else self.marker_size
        half = float(cube_size) * 0.5
        cube_points = np.array(
            [
                [-half, -half, -half],
                [half, -half, -half],
                [half, half, -half],
                [-half, half, -half],
                [-half, -half, half],
                [half, -half, half],
                [half, half, half],
                [-half, half, half],
            ],
            dtype=np.float64,
        )
        cube_2d = self._project_points(cube_points, rvec, tvec) * scale
        draw_polyline(cube_2d[[0, 1, 2, 3]], "#06aed5", 1.4, closed=True, dash=True)
        draw_polyline(cube_2d[[4, 5, 6, 7]], "#06aed5", 2.0, closed=True)
        for front_index, back_index in ((0, 4), (1, 5), (2, 6), (3, 7)):
            draw_polyline(np.vstack([cube_2d[front_index], cube_2d[back_index]]), "#06aed5", 1.4)

        axis = max(float(self.marker_size) * 0.75, 1e-4)
        axis_points = np.array(
            [[0.0, 0.0, 0.0], [axis, 0.0, 0.0], [0.0, axis, 0.0], [0.0, 0.0, axis]],
            dtype=np.float64,
        )
        axis_2d = self._project_points(axis_points, rvec, tvec) * scale
        if self._valid_pixel_points(axis_2d):
            origin = axis_2d[0]
            for point, color, name in (
                (axis_2d[1], "#f04438", "+X"),
                (axis_2d[2], "#12b76a", "+Y"),
                (axis_2d[3], "#2e90fa", "+Z"),
            ):
                draw_axis_arrow(origin, point, color, name)

        label_position = corners_2d_by_tag[0][0]
        tag_text = ";".join(f"{tag_id}/{face}" for tag_id, face in zip(tag_ids, face_names))
        painter.setPen(QColor("#101828"))
        painter.setFont(QFont("Sans Serif", 10, QFont.Bold))
        painter.drawText(
            QPointF(float(label_position[0]) + 6.0, float(label_position[1]) - 8.0),
            f"{CUBE_LAYOUT_LABEL} tags={tag_text}  reproj={reproj_error:.2f}px  z={float(tvec[2]):.3f}m",
        )
        painter.setPen(QColor("#344054"))
        painter.setFont(QFont("Sans Serif", 9, QFont.Bold))
        painter.drawText(
            QPointF(float(label_position[0]) + 6.0, float(label_position[1]) + 10.0),
            "+X->ID2/right  +Y->ID3/bottom  +Z->Cube内",
        )
        painter.end()
        max_rotation_deg, max_translation_m = self.last_multitag_consistency
        consistency_text = self._format_multitag_consistency(max_rotation_deg, max_translation_m)
        self.last_overlay_info = (
            f"叠加显示：{CUBE_LAYOUT_LABEL}，{len(tag_ids)} 个 Tag 估计 Cube 位姿，"
            f"reproj={reproj_error:.2f}px{consistency_text}，Cube边长={cube_size:.3f}m"
        )
        return overlay

    def _publish_tf(self, pose: CubePose) -> None:
        if self.tf_broadcaster is None:
            return
        transform = TransformStamped()
        transform.header.stamp = pose.header.stamp
        transform.header.frame_id = self.camera_frame
        transform.child_frame_id = self.cube_frame
        transform.transform.translation.x = pose.translation.x
        transform.transform.translation.y = pose.translation.y
        transform.transform.translation.z = pose.translation.z
        transform.transform.rotation = pose.orientation
        self.tf_broadcaster.sendTransform(transform)

    def _append_pose_csv(
        self,
        pose: CubePose,
        tag_count: int,
        reproj_error: float,
        tag_ids: list[int],
    ) -> None:
        path = self.pose_csv_path.strip()
        if not path:
            return
        try:
            header = "timestamp,tx,ty,tz,qx,qy,qz,qw,tag_count,reproj_error,tag_ids\n"
            if path != self.pose_csv_open_path:
                if self.pose_csv_file is not None:
                    self.pose_csv_file.close()
                directory = os.path.dirname(os.path.abspath(path))
                if directory:
                    os.makedirs(directory, exist_ok=True)
                new_file = not os.path.exists(path) or os.path.getsize(path) == 0
                old_header = ""
                if not new_file:
                    with open(path, "r", encoding="utf-8") as existing_file:
                        old_header = existing_file.readline().strip()
                self.pose_csv_file = open(path, "a", encoding="utf-8", buffering=1)
                self.pose_csv_open_path = path
                if new_file or old_header != header.strip():
                    self.pose_csv_file.write(header)
            tag_id_text = ";".join(str(tag_id) for tag_id in tag_ids)
            self.pose_csv_file.write(
                f"{stamp_to_float(pose.header.stamp):.9f},"
                f"{pose.translation.x:.12g},{pose.translation.y:.12g},{pose.translation.z:.12g},"
                f"{pose.orientation.x:.12g},{pose.orientation.y:.12g},"
                f"{pose.orientation.z:.12g},{pose.orientation.w:.12g},"
                f"{int(tag_count)},{float(reproj_error):.6g},{tag_id_text}\n"
            )
        except Exception as exc:  # noqa: BLE001 - keep GUI alive and surface CSV errors.
            self.last_service_message = f"位姿 CSV 写入失败：{exc}"

    def _imu_cb(self, msg: Imu) -> None:
        now = time.monotonic()
        stamp, issue = self._checked_stamp("imu", msg.header.stamp, self.last_imu_stamp)
        self.last_imu_stamp = stamp
        self.last_imu_stamp_issue = issue
        if stamp <= 0.0:
            stamp = now
        accel = np.array(
            [
                float(msg.linear_acceleration.x),
                float(msg.linear_acceleration.y),
                float(msg.linear_acceleration.z),
            ],
            dtype=float,
        )
        gyro = np.array(
            [
                float(msg.angular_velocity.x),
                float(msg.angular_velocity.y),
                float(msg.angular_velocity.z),
            ],
            dtype=float,
        )
        accel_g = accel / GRAVITY_M_S2
        gyro_deg_s = gyro * RAD_TO_DEG
        sample = (
            stamp,
            float(accel[0]),
            float(accel[1]),
            float(accel[2]),
            float(gyro[0]),
            float(gyro[1]),
            float(gyro[2]),
            now,
        )
        self.imu_samples.append(sample)
        self.imu_times.append(now)
        self.last_imu_wall = now
        self.last_imu_info = (
            f"acc=[{sample[1]:.3f}, {sample[2]:.3f}, {sample[3]:.3f}] m/s^2   "
            f"gyro=[{sample[4]:.3f}, {sample[5]:.3f}, {sample[6]:.3f}] rad/s"
        )
        self.last_imu_accel_g_info = fmt_vec(accel_g, 3)
        self.last_imu_gyro_deg_info = fmt_vec(gyro_deg_s, 2)
        self.last_imu_norm_info = (
            f"|a|={float(np.linalg.norm(accel_g)):.3f} g   "
            f"|w|={float(np.linalg.norm(gyro_deg_s)):.2f} deg/s"
        )

    @staticmethod
    def _rate_hz(times) -> float:
        if len(times) < 2:
            return 0.0
        span = float(times[-1] - times[0])
        if span <= 1e-6:
            return 0.0
        return float(len(times) - 1) / span

    def image_rate_hz(self) -> float:
        return self._rate_hz(self.image_times)

    def cube_pose_rate_hz(self) -> float:
        return self._rate_hz(self.cube_pose_times)

    def imu_rate_hz(self) -> float:
        return self._rate_hz(self.imu_times)

    def call_trigger(self, client, action_name: str) -> None:
        if not client.service_is_ready():
            self.last_service_message = f"{action_name}失败：服务不可用"
            return

        future = client.call_async(Trigger.Request())

        def _done(done_future) -> None:
            try:
                result = done_future.result()
                prefix = f"{action_name}成功" if result.success else f"{action_name}失败"
                self.last_service_message = f"{prefix}：{result.message}"
            except Exception as exc:  # noqa: BLE001 - GUI should surface any service error.
                self.last_service_message = f"{action_name}异常：{exc}"

        future.add_done_callback(_done)
        self.last_service_message = f"{action_name}请求已发送"

    def start_recording(self) -> None:
        path = self.direct_recorder.start(
            self.bag_path,
            self.record_duration_sec,
            self.camera_matrix,
            self.dist_coeffs,
        )
        if self.pose_csv_file is not None:
            self.pose_csv_file.close()
            self.pose_csv_file = None
        self.pose_csv_open_path = ""
        self.pose_csv_path = os.path.join(path, "tag_pose.csv")
        self.last_status = self.direct_recorder.status(self.direct_sensor.stats(), self.camera_info_ready)
        self.last_status_wall = time.monotonic()
        self.last_service_message = f"开始直连录制：{path}"

    def stop_recording(self) -> None:
        self.direct_recorder.stop()
        self.last_status = self.direct_recorder.status(self.direct_sensor.stats(), self.camera_info_ready)
        self.last_status_wall = time.monotonic()
        self.last_service_message = f"停止直连录制：{self.direct_recorder.path}"

    def spin_once(self) -> None:
        for _ in range(5):
            rclpy.spin_once(self.node, timeout_sec=0.0)
        self._poll_direct_sensor()

    def shutdown(self) -> None:
        self.direct_recorder.stop()
        self.stop_direct_sensor()
        if self.direct_start_thread is not None and self.direct_start_thread.is_alive():
            self.direct_start_thread.join(timeout=1.0)
        self.direct_sensor.close()
        if self.pose_csv_file is not None:
            self.pose_csv_file.close()
            self.pose_csv_file = None
        self.node.destroy_node()


class StatusValue(QLabel):
    def __init__(self, text: str = "-") -> None:
        super().__init__(text)
        self.setTextInteractionFlags(Qt.TextSelectableByMouse)
        self.setMinimumHeight(24)
        self.setStyleSheet(
            "QLabel { padding: 3px 7px; border: 1px solid #d0d5dd; border-radius: 4px; }"
        )

    def set_state(self, text: str, state: str = "neutral", tooltip: str | None = None) -> None:
        colors = {
            "ok": ("#ecfdf3", "#027a48", "#abefc6"),
            "warn": ("#fffaeb", "#b54708", "#fedf89"),
            "bad": ("#fef3f2", "#b42318", "#fecdca"),
            "neutral": ("#f8fafc", "#344054", "#d0d5dd"),
        }
        bg, fg, border = colors.get(state, colors["neutral"])
        self.setText(text)
        self.setToolTip(text if tooltip is None else tooltip)
        self.setStyleSheet(
            "QLabel {"
            f"background: {bg}; color: {fg}; border: 1px solid {border};"
            "border-radius: 4px; padding: 3px 7px;"
            "}"
        )


class HeaderStatusValue(StatusValue):
    FIXED_WIDTH = 580
    FIXED_HEIGHT = 42

    def __init__(self, text: str = "-") -> None:
        super().__init__(text)
        self.setFixedSize(self.FIXED_WIDTH, self.FIXED_HEIGHT)
        self.setSizePolicy(QSizePolicy.Fixed, QSizePolicy.Fixed)
        self.setWordWrap(False)
        self.setAlignment(Qt.AlignVCenter | Qt.AlignLeft)
        self.setTextInteractionFlags(Qt.NoTextInteraction)
        self.setFont(QFont("Sans Serif", 9, QFont.Medium))

    def set_state(self, text: str, state: str = "neutral", tooltip: str | None = None) -> None:
        colors = {
            "ok": ("#ecfdf3", "#027a48", "#abefc6", "就绪"),
            "warn": ("#fffaeb", "#b54708", "#fedf89", "等待"),
            "bad": ("#fef3f2", "#b42318", "#fecdca", "异常"),
            "neutral": ("#f8fafc", "#344054", "#d0d5dd", "状态"),
        }
        bg, fg, border, label = colors.get(state, colors["neutral"])
        raw_text = " ".join(str(text).split())
        display = f"{label} | {raw_text}"
        metrics = QFontMetrics(self.font())
        self.setText(metrics.elidedText(display, Qt.ElideRight, self.width() - 28))
        self.setToolTip(str(text) if tooltip is None else tooltip)
        self.setStyleSheet(
            "QLabel {"
            f"background: {bg}; color: {fg}; border: 1px solid {border};"
            "border-radius: 7px; padding: 0 12px;"
            "}"
        )


class StablePreviewLabel(QLabel):
    def sizeHint(self):  # noqa: N802 - Qt override name.
        return self.minimumSize()

    def minimumSizeHint(self):  # noqa: N802 - Qt override name.
        return self.minimumSize()


class ImuPlotWidget(QWidget):
    def __init__(self) -> None:
        super().__init__()
        self.samples = []
        self.window_sec = 2.0
        self.range_ready = [False] * 6
        self.range_min = [0.0] * 6
        self.range_max = [0.0] * 6
        self.setMinimumSize(360, 540)
        self.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Expanding)
        self.setStyleSheet("background: #ffffff;")

    def set_samples(self, samples) -> None:
        self.samples = list(samples)
        self.update()

    def paintEvent(self, event) -> None:  # noqa: N802 - Qt API name.
        del event
        painter = QPainter(self)
        painter.setRenderHint(QPainter.Antialiasing)

        painter.fillRect(self.rect(), QColor("#ffffff"))
        area = QRectF(self.rect()).adjusted(12, 10, -12, -12)
        painter.setPen(QColor("#101828"))
        painter.setFont(QFont("Sans Serif", 10, QFont.Medium))
        painter.drawText(area.adjusted(0, 0, 0, -area.height() + 24), Qt.AlignLeft, "IMU 六轴规范化波形")
        painter.setPen(QColor("#667085"))
        painter.setFont(QFont("Sans Serif", 8))
        painter.drawText(
            area.adjusted(0, 0, 0, -area.height() + 24),
            Qt.AlignRight,
            f"横向自适应，约 {self.window_sec:.1f}s",
        )

        if len(self.samples) < 2:
            painter.setPen(QColor("#667085"))
            painter.drawText(area, Qt.AlignCenter, "等待 IMU 数据")
            return

        plot_top = area.top() + 34.0
        gap = 8.0
        plot_count = 6
        plot_height = max(46.0, (area.bottom() - plot_top - gap * (plot_count - 1)) / plot_count)
        specs = (
            ("加速度 X", "g", 1, 1.2, "#ff6052", 1.0 / GRAVITY_M_S2),
            ("加速度 Y", "g", 2, 1.2, "#69e27a", 1.0 / GRAVITY_M_S2),
            ("加速度 Z", "g", 3, 1.2, "#4cb2ff", 1.0 / GRAVITY_M_S2),
            ("角速度 X", "deg/s", 4, 5.0, "#ffb54c", RAD_TO_DEG),
            ("角速度 Y", "deg/s", 5, 5.0, "#bb84ff", RAD_TO_DEG),
            ("角速度 Z", "deg/s", 6, 5.0, "#52e2e0", RAD_TO_DEG),
        )
        for index, spec in enumerate(specs):
            rect = QRectF(
                area.left(),
                plot_top + index * (plot_height + gap),
                area.width(),
                plot_height,
            )
            self._draw_scalar_panel(painter, rect, index, *spec)

    def _stabilized_range(
        self,
        plot_index: int,
        target_min: float,
        target_max: float,
        fallback_span: float,
    ) -> tuple[float, float]:
        if plot_index < 0 or plot_index >= len(self.range_ready) or not target_max > target_min:
            return target_min, target_max

        min_span = max(1e-6, fallback_span * 0.08)
        target_span = max(min_span, target_max - target_min)
        target_center = 0.5 * (target_min + target_max)
        if not self.range_ready[plot_index]:
            self.range_ready[plot_index] = True
            self.range_min[plot_index] = target_center - 0.5 * target_span
            self.range_max[plot_index] = target_center + 0.5 * target_span
            return self.range_min[plot_index], self.range_max[plot_index]

        current_min = self.range_min[plot_index]
        current_max = self.range_max[plot_index]
        current_span = max(min_span, current_max - current_min)
        current_center = 0.5 * (current_min + current_max)
        next_center = current_center * 0.92 + target_center * 0.08
        next_span = current_span
        if target_min < current_min or target_max > current_max:
            next_span = max(current_span, target_span * 1.15)
            next_center = target_center
        elif target_span < current_span * 0.65:
            next_span = current_span * 0.97 + target_span * 0.03

        self.range_min[plot_index] = next_center - 0.5 * next_span
        self.range_max[plot_index] = next_center + 0.5 * next_span
        return self.range_min[plot_index], self.range_max[plot_index]

    def _draw_scalar_panel(
        self,
        painter: QPainter,
        rect: QRectF,
        plot_index: int,
        title: str,
        unit: str,
        value_index: int,
        min_scale: float,
        color: str,
        value_scale: float,
    ) -> None:
        values = np.array([sample[value_index] * value_scale for sample in self.samples], dtype=float)
        times = np.array([sample[0] for sample in self.samples], dtype=float)
        finite_time = np.isfinite(times)
        if not np.any(finite_time):
            return
        t_max = float(times[finite_time][-1])
        visible_mask = finite_time & (times >= t_max - self.window_sec) & np.isfinite(values)
        if not np.any(visible_mask):
            visible_mask = finite_time & np.isfinite(values)
        visible_times = times[visible_mask]
        visible_values = values[visible_mask]
        finite_values = visible_values[np.isfinite(visible_values)]
        if finite_values.size == 0:
            value_min = -min_scale
            value_max = min_scale
        else:
            center = float(np.median(finite_values))
            deviations = np.abs(finite_values - center)
            robust_span = float(np.percentile(deviations, 95)) if deviations.size else 0.0
            norm_floor = 0.05 if unit == "g" else 2.0
            half_span = max(robust_span * 1.35, norm_floor)
            value_min = center - half_span
            value_max = center + half_span
        value_min, value_max = self._stabilized_range(
            plot_index,
            value_min,
            value_max,
            max(1e-6, min_scale * 2.0),
        )

        painter.fillRect(rect, QColor("#f8fafc"))
        painter.setPen(QPen(QColor("#d0d5dd"), 1))
        painter.drawRect(rect.adjusted(0, 0, -1, -1))
        painter.setPen(QColor("#344054"))
        painter.setFont(QFont("Sans Serif", 8, QFont.Medium))
        painter.drawText(rect.adjusted(8, 3, -110, 0), Qt.AlignTop | Qt.AlignLeft, f"{title} [{unit}]")

        plot = rect.adjusted(54, 24, -70, -10)
        if plot.width() <= 2 or plot.height() <= 2:
            return

        for tick in range(3):
            y = plot.top() + plot.height() * tick / 2.0
            value = value_max - (value_max - value_min) * tick / 2.0
            painter.setPen(QColor("#98a2b3") if tick == 1 else QColor("#eaecf0"))
            painter.drawLine(QPointF(plot.left(), y), QPointF(plot.right(), y))
            painter.setPen(QColor("#667085"))
            painter.setFont(QFont("Sans Serif", 7))
            painter.drawText(
                QRectF(rect.left() + 4, y - 8, 46, 16),
                Qt.AlignRight | Qt.AlignVCenter,
                f"{value:.2f}" if abs(value) < 10.0 else f"{value:.1f}",
            )

        t_min = float(visible_times[0])
        t_end = float(visible_times[-1])
        time_span = max(t_end - t_min, 1e-6)

        path = QPainterPath()
        started = False
        for sample_index in np.flatnonzero(visible_mask):
            value = float(values[sample_index])
            if not np.isfinite(value):
                continue
            clipped = float(np.clip(value, value_min, value_max))
            x_ratio = np.clip((times[sample_index] - t_min) / time_span, 0.0, 1.0)
            y_ratio = np.clip((value_max - clipped) / max(value_max - value_min, 1e-9), 0.0, 1.0)
            point = QPointF(plot.left() + x_ratio * plot.width(), plot.top() + y_ratio * plot.height())
            if not started:
                path.moveTo(point)
                started = True
            else:
                path.lineTo(point)
        painter.setPen(QPen(QColor(color), 1.7))
        painter.drawPath(path)

        latest = float(values[-1])
        painter.setPen(QColor(color))
        painter.setFont(QFont("Sans Serif", 8, QFont.Medium))
        painter.drawText(
            rect.adjusted(0, 4, -8, 0),
            Qt.AlignRight | Qt.AlignTop,
            f"{latest:+.3f}" if abs(latest) < 10.0 else f"{latest:+.1f}",
        )


class MotionGuideWidget(QWidget):
    def __init__(self) -> None:
        super().__init__()
        self.phase = -1
        self.recording = False
        self.pose_visible = False
        self.detection_enabled = False
        self.progress = 0.0
        self.time_text = ""
        self.setMinimumHeight(160)
        self.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Expanding)

    def set_guide(
        self,
        phase: int,
        recording: bool,
        pose_visible: bool,
        progress: float,
        time_text: str = "",
        detection_enabled: bool = False,
    ) -> None:
        self.phase = int(phase)
        self.recording = bool(recording)
        self.pose_visible = bool(pose_visible)
        self.detection_enabled = bool(detection_enabled)
        self.progress = max(0.0, min(1.0, float(progress)))
        self.time_text = str(time_text)
        self.update()

    def paintEvent(self, event) -> None:  # noqa: N802 - Qt API name.
        del event
        painter = QPainter(self)
        painter.setRenderHint(QPainter.Antialiasing)
        painter.fillRect(self.rect(), QColor("#f8fafc"))
        area = QRectF(self.rect()).adjusted(10, 8, -10, -8)

        border = "#abefc6" if self.pose_visible else "#fedf89"
        painter.setPen(QPen(QColor(border), 1.2))
        painter.setBrush(QColor("#ffffff"))
        painter.drawRoundedRect(area, 5, 5)

        title = "采集前：带 Tag 的 IMU 模块放入画面中央" if not self.recording else self._phase_title()
        painter.setPen(QColor("#101828"))
        painter.setFont(QFont("Sans Serif", 9, QFont.Bold))
        painter.drawText(area.adjusted(12, 7, -12, 0), Qt.AlignTop | Qt.AlignLeft, title)

        view = self._guide_view(area)
        self._draw_camera_frame(painter, view)
        self._draw_axis_legend(painter, view)
        self._draw_cube(painter, view)
        self._draw_phase_motion(painter, view)
        self._draw_progress(painter, area)

    def _phase_title(self) -> str:
        titles = {
            0: "1/6 居中静止",
            1: "2/6 绕 +X 轴（ID2/right）",
            2: "3/6 绕 +Y 轴（ID3/bottom）",
            3: "4/6 绕 +Z 轴（Cube内）",
            4: "5/6 平移 + 组合旋转",
            5: "6/6 八字/斜向运动",
            6: "向导完成",
        }
        return titles.get(self.phase, "动作向导")

    @staticmethod
    def _guide_view(area: QRectF) -> QRectF:
        return QRectF(area.left() + 14, area.top() + 30, area.width() - 28, area.height() - 52)

    def _draw_camera_frame(self, painter: QPainter, area: QRectF) -> None:
        painter.setPen(QPen(QColor("#d0d5dd"), 1.1, Qt.DashLine))
        painter.setBrush(QColor("#f8fafc"))
        painter.drawRoundedRect(area, 3, 3)

    def _cube_points(self, area: QRectF) -> tuple[list[QPointF], list[QPointF], list[QPointF]]:
        center = QPointF(area.center().x(), area.center().y() + 8)
        w = min(area.width() * 0.25, 92.0)
        h = w * 0.62
        dx = w * 0.30
        dy = h * 0.34
        front = [
            QPointF(center.x() - w * 0.5, center.y() - h * 0.5),
            QPointF(center.x() + w * 0.5, center.y() - h * 0.5),
            QPointF(center.x() + w * 0.5, center.y() + h * 0.5),
            QPointF(center.x() - w * 0.5, center.y() + h * 0.5),
        ]
        top = [
            QPointF(front[0].x(), front[0].y()),
            QPointF(front[1].x(), front[1].y()),
            QPointF(front[1].x() + dx, front[1].y() - dy),
            QPointF(front[0].x() + dx, front[0].y() - dy),
        ]
        side = [
            QPointF(front[1].x(), front[1].y()),
            QPointF(front[2].x(), front[2].y()),
            QPointF(front[2].x() + dx, front[2].y() - dy),
            QPointF(front[1].x() + dx, front[1].y() - dy),
        ]
        return front, top, side

    def _draw_polygon(self, painter: QPainter, points: list[QPointF], fill: str, outline: str) -> None:
        path = QPainterPath()
        path.moveTo(points[0])
        for point in points[1:]:
            path.lineTo(point)
        path.closeSubpath()
        painter.fillPath(path, QColor(fill))
        painter.setPen(QPen(QColor(outline), 1.4))
        painter.drawPath(path)

    def _draw_cube(self, painter: QPainter, area: QRectF) -> None:
        center = QPointF(area.center().x() - min(28.0, area.width() * 0.05), area.center().y() + 4)
        painter.save()
        painter.translate(center)
        painter.scale(1.5, 1.5)

        strap = QPainterPath()
        strap.moveTo(QPointF(-32, 18))
        strap.cubicTo(
            QPointF(-38, 40),
            QPointF(34, 42),
            QPointF(30, 18),
        )
        painter.setPen(QPen(QColor("#101828"), 5.2, Qt.SolidLine, Qt.RoundCap))
        painter.drawPath(strap)
        painter.setPen(QPen(QColor("#667085"), 1.2))
        for offset in (-18, 0, 18):
            painter.drawEllipse(QPointF(offset, 31), 1.8, 1.8)

        imu_body = QRectF(-42, -6, 70, 26)
        painter.setPen(QPen(QColor("#475467"), 1.2))
        painter.setBrush(QColor("#101828"))
        painter.drawRoundedRect(imu_body, 5, 5)
        painter.fillRect(imu_body.adjusted(8, 7, -45, -8), QColor("#98a2b3"))
        painter.setPen(QColor("#ffffff"))
        painter.setFont(QFont("Sans Serif", 8, QFont.Bold))
        painter.drawText(imu_body, Qt.AlignCenter, "IMU")

        tag_center = QPointF(28, -26)
        tag_side = min(max(area.width() * 0.10, 34.0), 46.0)
        painter.save()
        painter.translate(tag_center)
        painter.rotate(-18.0)
        tag_rect = QRectF(-tag_side * 0.5, -tag_side * 0.5, tag_side, tag_side)
        painter.setPen(QPen(QColor("#06aed5"), 2.0))
        painter.setBrush(QColor("#101828"))
        painter.drawRect(tag_rect)
        cell = tag_side / 6.0
        white_cells = ((1, 1), (3, 1), (4, 2), (1, 3), (2, 3), (3, 4), (4, 4))
        painter.setPen(Qt.NoPen)
        painter.setBrush(QColor("#f8fafc"))
        for col, row in white_cells:
            painter.drawRect(QRectF(
                tag_rect.left() + col * cell,
                tag_rect.top() + row * cell,
                cell * 0.92,
                cell * 0.92,
            ))
        painter.setPen(QPen(QColor("#f04438"), 2.0, Qt.SolidLine, Qt.RoundCap))
        self._draw_arrow(painter, QPointF(0, 0), QPointF(tag_side * 0.42, 0), "#f04438", 2.0)
        painter.setPen(QPen(QColor("#12b76a"), 2.0, Qt.SolidLine, Qt.RoundCap))
        self._draw_arrow(painter, QPointF(0, 0), QPointF(0, tag_side * 0.42), "#12b76a", 2.0)
        painter.setPen(QPen(QColor("#2e90fa"), 2.0))
        painter.setBrush(QColor("#2e90fa"))
        painter.drawEllipse(QPointF(0, 0), 3.0, 3.0)
        painter.setPen(QColor("#f04438"))
        painter.setFont(QFont("Sans Serif", 7, QFont.Bold))
        painter.drawText(QPointF(tag_side * 0.24, -3.0), "+X")
        painter.setPen(QColor("#12b76a"))
        painter.drawText(QPointF(3.0, tag_side * 0.36), "+Y")
        painter.setPen(QColor("#2e90fa"))
        painter.drawText(QPointF(-tag_side * 0.48, -tag_side * 0.28), "+Z入")
        painter.restore()
        painter.restore()

    def _draw_arrow(
        self,
        painter: QPainter,
        start: QPointF,
        end: QPointF,
        color: str,
        width: float = 2.4,
    ) -> None:
        painter.setPen(QPen(QColor(color), width, Qt.SolidLine, Qt.RoundCap, Qt.RoundJoin))
        painter.drawLine(start, end)
        vector = np.array([end.x() - start.x(), end.y() - start.y()], dtype=float)
        norm = float(np.linalg.norm(vector))
        if norm < 1e-6:
            return
        direction = vector / norm
        normal = np.array([-direction[1], direction[0]])
        size = 9.0
        p1 = np.array([end.x(), end.y()]) - direction * size + normal * size * 0.45
        p2 = np.array([end.x(), end.y()]) - direction * size - normal * size * 0.45
        head = QPainterPath()
        head.moveTo(end)
        head.lineTo(QPointF(float(p1[0]), float(p1[1])))
        head.lineTo(QPointF(float(p2[0]), float(p2[1])))
        head.closeSubpath()
        painter.fillPath(head, QColor(color))

    def _draw_arc_arrow(self, painter: QPainter, rect: QRectF, start_angle: int, span_angle: int, color: str) -> None:
        painter.setPen(QPen(QColor(color), 2.5, Qt.SolidLine, Qt.RoundCap))
        painter.drawArc(rect, start_angle * 16, span_angle * 16)
        angle = np.deg2rad(start_angle + span_angle)
        rx = rect.width() * 0.5
        ry = rect.height() * 0.5
        end = QPointF(rect.center().x() + rx * np.cos(angle), rect.center().y() - ry * np.sin(angle))
        tangent = QPointF(-np.sin(angle) * np.sign(span_angle), -np.cos(angle) * np.sign(span_angle))
        start = QPointF(end.x() - tangent.x() * 16.0, end.y() - tangent.y() * 16.0)
        self._draw_arrow(painter, start, end, color, 0.1)

    def _draw_phase_motion(self, painter: QPainter, area: QRectF) -> None:
        center = QPointF(area.center().x() - min(28.0, area.width() * 0.05), area.center().y() + 4)
        painter.save()
        painter.translate(center)
        painter.scale(1.5, 1.5)
        if not self.recording:
            self._draw_arrow(painter, QPointF(-90, 0), QPointF(-50, 0), "#2e90fa")
            self._draw_arrow(painter, QPointF(90, 0), QPointF(50, 0), "#2e90fa")
            painter.restore()
            return

        if self.phase == 0:
            painter.setPen(QPen(QColor("#12b76a"), 2.0))
            painter.drawEllipse(QPointF(0, 0), 38, 30)
            painter.setFont(QFont("Sans Serif", 8, QFont.Bold))
            painter.drawText(QRectF(-55, 36, 110, 18), Qt.AlignCenter, "静止")
        elif self.phase == 1:
            self._draw_arc_arrow(painter, QRectF(-70, -42, 140, 84), 25, 250, "#f04438")
            painter.setPen(QColor("#f04438"))
            painter.setFont(QFont("Sans Serif", 8, QFont.Bold))
            painter.drawText(QRectF(-60, 34, 120, 30), Qt.AlignCenter, "绕 +X\nID2/right")
        elif self.phase == 2:
            self._draw_arc_arrow(painter, QRectF(-44, -58, 88, 116), -65, 250, "#12b76a")
            painter.setPen(QColor("#12b76a"))
            painter.setFont(QFont("Sans Serif", 8, QFont.Bold))
            painter.drawText(QRectF(-70, 34, 140, 30), Qt.AlignCenter, "绕 +Y\nID3/bottom")
        elif self.phase == 3:
            self._draw_arc_arrow(painter, QRectF(-58, -58, 116, 116), 210, -270, "#2e90fa")
            painter.setPen(QColor("#2e90fa"))
            painter.setFont(QFont("Sans Serif", 8, QFont.Bold))
            painter.drawText(QRectF(-58, 36, 116, 28), Qt.AlignCenter, "绕 +Z\n入内")
        elif self.phase == 4:
            self._draw_arrow(painter, QPointF(-100, 42), QPointF(-46, 14), "#f04438")
            self._draw_arrow(painter, QPointF(100, -42), QPointF(50, -14), "#12b76a")
            self._draw_arc_arrow(painter, QRectF(-66, -52, 132, 104), 200, 220, "#2e90fa")
        elif self.phase == 5:
            path = QPainterPath()
            path.moveTo(QPointF(-74, 0))
            path.cubicTo(
                QPointF(-38, -46),
                QPointF(38, -46),
                QPointF(74, 0),
            )
            path.cubicTo(
                QPointF(38, 46),
                QPointF(-38, 46),
                QPointF(-74, 0),
            )
            painter.setPen(QPen(QColor("#7a5af8"), 2.6, Qt.SolidLine, Qt.RoundCap))
            painter.drawPath(path)
            self._draw_arrow(painter, QPointF(50, -20), QPointF(76, 0), "#7a5af8")
        else:
            painter.setPen(QPen(QColor("#12b76a"), 3.0, Qt.SolidLine, Qt.RoundCap, Qt.RoundJoin))
            painter.drawLine(QPointF(-38, 0), QPointF(-8, 28))
            painter.drawLine(QPointF(-8, 28), QPointF(46, -36))
        painter.restore()

    def _draw_axis_legend(self, painter: QPainter, area: QRectF) -> None:
        legend = QRectF(area.right() - 102, area.top() + 7, 92, 58)
        painter.setFont(QFont("Sans Serif", 8, QFont.Bold))
        for index, (text, color) in enumerate((
            ("+X ID2/right", "#f04438"),
            ("+Y ID3/bottom", "#12b76a"),
            ("+Z 入内", "#2e90fa"),
        )):
            row = QRectF(legend.left(), legend.top() + index * 19, legend.width(), 16)
            painter.setPen(QColor(color))
            painter.setBrush(QColor("#ffffff"))
            painter.drawRoundedRect(row, 3, 3)
            painter.drawText(row.adjusted(6, 0, -4, 0), Qt.AlignVCenter | Qt.AlignLeft, text)

    def _draw_progress(self, painter: QPainter, area: QRectF) -> None:
        bar = QRectF(area.left() + 16, area.bottom() - 14, area.width() - 32, 5)
        painter.fillRect(bar, QColor("#eaecf0"))
        done = QRectF(bar.left(), bar.top(), bar.width() * self.progress, bar.height())
        painter.fillRect(done, QColor("#84caff"))


class CalibrationGui(QWidget):
    def __init__(self, bridge: RosBridge) -> None:
        super().__init__()
        self.bridge = bridge
        self.setWindowTitle("Cube-IMU Direct Data Recorder")

        self.values = {}
        self.runtime_process = None
        self.sensor_process = None
        self.direct_start_in_progress = False
        self.direct_start_requested_wall = 0.0
        self.direct_start_result = None
        self.direct_start_thread = None
        self.pending_auto_record = False
        self.current_motion_hint = "采集前：确认视频、串口 IMU 和 CameraInfo 稳定，然后开始录制。"
        self.motion_was_recording = False
        self.motion_phase = 0
        self.motion_phase_start_wall = time.monotonic()
        self.motion_last_imu_stamp = None
        self.motion_phase_abs_angle = np.zeros(3, dtype=float)
        self.motion_phase_signed_angle = np.zeros(3, dtype=float)
        self.motion_latest_gyro = np.zeros(3, dtype=float)
        self.motion_stable_start_wall = None
        self.motion_phase_positions = deque(maxlen=240)
        self.motion_phase_progress = 0.0
        self.scheduled_motion_phase = 0
        self.scheduled_motion_progress = 0.0
        self.current_action_time_text = ""
        self.recording_wall_start = 0.0
        self.phase_workflow_index = 0
        self.phase_workflow_mode = "idle"
        self.phase_detect_start_wall = 0.0
        self.phase_collect_start_wall = 0.0
        self.phase_collect_elapsed = 0.0
        self.phase_collected = [False] * len(ACTION_PHASES)
        self.current_phase_detection = ("warn", "等待姿态检测")
        self.stop_record_request_wall = 0.0
        self._preview_cache_key = None
        self._preview_cache_pixmap = None
        self._last_preview_paint_wall = 0.0
        self._content_splitter_sizes = []
        self._content_splitter_applying = False
        self._content_splitter_restore_pending = False
        self._apply_light_style()
        self._build_ui()
        self._fit_initial_window()

        self.timer = QTimer(self)
        self.timer.timeout.connect(self._on_timer)
        self.timer.start(20)

    def _build_ui(self) -> None:
        outer = QVBoxLayout(self)
        outer.setContentsMargins(14, 12, 14, 14)
        outer.setSpacing(10)

        header = QHBoxLayout()
        title = QLabel("Cube-IMU 数据采集面板")
        title.setObjectName("AppTitle")
        header.addWidget(title)
        header.addStretch(1)

        self.tip = HeaderStatusValue("正在连接直连硬件采集...")
        self.tip.set_state("正在连接直连硬件采集...", "warn")
        header.addWidget(self.tip, 0, Qt.AlignRight | Qt.AlignVCenter)
        outer.addLayout(header)

        content = QSplitter(Qt.Horizontal, self)
        content.setChildrenCollapsible(False)
        content.splitterMoved.connect(self._on_content_splitter_moved)
        self.content_splitter = content

        left_column = QWidget()
        left_column.setMinimumWidth(360)
        left_column.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Expanding)
        left_layout = QVBoxLayout(left_column)
        left_layout.setContentsMargins(0, 0, 0, 0)
        left_layout.setSpacing(10)
        left_layout.addWidget(self._imu_group(), 1)
        left_layout.addWidget(self._topic_group(), 0)

        center_column = QWidget()
        center_column.setMinimumWidth(460)
        center_column.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Expanding)
        center_layout = QVBoxLayout(center_column)
        center_layout.setContentsMargins(0, 0, 0, 0)
        center_layout.setSpacing(10)
        center_layout.addWidget(self._image_group(), 2)
        center_layout.addWidget(self._motion_group(), 1)

        right_scroll = QScrollArea()
        right_scroll.setMinimumWidth(600)
        right_scroll.setMaximumWidth(900)
        right_scroll.setSizePolicy(QSizePolicy.Preferred, QSizePolicy.Expanding)
        right_scroll.setWidgetResizable(True)
        right_scroll.setHorizontalScrollBarPolicy(Qt.ScrollBarAlwaysOff)
        right_scroll.setFrameShape(QFrame.NoFrame)
        right_column = QWidget()
        right_column.setSizePolicy(QSizePolicy.Ignored, QSizePolicy.Expanding)
        right_layout = QVBoxLayout(right_column)
        right_layout.setContentsMargins(0, 0, 0, 0)
        right_layout.setSpacing(6)
        right_layout.addWidget(self._settings_group(), 0)
        right_layout.addWidget(self._control_group(), 0)
        right_layout.addWidget(self._recorder_group(), 0)
        right_layout.addWidget(self._node_group(), 0)
        right_layout.addStretch(1)
        right_scroll.setWidget(right_column)

        content.addWidget(left_column)
        content.addWidget(center_column)
        content.addWidget(right_scroll)
        content.setStretchFactor(0, 30)
        content.setStretchFactor(1, 40)
        content.setStretchFactor(2, 30)
        outer.addWidget(content, 1)

    def _fit_initial_window(self) -> None:
        screen = QApplication.primaryScreen()
        if screen is None:
            self.resize(1800, 920)
        else:
            geometry = screen.availableGeometry()
            width = min(max(1700, int(geometry.width() * 0.96)), geometry.width() - 20)
            height = min(max(760, int(geometry.height() * 0.92)), geometry.height() - 40)
            self.resize(width, height)
            self.move(
                geometry.x() + max(0, (geometry.width() - width) // 2),
                geometry.y() + max(0, (geometry.height() - height) // 2),
            )
        QTimer.singleShot(0, self._set_initial_splitter_sizes)

    def _set_initial_splitter_sizes(self) -> None:
        width = max(1, self.content_splitter.width())
        self._apply_content_splitter_sizes([
            int(width * 0.30),
            int(width * 0.40),
            int(width * 0.30),
        ])

    def _on_content_splitter_moved(self, position: int, index: int) -> None:
        del position, index
        if not self._content_splitter_applying:
            self._remember_content_splitter_sizes()

    def _remember_content_splitter_sizes(self) -> None:
        sizes = self.content_splitter.sizes()
        if len(sizes) == 3 and sum(sizes) > 0:
            self._content_splitter_sizes = sizes

    def _apply_content_splitter_sizes(self, sizes: list[int], remember: bool = True) -> None:
        self._content_splitter_applying = True
        try:
            self.content_splitter.setSizes(sizes)
        finally:
            self._content_splitter_applying = False
        if remember:
            self._remember_content_splitter_sizes()

    def _splitter_sizes_for_total(self, target_total: int) -> list[int]:
        sizes = self._content_splitter_sizes or self.content_splitter.sizes()
        if len(sizes) != 3 or target_total <= 0:
            return sizes
        right = min(max(sizes[2], 600), 900)
        remaining = max(2, target_total - right)
        left_center_total = max(1, sizes[0] + sizes[1])
        left = max(1, int(round(remaining * sizes[0] / left_center_total)))
        center = max(1, remaining - left)
        return [left, center, right]

    def _resize_content_splitter_for_window(self) -> None:
        if self._content_splitter_applying:
            return
        target_total = sum(self.content_splitter.sizes())
        if target_total <= 0:
            return
        self._apply_content_splitter_sizes(
            self._splitter_sizes_for_total(target_total),
            remember=True,
        )

    def _restore_content_splitter_after_refresh(self) -> None:
        self._content_splitter_restore_pending = False
        if self._content_splitter_applying or not self._content_splitter_sizes:
            return
        current = self.content_splitter.sizes()
        if len(current) != 3:
            return
        if abs(sum(current) - sum(self._content_splitter_sizes)) > 2:
            return
        if any(abs(left - right) > 1 for left, right in zip(current, self._content_splitter_sizes)):
            self._apply_content_splitter_sizes(self._content_splitter_sizes, remember=False)

    def _schedule_content_splitter_restore(self) -> None:
        if self._content_splitter_restore_pending:
            return
        self._content_splitter_restore_pending = True
        QTimer.singleShot(0, self._restore_content_splitter_after_refresh)

    def resizeEvent(self, event) -> None:  # noqa: N802 - Qt override name.
        super().resizeEvent(event)
        if hasattr(self, "content_splitter"):
            QTimer.singleShot(0, self._resize_content_splitter_for_window)

    def _apply_light_style(self) -> None:
        self.setStyleSheet(
            """
            QWidget {
                background: #f2f4f7;
                color: #101828;
                font-family: "Noto Sans CJK SC", "Microsoft YaHei", "Sans Serif";
                font-size: 13px;
            }
            #AppTitle {
                font-size: 21px;
                font-weight: 700;
                color: #101828;
            }
            QGroupBox {
                background: #ffffff;
                border: 1px solid #d0d5dd;
                border-radius: 4px;
                margin-top: 15px;
                padding: 8px;
                font-weight: 700;
                color: #101828;
            }
            QGroupBox::title {
                subcontrol-origin: margin;
                left: 10px;
                padding: 0 6px;
                background: #ffffff;
                color: #175cd3;
            }
            QLabel {
                color: #101828;
            }
            QLineEdit, QDoubleSpinBox, QSpinBox {
                background: #ffffff;
                color: #344054;
                border: 1px solid #98a2b3;
                border-radius: 3px;
                padding: 3px 6px;
                min-height: 22px;
            }
            QLineEdit:disabled, QDoubleSpinBox:disabled, QSpinBox:disabled {
                color: #98a2b3;
                background: #f2f4f7;
            }
            QPushButton {
                background: #ffffff;
                color: #101828;
                border: 1px solid #98a2b3;
                border-radius: 4px;
                padding: 5px 9px;
                min-height: 26px;
            }
            QPushButton:hover {
                background: #f9fafb;
                border-color: #667085;
            }
            QPushButton:disabled {
                color: #98a2b3;
                background: #f2f4f7;
                border-color: #d0d5dd;
            }
            QPushButton#PrimaryAction {
                background: #175cd3;
                color: #ffffff;
                border-color: #175cd3;
                font-weight: 700;
            }
            QPushButton#PrimaryAction:hover {
                background: #1849a9;
                border-color: #1849a9;
            }
            QPushButton#DangerAction {
                background: #fef3f2;
                color: #b42318;
                border-color: #fecdca;
                font-weight: 700;
            }
            QPushButton#DangerAction:hover {
                background: #fee4e2;
                border-color: #fda29b;
            }
            QProgressBar {
                background: #ffffff;
                color: #101828;
                border: 1px solid #98a2b3;
                border-radius: 3px;
                min-height: 20px;
                text-align: center;
            }
            QProgressBar::chunk {
                background: #84caff;
                border-radius: 2px;
            }
            QSplitter::handle {
                background: #e4e7ec;
            }
            QScrollArea {
                border: none;
                background: #f2f4f7;
            }
            """
        )

    def _row(self, layout: QGridLayout, row: int, name: str, key: str) -> None:
        label = QLabel(name)
        label.setMinimumWidth(88)
        value = StatusValue()
        value.setSizePolicy(QSizePolicy.Ignored, QSizePolicy.Preferred)
        layout.addWidget(label, row, 0)
        layout.addWidget(value, row, 1)
        self.values[key] = value

    @staticmethod
    def _fit_right_field(widget: QWidget) -> QWidget:
        widget.setMinimumWidth(0)
        widget.setSizePolicy(QSizePolicy.Ignored, QSizePolicy.Fixed)
        return widget

    @staticmethod
    def _fit_right_button(button: QPushButton) -> QPushButton:
        button.setMinimumWidth(0)
        button.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)
        return button

    @staticmethod
    def _compact_text(text: str, limit: int = 96) -> str:
        normalized = " ".join(str(text).split())
        if len(normalized) <= limit:
            return normalized
        head = max(16, limit // 2 - 4)
        tail = max(16, limit - head - 5)
        return f"{normalized[:head]} ... {normalized[-tail:]}"

    @staticmethod
    def _compact_path(path: str, limit: int = 78) -> str:
        normalized = os.path.normpath(str(path))
        if len(normalized) <= limit:
            return normalized
        parts = [part for part in normalized.split(os.sep) if part]
        for keep in range(min(4, len(parts)), 0, -1):
            candidate = f".../{'/'.join(parts[-keep:])}"
            if len(candidate) <= limit:
                return candidate
        return CalibrationGui._compact_text(normalized, limit)

    def _settings_group(self) -> QGroupBox:
        group = QGroupBox("采集设置")
        layout = QGridLayout(group)
        layout.setHorizontalSpacing(8)
        layout.setVerticalSpacing(5)

        self.image_topic_edit = QLineEdit(f"Orbbec SDK 直连 1280x720 mono8 >= {MIN_RECORD_IMAGE_RATE_HZ:.0f}Hz")
        self.imu_topic_edit = QLineEdit(f"Cube 串口 IMU 直连 >= {MIN_RECORD_IMU_RATE_HZ:.0f}Hz")
        self.camera_info_topic_edit = QLineEdit("Orbbec SDK CameraInfo 直读")
        self.bag_path_edit = QLineEdit(self.bridge.bag_path)
        self.camera_frame_edit = QLineEdit(self.bridge.camera_frame)
        self.imu_frame_edit = QLineEdit(self.bridge.imu_frame)
        self.cube_layout_edit = QLineEdit(f"{CUBE_LAYOUT_LABEL} ({CUBE_LAYOUT_NAME})")
        self.imu_source_edit = QLineEdit("cube_serial (Cube 串口 IMU)")
        self.serial_port_edit = QLineEdit(self.bridge.serial_port)
        for widget in (
            self.image_topic_edit,
            self.imu_topic_edit,
            self.camera_info_topic_edit,
            self.bag_path_edit,
            self.camera_frame_edit,
            self.imu_frame_edit,
            self.cube_layout_edit,
            self.imu_source_edit,
            self.serial_port_edit,
        ):
            self._fit_right_field(widget)
        for widget in (self.image_topic_edit, self.imu_topic_edit, self.camera_info_topic_edit):
            widget.setReadOnly(True)
        self.cube_layout_edit.setReadOnly(True)
        self.cube_layout_edit.setToolTip("模板标题应为 AprilTag 36h11 Cube - Left Hand Cube；ID 布局为 0=center, 1=top, 2=right, 3=bottom, 4=left。")
        self.imu_source_edit.setReadOnly(True)
        self.imu_source_edit.setToolTip("本采集流程只录制 Cube 上的串口 IMU；Orbbec 相机内置 IMU 不用于这组数据。")

        self.duration_spin = QDoubleSpinBox()
        self.duration_spin.setRange(0.0, 3600.0)
        self.duration_spin.setDecimals(1)
        self.duration_spin.setSingleStep(10.0)
        self.duration_spin.setValue(self.bridge.record_duration_sec)
        self.duration_spin.setSuffix(" s")

        rows = [
            ("图像", self.image_topic_edit),
            ("IMU", self.imu_topic_edit),
            ("内参", self.camera_info_topic_edit),
            ("前缀", self.bag_path_edit),
            ("时长", self.duration_spin),
            ("相机帧", self.camera_frame_edit),
            ("IMU帧", self.imu_frame_edit),
            ("模板", self.cube_layout_edit),
            ("来源", self.imu_source_edit),
            ("串口", self.serial_port_edit),
        ]
        for index, (name, widget) in enumerate(rows):
            row = index // 2
            column = (index % 2) * 2
            label = QLabel(name)
            label.setMinimumWidth(48)
            layout.addWidget(label, row, column)
            layout.addWidget(widget, row, column + 1)
        layout.setColumnStretch(1, 1)
        layout.setColumnStretch(3, 1)
        return group

    def _image_group(self) -> QGroupBox:
        group = QGroupBox("视觉模块 / 实拍相机画面")
        layout = QVBoxLayout(group)

        self.image_preview = StablePreviewLabel("等待图像")
        self.image_preview.setAlignment(Qt.AlignCenter)
        self.image_preview.setMinimumSize(360, 270)
        self.image_preview.setSizePolicy(QSizePolicy.Ignored, QSizePolicy.Ignored)
        self.image_preview.setStyleSheet(
            "QLabel { background: #f8fafc; color: #667085; "
            "border: 1px solid #d0d5dd; border-radius: 4px; }"
        )
        self.image_info = StatusValue("等待图像")
        self.image_info.setWordWrap(False)
        self.image_info.setFixedHeight(32)
        self.image_info.setSizePolicy(QSizePolicy.Ignored, QSizePolicy.Fixed)

        layout.addWidget(self.image_preview, 1)
        layout.addWidget(self.image_info, 0)
        return group

    def _motion_group(self) -> QGroupBox:
        group = QGroupBox("动作向导")
        group.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Expanding)
        layout = QVBoxLayout(group)
        layout.setContentsMargins(8, 10, 8, 8)
        layout.setSpacing(6)

        self.motion_hint = StatusValue(self.current_motion_hint)
        self.motion_hint.setWordWrap(True)
        self.motion_hint.setMinimumHeight(38)
        self.motion_hint.setMaximumHeight(48)
        self.motion_guide = MotionGuideWidget()

        layout.addWidget(self.motion_hint, 0)
        layout.addWidget(self.motion_guide, 1)
        return group

    def _imu_group(self) -> QGroupBox:
        group = QGroupBox("IMU 模块 / 六轴波形")
        layout = QVBoxLayout(group)
        telemetry = QGridLayout()
        telemetry.setHorizontalSpacing(8)
        telemetry.setVerticalSpacing(6)
        self.imu_accel_value = StatusValue("等待加速度")
        self.imu_gyro_value = StatusValue("等待角速度")
        self.imu_norm_value = StatusValue("等待模长")
        telemetry.addWidget(QLabel("加速度 [g]"), 0, 0)
        telemetry.addWidget(self.imu_accel_value, 0, 1)
        telemetry.addWidget(QLabel("角速度 [deg/s]"), 1, 0)
        telemetry.addWidget(self.imu_gyro_value, 1, 1)
        telemetry.addWidget(QLabel("模长"), 2, 0)
        telemetry.addWidget(self.imu_norm_value, 2, 1)
        self.imu_plot = ImuPlotWidget()
        self.imu_info = StatusValue("等待 IMU")
        self.imu_info.setWordWrap(False)
        self.imu_info.setFixedHeight(32)
        self.imu_info.setSizePolicy(QSizePolicy.Ignored, QSizePolicy.Fixed)
        layout.addLayout(telemetry)
        layout.addWidget(self.imu_plot, 1)
        layout.addWidget(self.imu_info, 0)
        return group

    def _control_group(self) -> QGroupBox:
        group = QGroupBox("一键控制")
        layout = QGridLayout(group)
        layout.setHorizontalSpacing(8)
        layout.setVerticalSpacing(6)

        self.runtime_state = StatusValue("直连采集未启动")
        self.runtime_state.setWordWrap(True)
        self.runtime_state.setSizePolicy(QSizePolicy.Ignored, QSizePolicy.Preferred)
        layout.addWidget(QLabel("采集状态"), 0, 0)
        layout.addWidget(self.runtime_state, 0, 1)

        runtime_buttons = QGridLayout()
        runtime_buttons.setHorizontalSpacing(8)
        self.prepare_button = QPushButton("启动直连采集")
        self.one_click_button = QPushButton("一键开始采集")
        self.stop_runtime_button = QPushButton("停止直连采集")
        self.one_click_button.setObjectName("PrimaryAction")
        self.stop_runtime_button.setObjectName("DangerAction")
        self.prepare_button.clicked.connect(self._start_runtime_nodes)
        self.one_click_button.clicked.connect(self._one_click_record)
        self.stop_runtime_button.clicked.connect(self._stop_runtime_nodes)
        for column, button in enumerate((
            self.prepare_button,
            self.one_click_button,
            self.stop_runtime_button,
        )):
            runtime_buttons.addWidget(self._fit_right_button(button), 0, column)
            runtime_buttons.setColumnStretch(column, 1)
        layout.addLayout(runtime_buttons, 1, 0, 1, 2)

        layout.setColumnStretch(1, 1)
        return group

    def _node_group(self) -> QGroupBox:
        group = QGroupBox("直连采集链路")
        layout = QGridLayout(group)
        self._row(layout, 0, "Orbbec 相机", "real_sensor_node")
        self._row(layout, 1, "直连记录器", "data_recorder_node")
        self._row(layout, 2, "可视化/检测", "calibration_gui_node")
        self._row(layout, 3, "实时数据流", "live_stream")
        self._row(layout, 4, "IMU 启动诊断", "imu_diagnostic")
        self._row(layout, 5, "时间戳诊断", "timestamp_diag")
        for key in (
            "real_sensor_node",
            "data_recorder_node",
            "calibration_gui_node",
            "live_stream",
            "imu_diagnostic",
            "timestamp_diag",
        ):
            self.values[key].setWordWrap(True)
        return group

    def _apply_settings_to_bridge(self) -> None:
        self.bridge.bag_path = self.bag_path_edit.text().strip() or "output_bag"
        self.bridge.record_duration_sec = float(self.duration_spin.value())
        self.bridge.camera_frame = self.camera_frame_edit.text().strip() or "camera_link"
        self.bridge.imu_frame = self.imu_frame_edit.text().strip() or "imu_link"
        self.bridge.imu_source = "cube_serial"
        self.bridge.serial_port = self.serial_port_edit.text().strip() or "/dev/ttyACM0"
        self.bridge.last_pose = None
        self.bridge.last_pose_wall = 0.0
        self.bridge.configure_data_subscriptions()

    def _runtime_running(self) -> bool:
        return self.bridge.direct_sensor.is_started()

    def _sensor_running(self) -> bool:
        return self.bridge.direct_sensor.is_started()

    def _recorder_status_fresh(self) -> bool:
        return (
            self.bridge.last_status is not None
            and time.monotonic() - self.bridge.last_status_wall < 3.0
        )

    def _recorder_online(self) -> bool:
        return True

    def _preview_source_label(self) -> str:
        return "直连硬件"

    def _bag_path_for_display(self, status) -> tuple[str, str, str]:
        raw_path = ""
        recording = False
        if status is not None and getattr(status, "bag_path", ""):
            raw_path = status.bag_path
            recording = bool(getattr(status, "recording", False))
        if not raw_path:
            raw_path = self.bridge.bag_path

        expanded = os.path.expanduser(str(raw_path))
        absolute = os.path.abspath(expanded)
        compact = self._compact_path(absolute)
        if recording:
            return compact, "ok", absolute
        return (
            f"{compact}；开始录制时自动建目录",
            "neutral",
            f"{absolute}  （开始录制时创建；若目录已存在会自动追加时间戳）",
        )

    def _action_schedule(self, target_duration_sec: float) -> list[tuple[int, float, float, str, str]]:
        total_base = sum(phase[2] for phase in ACTION_PHASES)
        total = float(target_duration_sec) if target_duration_sec > 0.0 else total_base
        total = max(total, 1.0)
        scale = total / total_base
        schedule = []
        start = 0.0
        for index, (title, description, base_duration) in enumerate(ACTION_PHASES):
            end = total if index == len(ACTION_PHASES) - 1 else start + base_duration * scale
            schedule.append((index, start, end, title, description))
            start = end
        return schedule

    def _action_schedule_text(self, target_duration_sec: float) -> str:
        parts = []
        for index, start, end, title, _ in self._action_schedule(target_duration_sec):
            parts.append(f"{index + 1}. {fmt_seconds(start)}-{fmt_seconds(end)} {title}")
        return "  |  ".join(parts)

    @staticmethod
    def _phase_precheck_duration(phase_duration: float) -> float:
        return min(PHASE_PRECHECK_MAX_SEC, max(1.0, phase_duration * PHASE_PRECHECK_RATIO))

    @staticmethod
    def _phase_bar_stylesheet(state: str) -> str:
        colors = {
            "ok": ("#ecfdf3", "#027a48", "#abefc6", "#73c0f8"),
            "warn": ("#fffaeb", "#b54708", "#fedf89", "#fdb022"),
            "neutral": ("#ffffff", "#101828", "#98a2b3", "#d0d5dd"),
        }
        bg, fg, border, chunk = colors.get(state, colors["neutral"])
        return (
            "QProgressBar {"
            f"background: {bg}; color: {fg}; border: 1px solid {border};"
            "border-radius: 3px; text-align: center; min-height: 22px;"
            "}"
            "QProgressBar::chunk {"
            f"background: {chunk}; border-radius: 2px;"
            "}"
        )

    def _set_phase_bar_state(self, bar: QProgressBar, state: str) -> None:
        bar.setStyleSheet(self._phase_bar_stylesheet(state))

    def _phase_samples_by_elapsed(self, start: float, end: float) -> list[tuple]:
        if self.recording_wall_start <= 0.0:
            return []
        samples = []
        for sample in self.bridge.imu_samples:
            if len(sample) < 8:
                continue
            elapsed = float(sample[7]) - self.recording_wall_start
            if start <= elapsed <= end:
                samples.append(sample)
        return samples

    def _phase_samples_by_wall(self, start_wall: float, end_wall: float) -> list[tuple]:
        if start_wall <= 0.0:
            return []
        samples = []
        for sample in self.bridge.imu_samples:
            if len(sample) < 8:
                continue
            wall = float(sample[7])
            if start_wall <= wall <= end_wall:
                samples.append(sample)
        return samples

    @staticmethod
    def _motion_arrays(samples: list[tuple]) -> tuple[np.ndarray, np.ndarray]:
        if not samples:
            return np.empty((0, 3), dtype=float), np.empty((0, 3), dtype=float)
        accel = np.array([sample[1:4] for sample in samples], dtype=float) / GRAVITY_M_S2
        gyro_deg = np.array([sample[4:7] for sample in samples], dtype=float) * RAD_TO_DEG
        return accel, gyro_deg

    def _phase_precheck_detection(
        self,
        index: int,
        start_wall: float,
        precheck_sec: float,
    ) -> tuple[str, str]:
        now = time.monotonic()
        window_sec = min(precheck_sec, 1.0)
        window_start = max(start_wall, now - window_sec)
        observed_sec = max(0.0, now - window_start)
        samples = self._phase_samples_by_wall(window_start, now)
        if observed_sec < POSE_CHECK_MIN_EVAL_SEC or len(samples) < 3:
            return "warn", "姿态检测中"

        accel_g, gyro_deg = self._motion_arrays(samples)
        gyro_norm = np.linalg.norm(gyro_deg, axis=1)
        accel_mean = np.mean(accel_g, axis=0)
        gravity_norm = float(np.linalg.norm(accel_mean))
        gyro_p80 = float(np.percentile(gyro_norm, 80.0))

        if not (POSE_GRAVITY_MIN_G <= gravity_norm <= POSE_GRAVITY_MAX_G):
            return "warn", f"姿态不稳：|a|={gravity_norm:.2f}g"
        if gyro_p80 > POSE_STILL_GYRO_WARN_DEG_S:
            return "warn", f"请先保持姿态静止 |w|={gyro_p80:.0f}"

        if self.bridge.enable_cube_detection:
            pose_age = (
                now - self.bridge.last_pose_wall
                if self.bridge.last_pose_wall > 0.0
                else float("inf")
            )
            if pose_age > 1.0:
                return "warn", "等待 Cube 视觉位姿"
            cube_pose_rate = self.bridge.cube_pose_rate_hz()
            if cube_pose_rate < MIN_RECORD_CUBE_POSE_RATE_HZ:
                return "warn", f"等待 Cube pose 频率≥{MIN_RECORD_CUBE_POSE_RATE_HZ:.0f}Hz 当前 {cube_pose_rate:.1f}Hz"
            reproj_error = float(self.bridge.last_reproj_error)
            reproj_hint = ""
            if np.isfinite(reproj_error):
                if reproj_error > POSE_REPROJ_BLOCK_PX:
                    return "warn", f"重投影过大 {reproj_error:.1f}px，请让 Tag 更清晰/更正对"
                if reproj_error > POSE_REPROJ_SOFT_WARN_PX:
                    reproj_hint = f"；重投影 {reproj_error:.1f}px，可采集但建议靠近或放慢"
            if index == 4 and self.bridge.last_detection_tag_count < 2:
                return "warn", "请移到相邻面边界，让双 Tag 同时可见"

        if index == 0:
            return "ok", "姿态通过：居中静止" + reproj_hint
        elif index in (1, 2, 3):
            axis_name = ("X", "Y", "Z")[index - 1]
            return "ok", f"准备通过：Cube 可见，开始绕 {axis_name} 轴慢速往返" + reproj_hint

        return "ok", "姿态通过：可以开始本段动作" + reproj_hint

    def _phase_duration(self, schedule, index: int) -> float:
        _, start, end, _, _ = schedule[index]
        return max(end - start, 1e-6)

    def _collected_action_seconds(self, schedule) -> float:
        collected_sec = sum(
            self._phase_duration(schedule, phase_index)
            for phase_index, collected in enumerate(self.phase_collected)
            if collected and phase_index < len(schedule)
        )
        if self.phase_workflow_mode == "collect" and self.phase_workflow_index < len(schedule):
            collected_sec += min(
                self.phase_collect_elapsed,
                self._phase_duration(schedule, self.phase_workflow_index),
            )
        return collected_sec

    def _update_formal_collection_progress(self, status) -> None:
        schedule = self._action_schedule(self.bridge.record_duration_sec)
        total = max(sum(self._phase_duration(schedule, index) for index in range(len(schedule))), 1e-6)
        collected_sec = max(0.0, min(total, self._collected_action_seconds(schedule)))
        ratio = collected_sec / total

        self.progress_bar.setRange(0, 1000)
        self.progress_bar.setValue(int(ratio * 1000))
        recording = bool(status is not None and status.recording)
        if self.phase_workflow_mode == "complete":
            self.progress_bar.setFormat(f"正式采集完成 {collected_sec:.1f}s / {total:.1f}s")
        elif self.phase_workflow_mode == "collect" and self.phase_workflow_index < len(schedule):
            phase_total = self._phase_duration(schedule, self.phase_workflow_index)
            phase_ratio = min(1.0, self.phase_collect_elapsed / phase_total)
            self.progress_bar.setFormat(
                f"第{self.phase_workflow_index + 1}段 {phase_ratio * 100.0:.0f}% | 总 {ratio * 100.0:.1f}%"
            )
        elif recording:
            self.progress_bar.setFormat(f"姿态检测/正式采集中 | 已采 {collected_sec:.1f}s / {total:.1f}s")
        else:
            self.progress_bar.setFormat("等待正式采集")

    def _reset_phase_workflow(self, mode: str = "detect") -> None:
        now = time.monotonic()
        self.phase_workflow_index = 0
        self.phase_workflow_mode = mode
        self.phase_detect_start_wall = now if mode == "detect" else 0.0
        self.phase_collect_start_wall = now if mode == "collect" else 0.0
        self.phase_collect_elapsed = 0.0
        self.phase_collected = [False] * len(ACTION_PHASES)
        self.current_phase_detection = ("warn", "等待姿态检测")

    def _finish_current_phase_if_needed(self, schedule) -> None:
        if self.phase_workflow_mode != "collect":
            return
        index = self.phase_workflow_index
        if index >= len(schedule):
            return
        duration = self._phase_duration(schedule, index)
        self.phase_collect_elapsed = max(0.0, time.monotonic() - self.phase_collect_start_wall)
        if self.phase_collect_elapsed < duration:
            return

        self.phase_collected[index] = True
        if index + 1 >= len(schedule):
            self.phase_workflow_index = len(schedule)
            self.phase_workflow_mode = "complete"
            self.bridge.last_service_message = "六段正式采集已完成：可以停止录制并保存直连数据集"
            return

        self.phase_workflow_index = index + 1
        self.phase_workflow_mode = "detect"
        self.phase_detect_start_wall = time.monotonic()
        self.phase_collect_start_wall = 0.0
        self.phase_collect_elapsed = 0.0
        self.current_phase_detection = ("warn", "等待姿态检测")
        next_title = schedule[self.phase_workflow_index][3]
        next_duration = self._phase_duration(schedule, self.phase_workflow_index)
        self.bridge.last_service_message = (
            f"第 {index + 1} 段采集完成：进入第 {index + 2} 段“{next_title}”姿态检测，"
            f"检测通过后自动采集 {fmt_seconds(next_duration)}"
        )

    def _set_phase_collect_button(self, enabled: bool, text: str) -> None:
        if hasattr(self, "phase_collect_button"):
            self.phase_collect_button.setEnabled(enabled)
            self.phase_collect_button.setText(text)

    def _update_phase_progress(self, schedule, elapsed: float, recording: bool) -> None:
        if not hasattr(self, "phase_progress_bars"):
            return
        if not recording:
            self._set_phase_collect_button(False, "录制开始后先检测")
            for index, start, end, title, _ in schedule:
                del title
                bar = self.phase_progress_bars[index]
                status = self.phase_status_values[index]
                bar.setValue(0)
                bar.setFormat(f"{fmt_seconds(start)}-{fmt_seconds(end)}")
                self._set_phase_bar_state(bar, "neutral")
                state = "warn" if index == 0 else "neutral"
                text = "等待录制" if index == 0 else "待采集"
                status.set_state(text, state)
            return

        del elapsed
        if self.phase_workflow_mode in ("idle", "complete") and self.phase_workflow_index < len(schedule):
            self._reset_phase_workflow("detect")

        self._finish_current_phase_if_needed(schedule)

        button_enabled = False
        button_text = "姿态检测中"
        current = self.phase_workflow_index
        if self.phase_workflow_mode == "complete":
            button_text = "六段已完成"
        elif current < len(schedule):
            button_text = (
                f"第 {current + 1} 段采集中"
                if self.phase_workflow_mode == "collect"
                else f"第 {current + 1} 段检测中"
            )

        for index, start, end, title, _ in schedule:
            duration = self._phase_duration(schedule, index)
            detect_sec = min(self._phase_precheck_duration(duration), duration * 0.8)
            bar = self.phase_progress_bars[index]
            status = self.phase_status_values[index]

            if index < current or self.phase_collected[index]:
                bar.setValue(1000)
                bar.setFormat("已采集")
                self._set_phase_bar_state(bar, "ok")
                status.set_state("已采集", "ok")
                continue

            if index > current or self.phase_workflow_mode == "complete":
                bar.setValue(0)
                bar.setFormat(f"待采集 {fmt_seconds(start)}")
                self._set_phase_bar_state(bar, "neutral")
                status.set_state("等待", "neutral")
                continue

            if self.phase_workflow_mode == "detect":
                state, text = self._phase_precheck_detection(index, self.phase_detect_start_wall, detect_sec)
                self.current_phase_detection = (state, text)
                if state == "ok":
                    self.phase_workflow_mode = "collect"
                    self.phase_collect_start_wall = time.monotonic()
                    self.phase_collect_elapsed = 0.0
                    self.phase_detect_start_wall = 0.0
                    button_text = f"第 {index + 1} 段采集中"
                    duration_text = fmt_seconds(duration)
                    self.bridge.last_service_message = (
                        f"第 {index + 1} 段姿态检测通过：开始“{title}”正式采集，"
                        f"本段时长 {duration_text}"
                    )
                else:
                    bar.setValue(0)
                    bar.setFormat("姿态检测中")
                    self._set_phase_bar_state(bar, state)
                    status.set_state(text, state)
                    continue

            if self.phase_workflow_mode == "collect":
                self.phase_collect_elapsed = max(0.0, time.monotonic() - self.phase_collect_start_wall)
                collect_progress = min(1.0, self.phase_collect_elapsed / duration)
                remaining = max(0.0, duration - self.phase_collect_elapsed)
                bar.setValue(int(collect_progress * 1000))
                bar.setFormat(f"正式采集 {collect_progress * 100.0:.0f}%")
                self._set_phase_bar_state(bar, "ok")
                status.set_state(f"采集中 剩 {fmt_seconds(remaining)}", "ok")

        self._set_phase_collect_button(button_enabled, button_text)

    def _update_action_timing(self, status) -> None:
        target_duration = self.bridge.record_duration_sec
        recording = bool(status is not None and status.recording)
        if status is not None and status.target_duration_sec > 0.0:
            target_duration = float(status.target_duration_sec)
        if target_duration <= 0.0:
            target_duration = sum(phase[2] for phase in ACTION_PHASES)

        schedule = self._action_schedule(target_duration)

        if status is None:
            self.scheduled_motion_phase = 0
            self.scheduled_motion_progress = 0.0
            self.recording_wall_start = 0.0
            self.current_action_time_text = f"等待录制；动作计划总时长 {fmt_seconds(target_duration)}"
            self.values["action_time"].set_state(self.current_action_time_text, "warn")
            self._update_phase_progress(schedule, 0.0, False)
            return

        elapsed = max(0.0, float(status.elapsed_sec))
        if recording and self.bridge.last_status_wall > 0.0:
            elapsed += max(0.0, time.monotonic() - self.bridge.last_status_wall)
        if not recording:
            self.scheduled_motion_phase = 0
            self.scheduled_motion_progress = 0.0
            self.recording_wall_start = 0.0
            self.current_action_time_text = (
                f"未录制；开始后按 {fmt_seconds(target_duration)} 六段流程动作，"
                "每段先做姿态检测，检测通过后自动进入正式采集"
            )
            self.values["action_time"].set_state(
                f"未录制；{fmt_seconds(target_duration)} 六段流程；每段先检测再采集",
                "neutral",
                self.current_action_time_text,
            )
            self._update_phase_progress(schedule, elapsed, False)
            return
        self.recording_wall_start = time.monotonic() - elapsed

        self._update_phase_progress(schedule, elapsed, True)
        if self.phase_workflow_mode == "complete":
            self.scheduled_motion_phase = len(ACTION_PHASES)
            self.scheduled_motion_progress = 1.0
            self.current_action_time_text = (
                f"六段正式采集已完成；数据集已录 {fmt_seconds(elapsed)}，可以点击停止录制保存数据"
            )
            self.values["action_time"].set_state(
                f"六段完成；已录 {fmt_seconds(elapsed)}；可停止保存",
                "ok",
                self.current_action_time_text,
            )
            return

        index = min(self.phase_workflow_index, len(schedule) - 1)
        _, start, end, title, description = schedule[index]
        phase_duration = self._phase_duration(schedule, index)
        collected_sec = sum(
            self._phase_duration(schedule, phase_index)
            for phase_index, collected in enumerate(self.phase_collected)
            if collected and phase_index < len(schedule)
        )
        if self.phase_workflow_mode == "collect":
            phase_mode = "正式采集"
            phase_elapsed = min(self.phase_collect_elapsed, phase_duration)
            phase_remaining = max(0.0, phase_duration - phase_elapsed)
            collected_sec += phase_elapsed
            self.scheduled_motion_progress = min(1.0, phase_elapsed / phase_duration)
            state = "ok"
            detail = (
                f"本段已采 {fmt_seconds(phase_elapsed)}/{fmt_seconds(phase_duration)}，"
                f"本段剩余 {fmt_seconds(phase_remaining)}"
            )
            action = f"正在采集：{description}；动作要慢、连续，保持视频清晰"
        else:
            detect_state, detect_text = self.current_phase_detection
            phase_mode = "姿态检测"
            phase_elapsed = max(0.0, time.monotonic() - self.phase_detect_start_wall)
            self.scheduled_motion_progress = 0.0
            state = "ok" if detect_state == "ok" else "warn"
            detail = f"检测耗时 {fmt_seconds(phase_elapsed)}；{detect_text}"
            action = f"先调整姿态：{description}；检测通过后自动进入正式采集"

        self.scheduled_motion_phase = index
        total_remaining = max(0.0, target_duration - collected_sec)
        self.current_action_time_text = (
            f"现在动作：阶段 {index + 1}/6 - {title}（{fmt_seconds(start)}-{fmt_seconds(end)}）\n"
            f"数据集已录 {fmt_seconds(elapsed)}；正式采集累计 {fmt_seconds(collected_sec)}/{fmt_seconds(target_duration)}，"
            f"剩余正式采集 {fmt_seconds(total_remaining)}\n"
            f"阶段状态：{phase_mode}；{detail}\n"
            f"操作：{action}"
        )
        action_summary = (
            f"阶段 {index + 1}/6：{title}；{phase_mode}\n"
            f"已录 {fmt_seconds(elapsed)}；累计 {fmt_seconds(collected_sec)}/{fmt_seconds(target_duration)}；"
            f"剩余 {fmt_seconds(total_remaining)}"
        )
        self.values["action_time"].set_state(action_summary, state, self.current_action_time_text)

    def _sensor_publishers_present(self) -> bool:
        stats = self.bridge.direct_sensor.stats()
        return bool(stats.image_rate_hz > 0.1 or stats.imu_rate_hz > 0.1 or self.bridge.camera_info_ready)

    def _real_sensor_arguments(self) -> list[str]:
        return [
            "run",
            "cube_imu_calibration",
            "real_sensor_node",
            "--ros-args",
            "-p",
            f"image_topic:={self.bridge.image_topic}",
            "-p",
            f"imu_topic:={self.bridge.imu_topic}",
            "-p",
            f"camera_info_topic:={self.bridge.camera_info_topic}",
            "-p",
            f"camera_frame:={self.bridge.camera_frame}",
            "-p",
            f"imu_frame:={self.bridge.imu_frame}",
            "-p",
            f"imu_source:={self.bridge.imu_source}",
            "-p",
            f"serial_port:={self.bridge.serial_port}",
            "-p",
            f"serial_baudrate:={self.bridge.serial_baudrate}",
            "-p",
            f"serial_protocol_mode:={self.bridge.serial_protocol_mode}",
            "-p",
            f"serial_imu_rate_hz:={self.bridge.serial_imu_rate_hz}",
            "-p",
            f"serial_startup_command:={self.bridge.serial_startup_command}",
            "-p",
            f"output_encoding:={self.bridge.camera_output_encoding}",
            "-p",
            f"color_fps:={self.bridge.color_fps}",
            "-p",
            f"image_qos_depth:={self.bridge.real_sensor_image_qos_depth}",
            "-p",
            f"imu_qos_depth:={self.bridge.real_sensor_imu_qos_depth}",
        ]

    def _start_real_sensor_node(self) -> None:
        self._launch_direct_start_thread()

    def _stop_real_sensor_node(self) -> None:
        self.bridge.stop_direct_sensor()

    def _kill_sensor_if_needed(self) -> None:
        return

    def _sensor_finished(self, exit_code: int, exit_status: QProcess.ExitStatus) -> None:
        del exit_status
        self.bridge.last_service_message = f"真实传感器节点已退出，退出码={exit_code}"

    def _sensor_error(self, error: QProcess.ProcessError) -> None:
        self.bridge.last_service_message = f"真实传感器节点启动异常：{error}"

    def _start_runtime_nodes(self) -> None:
        self._apply_settings_to_bridge()
        if self.bridge.direct_sensor.is_started():
            self.bridge.last_service_message = "直连采集已经在运行"
        else:
            self._launch_direct_start_thread()

    def _launch_direct_start_thread(self) -> None:
        if self.direct_start_in_progress:
            self.bridge.last_service_message = "直连采集正在启动中，请等待 Orbbec SDK 返回"
            return
        if not self.bridge.direct_sensor.available():
            self.bridge.last_service_message = "直连采集库不可用：" + self.bridge.direct_sensor.error
            self.runtime_state.set_state(self.bridge.last_service_message, "bad")
            return

        self.direct_start_in_progress = True
        self.direct_start_requested_wall = time.monotonic()
        self.direct_start_result = None
        self.bridge.last_service_message = "直连采集启动中：正在打开 Orbbec 相机和 Cube 串口 IMU..."
        self.runtime_state.set_state(self.bridge.last_service_message, "warn")
        self.prepare_button.setEnabled(False)
        self.one_click_button.setEnabled(False)
        QApplication.processEvents()

        def _worker() -> None:
            try:
                ok = self.bridge.start_direct_sensor()
                status = self.bridge.direct_sensor_status or self.bridge.direct_sensor.status_text()
                self.direct_start_result = (ok, status)
            except Exception as exc:  # noqa: BLE001 - surfaced on the next UI tick.
                self.direct_start_result = (False, f"{exc}")

        self.direct_start_thread = threading.Thread(target=_worker, daemon=True)
        self.direct_start_thread.start()

    def _consume_direct_start_result(self) -> None:
        if not self.direct_start_in_progress:
            return
        if self.direct_start_result is None:
            elapsed = time.monotonic() - self.direct_start_requested_wall
            self.runtime_state.set_state(
                f"直连采集启动中 {elapsed:.1f}s：正在等待 Orbbec SDK/串口返回",
                "warn",
            )
            return
        ok, status = self.direct_start_result
        self.direct_start_in_progress = False
        self.direct_start_result = None
        if ok:
            self.bridge.last_service_message = "直连采集已启动：Orbbec 图像 + Cube 串口 IMU"
            self.runtime_state.set_state(self.bridge.last_service_message, "ok")
        else:
            detail = status or self.bridge.direct_sensor_status or self.bridge.direct_sensor.error
            self.bridge.last_service_message = "直连采集启动失败：" + detail
            self.runtime_state.set_state(self.bridge.last_service_message, "bad")
        self.one_click_button.setEnabled(ok and self.bridge.direct_sensor.available())

    def _one_click_record(self) -> None:
        self.pending_auto_record = True
        if not self.bridge.direct_sensor.is_started():
            self._start_runtime_nodes()
        if self._ready_for_auto_record():
            self._start_recording_from_gui("一键采集")
            return

        missing = self._readiness_missing_reasons()
        if missing:
            self.bridge.last_service_message = "一键采集：等待 " + "、".join(missing) + " ready"
        else:
            self.bridge.last_service_message = "一键采集：准备完成，等待录制服务响应"

    def _stop_runtime_nodes(self) -> None:
        self.pending_auto_record = False
        if self.direct_start_in_progress:
            self.bridge.last_service_message = "直连采集仍在启动中，等启动返回后再停止"
            return
        if not self.bridge.direct_sensor.is_started():
            self.bridge.last_service_message = "直连采集未运行"
            return
        self.bridge.stop_direct_sensor()
        self.bridge.last_service_message = "已停止直连采集"

    def _kill_runtime_if_needed(self) -> None:
        return

    def _runtime_finished(self, exit_code: int, exit_status: QProcess.ExitStatus) -> None:
        del exit_status
        self.pending_auto_record = False
        self.bridge.last_service_message = f"直连采集已退出，退出码={exit_code}"

    def _runtime_error(self, error: QProcess.ProcessError) -> None:
        self.pending_auto_record = False
        self.bridge.last_service_message = f"采集节点启动异常：{error}"

    def _topic_group(self) -> QGroupBox:
        group = QGroupBox("直连输入")
        layout = QGridLayout(group)
        self._row(layout, 0, "图像", "image_topic")
        self._row(layout, 1, "IMU", "imu_topic")
        self._row(layout, 2, "CameraInfo", "camera_info_topic")
        self._row(layout, 3, "CubePose", "cube_pose_topic")
        self._row(layout, 4, "PoseStatus", "cube_pose_status_topic")
        return group

    def _recorder_group(self) -> QGroupBox:
        group = QGroupBox("直连数据录制")
        layout = QGridLayout(group)
        layout.setHorizontalSpacing(8)
        layout.setVerticalSpacing(5)
        self._row(layout, 0, "状态", "recording")
        self._row(layout, 1, "数据目录", "bag_path")
        self._row(layout, 2, "保存位置", "bag_save_path")
        self._row(layout, 3, "数据计数", "message_counts")
        self._row(layout, 4, "录制时间", "record_time")
        self._row(layout, 5, "当前动作时间", "action_time")
        layout.addWidget(QLabel("分段采集"), 6, 0)
        layout.addWidget(self._phase_progress_widget(), 6, 1)
        self._row(layout, 7, "服务提示", "service_message")
        for key in ("bag_save_path", "service_message"):
            self.values[key].setWordWrap(False)
            self.values[key].setMaximumHeight(32)
        self.values["action_time"].setWordWrap(True)
        self.values["action_time"].setMinimumHeight(42)
        self.values["action_time"].setMaximumHeight(48)

        label = QLabel("正式进度")
        self.progress_bar = QProgressBar()
        self.progress_bar.setRange(0, 1000)
        self.progress_bar.setValue(0)
        self.progress_bar.setTextVisible(True)
        layout.addWidget(label, 8, 0)
        layout.addWidget(self.progress_bar, 8, 1)

        self.stop_button = QPushButton("停止录制")
        self.stop_button.setObjectName("DangerAction")
        self.stop_button.clicked.connect(self._manual_stop_recording)
        layout.addWidget(self._fit_right_button(self.stop_button), 9, 0, 1, 2)

        layout.setColumnStretch(1, 1)
        return group

    def _phase_progress_widget(self) -> QWidget:
        panel = QWidget()
        layout = QGridLayout(panel)
        layout.setContentsMargins(0, 0, 0, 0)
        layout.setHorizontalSpacing(6)
        layout.setVerticalSpacing(5)
        self.phase_progress_bars = []
        self.phase_status_values = []
        for index, (title, _, _) in enumerate(ACTION_PHASES):
            compact_title = (
                title.replace("多面过渡+小平移", "多面+平移")
                .replace("8字+三轴组合", "8字+组合")
            )
            item = QWidget()
            item_layout = QGridLayout(item)
            item_layout.setContentsMargins(0, 0, 0, 0)
            item_layout.setHorizontalSpacing(5)
            label = QLabel(f"{index + 1}. {compact_title}")
            label.setMinimumWidth(82)
            label.setStyleSheet("QLabel { color: #344054; font-weight: 600; }")
            bar = QProgressBar()
            bar.setRange(0, 1000)
            bar.setValue(0)
            bar.setTextVisible(True)
            bar.setFormat("待采集")
            bar.setMinimumHeight(20)
            self._set_phase_bar_state(bar, "neutral")
            status = StatusValue("待采集")
            status.setMinimumWidth(86)
            status.setMinimumHeight(20)
            item_layout.addWidget(label, 0, 0)
            item_layout.addWidget(bar, 0, 1)
            item_layout.addWidget(status, 0, 2)
            item_layout.setColumnStretch(1, 1)
            layout.addWidget(item, index, 0)
            self.phase_progress_bars.append(bar)
            self.phase_status_values.append(status)
        layout.setColumnStretch(0, 1)
        return panel

    def _topic_state_from_count(self, topic: str, count: int) -> tuple[str, str]:
        state = "ok" if count > 0 else "bad"
        text = f"{topic}  发布者={count}"
        return text, state

    def _on_timer(self) -> None:
        self.bridge.spin_once()
        self._consume_direct_start_result()
        if self.pending_auto_record and self._ready_for_auto_record():
            self._start_recording_from_gui("一键采集")
        self._update_services()
        self._update_runtime()
        self._update_topics()
        self._update_node_chain()
        self._update_visuals()
        self._update_recorder()
        self._update_motion_hint()
        self._update_tip()
        self._schedule_content_splitter_restore()

    def _image_stream_ready(self, now: float) -> bool:
        stats = self.bridge.direct_sensor.stats()
        direct_rate = max(float(stats.image_rate_hz), float(stats.actual_color_fps))
        return (
            self.bridge.last_image_wall > 0.0
            and now - self.bridge.last_image_wall <= 2.0
            and max(self.bridge.image_rate_hz(), direct_rate) + RATE_READY_TOLERANCE_HZ
            >= MIN_RECORD_IMAGE_RATE_HZ
        )

    def _imu_stream_ready(self, now: float) -> bool:
        direct_rate = float(self.bridge.direct_sensor.stats().imu_rate_hz)
        return (
            bool(self.bridge.imu_samples)
            and now - self.bridge.last_imu_wall <= 2.0
            and max(self.bridge.imu_rate_hz(), direct_rate) + RATE_READY_TOLERANCE_HZ
            >= MIN_RECORD_IMU_RATE_HZ
        )

    def _camera_info_ready(self, now: float) -> bool:
        del now
        return self.bridge.camera_info_ready

    def _ready_for_auto_record(self) -> bool:
        if not self.bridge.direct_sensor.is_started():
            return False

        now = time.monotonic()
        image_ready = self._image_stream_ready(now)
        imu_ready = self._imu_stream_ready(now)
        camera_info_ready = self._camera_info_ready(now)

        return image_ready and imu_ready and camera_info_ready

    def _nearest_imu_delta_for_image(self) -> float | None:
        image_stamp = float(self.bridge.last_image_stamp)
        if image_stamp <= 0.0:
            return None
        imu_stamps = [
            float(sample[0])
            for sample in self.bridge.imu_samples
            if sample and float(sample[0]) > 0.0
        ]
        if not imu_stamps:
            return None
        nearest = min(imu_stamps, key=lambda stamp: abs(stamp - image_stamp))
        return image_stamp - nearest

    def _timestamp_ready(self) -> bool:
        image_stamp = float(self.bridge.last_image_stamp)
        camera_info_stamp = float(self.bridge.last_camera_info_stamp)
        nearest_imu_delta = self._nearest_imu_delta_for_image()
        if image_stamp <= 0.0 or camera_info_stamp <= 0.0 or nearest_imu_delta is None:
            return False
        issues = (
            self.bridge.last_image_stamp_issue,
            self.bridge.last_imu_stamp_issue,
            self.bridge.last_camera_info_stamp_issue,
        )
        if any(issue and issue != "ok" for issue in issues):
            return False
        image_info_delta = image_stamp - camera_info_stamp
        return max(abs(nearest_imu_delta), abs(image_info_delta)) <= 0.05

    def _manual_start_recording(self) -> None:
        if not self.bridge.direct_sensor.is_started():
            self.bridge.last_service_message = "开始录制失败：直连采集未启动"
            return
        if not self._ready_for_auto_record():
            missing = "、".join(self._readiness_missing_reasons()) or "数据流稳定"
            self.bridge.last_service_message = (
                f"开始录制已拦截：请等待 {missing} ready"
            )
            return
        self._start_recording_from_gui("手动开始")

    def _start_recording_from_gui(self, source: str) -> None:
        self.pending_auto_record = False
        warnings = self._readiness_warning_reasons()
        self.bridge.start_recording()
        if warnings:
            self.bridge.last_service_message = (
                f"{source}：录制请求已发送；注意 " + "、".join(warnings)
            )

    def _start_current_phase_collection(self) -> None:
        status = self.bridge.last_status
        if status is None or not status.recording:
            self.bridge.last_service_message = "本段采集失败：请先开始直连录制"
            return
        if self.phase_workflow_mode != "detect":
            self.bridge.last_service_message = "本段采集失败：当前阶段不是姿态检测状态"
            return
        if self.phase_workflow_index >= len(ACTION_PHASES):
            self.bridge.last_service_message = "本段采集失败：六段采集已经完成"
            return

        schedule = self._action_schedule(self.bridge.record_duration_sec)
        duration = self._phase_duration(schedule, self.phase_workflow_index)
        detect_sec = min(self._phase_precheck_duration(duration), duration * 0.8)
        state, text = self._phase_precheck_detection(
            self.phase_workflow_index,
            self.phase_detect_start_wall,
            detect_sec,
        )
        self.current_phase_detection = (state, text)
        if state != "ok":
            self.bridge.last_service_message = f"本段采集未开始：{text}，请调整姿态直到状态变绿"
            return

        self.phase_workflow_mode = "collect"
        self.phase_collect_start_wall = time.monotonic()
        self.phase_collect_elapsed = 0.0
        title = ACTION_PHASES[self.phase_workflow_index][0]
        self.bridge.last_service_message = (
            f"开始第 {self.phase_workflow_index + 1} 段“{title}”正式采集，"
            f"本段时长 {fmt_seconds(duration)}"
        )

    def _manual_stop_recording(self) -> None:
        self.pending_auto_record = False
        status = self.bridge.last_status
        if status is None:
            self.bridge.last_service_message = "停止录制失败：直连记录器未初始化"
            return
        if status is not None and not status.recording:
            self.bridge.last_service_message = "停止录制：当前没有正在录制的数据集"
            return
        self.stop_record_request_wall = time.monotonic()
        self.stop_button.setEnabled(False)
        self.bridge.stop_recording()

    def _readiness_missing_reasons(self) -> list[str]:
        now = time.monotonic()
        reasons = []
        if not self.bridge.direct_sensor.is_started():
            reasons.append("直连采集")
        if not self._image_stream_ready(now):
            reasons.append(f"图像≥{MIN_RECORD_IMAGE_RATE_HZ:.0f}Hz")
        if not self._imu_stream_ready(now):
            reasons.append(f"串口 IMU≥{MIN_RECORD_IMU_RATE_HZ:.0f}Hz")
        if not self._camera_info_ready(now):
            reasons.append("CameraInfo")
        return reasons

    def _readiness_warning_reasons(self) -> list[str]:
        if self._timestamp_ready():
            return []
        return ["时间戳诊断未变绿，数据会继续录制，后处理时请检查同步"]

    def _update_services(self) -> None:
        ready_to_record = self._ready_for_auto_record()
        status = self.bridge.last_status
        recording = bool(
            status is not None
            and status.recording
            and time.monotonic() - self.bridge.last_status_wall <= 3.0
        )
        stop_pending = (
            self.stop_record_request_wall > 0.0
            and time.monotonic() - self.stop_record_request_wall <= 10.0
        )
        if status is not None and not status.recording:
            self.stop_record_request_wall = 0.0
            stop_pending = False
        self.one_click_button.setEnabled(
            self.bridge.direct_sensor.available()
            and not recording
            and not self.direct_start_in_progress
        )
        self.stop_button.setEnabled(recording and not stop_pending)
        self.stop_button.setText("停止中..." if stop_pending and recording else "停止录制")
        settings_enabled = not self.bridge.direct_sensor.is_started() and not self.direct_start_in_progress
        for widget in (
            self.image_topic_edit,
            self.imu_topic_edit,
            self.camera_info_topic_edit,
            self.bag_path_edit,
            self.duration_spin,
            self.camera_frame_edit,
            self.imu_frame_edit,
            self.serial_port_edit,
        ):
            widget.setEnabled(settings_enabled)

    def _update_runtime(self) -> None:
        if self.direct_start_in_progress:
            elapsed = time.monotonic() - self.direct_start_requested_wall
            self.runtime_state.set_state(
                f"直连采集启动中 {elapsed:.1f}s：正在打开 Orbbec 相机和串口 IMU",
                "warn",
            )
            self.prepare_button.setEnabled(False)
            self.stop_runtime_button.setEnabled(False)
        elif self.bridge.direct_sensor.is_started():
            stats = self.bridge.direct_sensor.stats()
            image_rate = max(self.bridge.image_rate_hz(), float(stats.image_rate_hz))
            imu_rate = max(self.bridge.imu_rate_hz(), float(stats.imu_rate_hz))
            self.runtime_state.set_state(
                f"直连采集运行：image={image_rate:.1f}Hz "
                f"imu={imu_rate:.1f}Hz "
                f"actual={stats.actual_color_width}x{stats.actual_color_height}@{stats.actual_color_fps}",
                "ok" if self._image_stream_ready(time.monotonic()) else "warn",
            )
            self.prepare_button.setEnabled(False)
            self.stop_runtime_button.setEnabled(True)
        else:
            text = "直连采集未启动"
            if not self.bridge.direct_sensor.available():
                text += "；" + self.bridge.direct_sensor.error
            elif self.bridge.direct_sensor_status and self.bridge.direct_sensor_status != "直连采集库已加载":
                text += "；" + self.bridge.direct_sensor_status
            self.runtime_state.set_state(text, "warn")
            self.prepare_button.setEnabled(True)
            self.stop_runtime_button.setEnabled(False)

        missing = self._readiness_missing_reasons()
        if missing:
            self.one_click_button.setToolTip("等待：" + "、".join(missing))
        else:
            warnings = self._readiness_warning_reasons()
            ready_tip = (
                "图像、IMU、CameraInfo 已 ready，可以录制"
                if not self.bridge.enable_cube_detection
                else "图像、IMU、CameraInfo 已 ready，可以录制"
            )
            if warnings:
                ready_tip += "；注意：" + "、".join(warnings)
            self.one_click_button.setToolTip(ready_tip)

    def _update_topics(self) -> None:
        stats = self.bridge.direct_sensor.stats()
        image_rate = max(self.bridge.image_rate_hz(), float(stats.image_rate_hz))
        imu_rate = max(self.bridge.imu_rate_hz(), float(stats.imu_rate_hz))
        image_state = "ok" if self._image_stream_ready(time.monotonic()) else "warn"
        imu_state = "ok" if imu_rate >= MIN_RECORD_IMU_RATE_HZ else "warn"
        camera_state = "ok" if self.bridge.camera_info_ready else "warn"
        self.values["image_topic"].set_state(
            f"Orbbec SDK 直连 image={image_rate:.1f}Hz "
            f"目标≥{MIN_RECORD_IMAGE_RATE_HZ:.0f}Hz  "
            f"actual={stats.actual_color_width}x{stats.actual_color_height}@{stats.actual_color_fps}",
            image_state,
        )
        self.values["imu_topic"].set_state(
            f"串口 {self.bridge.serial_port} imu={imu_rate:.1f}Hz "
            f"目标≥{MIN_RECORD_IMU_RATE_HZ:.0f}Hz  dropped={int(stats.dropped_imu_count)}",
            imu_state,
        )
        self.values["camera_info_topic"].set_state(
            self.bridge.last_camera_info_info if self.bridge.camera_info_ready else "等待 Orbbec SDK 内参",
            camera_state,
        )
        self.values["cube_pose_topic"].set_state(
            f"GUI 内部检测 CubePose：{self.bridge.cube_pose_rate_hz():.1f}Hz，CSV={self.bridge.pose_csv_path}",
            "ok" if (not self.bridge.enable_cube_detection or self.bridge.cube_pose_rate_hz() > 0.1) else "warn",
        )
        self.values["cube_pose_status_topic"].set_state(
            self.bridge.last_tag_warning,
            "ok" if self.bridge.last_detection_tag_count > 0 or not self.bridge.enable_cube_detection else "warn",
        )

    def _update_node_chain(self) -> None:
        stats = self.bridge.direct_sensor.stats()
        sensor_started = self.bridge.direct_sensor.is_started()
        image_rate = max(self.bridge.image_rate_hz(), float(stats.image_rate_hz))
        imu_rate = max(self.bridge.imu_rate_hz(), float(stats.imu_rate_hz))
        if sensor_started:
            image_ready = image_rate + RATE_READY_TOLERANCE_HZ >= MIN_RECORD_IMAGE_RATE_HZ
            self.values["real_sensor_node"].set_state(
                f"直连运行：{self.bridge.direct_sensor_status}；"
                f"image={image_rate:.1f}Hz imu={imu_rate:.1f}Hz",
                "ok" if image_ready else "warn",
            )
        else:
            self.values["real_sensor_node"].set_state(
                "直连采集未运行：点击“启动直连采集”打开 Orbbec 相机 + Cube 串口 IMU",
                "bad" if self.bridge.direct_sensor.available() else "warn",
            )

        status = self.bridge.last_status
        recording = bool(status is not None and status.recording)
        self.values["data_recorder_node"].set_state(
            (
                f"直连写入中：images + image_timestamps.csv + imu.csv，path={status.bag_path}"
                if recording and status is not None
                else "直连记录器空闲：开始录制后写入 images/imu/camera_info 数据集"
            ),
            "ok" if recording else "neutral",
        )

        camera_info_text = (
            self.bridge.last_camera_info_info
            if self.bridge.camera_info_ready
            else "暂用 fallback 内参，等待 Orbbec SDK CameraInfo"
        )
        self.values["calibration_gui_node"].set_state(
            f"在线：Cube 检测 {'开启' if self.bridge.enable_cube_detection else '关闭'}；{camera_info_text}",
            "ok",
        )

        cube_pose_rate = self.bridge.cube_pose_rate_hz()
        now = time.monotonic()
        image_ready = self._image_stream_ready(now)
        imu_ready = self._imu_stream_ready(now)
        camera_info_ready = self._camera_info_ready(now)
        stream_ok = image_ready and imu_ready and camera_info_ready
        stream_discovered = sensor_started
        if stream_ok:
            stream_state = "ok"
        elif stream_discovered:
            stream_state = "warn"
        else:
            stream_state = "bad"
        camera_info_text = "ready" if camera_info_ready else "waiting"
        self.values["live_stream"].set_state(
            f"直连硬件：image={image_rate:.1f}Hz(≥{MIN_RECORD_IMAGE_RATE_HZ:.0f}), "
            f"imu={imu_rate:.1f}Hz(≥{MIN_RECORD_IMU_RATE_HZ:.0f}), "
            f"cube_pose={cube_pose_rate:.1f}Hz, camera_info={camera_info_text}, "
            f"frames={int(stats.image_count)} imu_samples={int(stats.imu_count)}",
            stream_state,
        )
        self._update_imu_diagnostic(1 if stats.imu_online else 0)
        self._update_timestamp_diag()

    def _serial_port_status_text(self) -> tuple[str, str]:
        configured = self.bridge.serial_port
        ports = sorted(glob.glob("/dev/ttyACM*") + glob.glob("/dev/ttyUSB*"))
        active_gids = set(os.getgroups())
        active_gids.add(os.getgid())
        active_groups = {group.gr_name for group in grp.getgrall() if group.gr_gid in active_gids}
        dialout_hint = "" if "dialout" in active_groups else "；当前 GUI 会话未生效 dialout 权限"
        if configured == "auto":
            if ports:
                return f"auto 可见串口={','.join(ports)}{dialout_hint}", "ok" if "dialout" in active_groups else "warn"
            return f"auto 未发现 /dev/ttyACM* 或 /dev/ttyUSB*{dialout_hint}", "bad"
        if os.path.exists(configured):
            if os.access(configured, os.R_OK | os.W_OK):
                return f"串口存在且可读写 {configured}", "ok"
            return f"串口存在但无读写权限 {configured}{dialout_hint}", "bad"
        suffix = f"；可见串口={','.join(ports)}" if ports else "；未发现 ttyACM/ttyUSB"
        return f"串口不存在 {configured}{suffix}{dialout_hint}", "bad"

    def _update_imu_diagnostic(self, imu_publishers: int) -> None:
        now = time.monotonic()
        serial_text, serial_state = self._serial_port_status_text()
        stats = self.bridge.direct_sensor.stats()
        imu_rate = max(self.bridge.imu_rate_hz(), float(stats.imu_rate_hz))
        has_samples = bool(self.bridge.imu_samples)
        fresh = has_samples and now - self.bridge.last_imu_wall <= 2.0

        if imu_publishers <= 0:
            self.values["imu_diagnostic"].set_state(
                f"未收到 Cube 串口 IMU 直连数据：检查直连采集、串口和权限；Orbbec 相机内置 IMU 不计入本流程；{serial_text}",
                "bad",
            )
            return

        if not has_samples:
            state = "bad" if serial_state == "bad" else "warn"
            self.values["imu_diagnostic"].set_state(
                f"串口 IMU 已启动但还没有解析到六轴样本：检查波特率、startup_command、协议；{serial_text}",
                state,
            )
            return

        if not fresh or imu_rate <= 0.1:
            age = now - self.bridge.last_imu_wall
            self.values["imu_diagnostic"].set_state(
                f"串口 IMU 曾收到数据但已 {age:.1f}s 无新样本，当前 {imu_rate:.1f}Hz；{serial_text}",
                "bad" if age > 5.0 else "warn",
            )
            return

        stamp_text = (
            f"stamp={self.bridge.last_imu_stamp:.6f}"
            if self.bridge.last_imu_stamp > 0.0 else "stamp=0/未就绪"
        )
        self.values["imu_diagnostic"].set_state(
            f"Cube 串口 IMU 直连正常：{imu_rate:.1f}Hz, {stamp_text}；{serial_text}",
            "ok",
        )

    def _update_timestamp_diag(self) -> None:
        image_stamp = float(self.bridge.last_image_stamp)
        imu_stamp = float(self.bridge.last_imu_stamp)
        camera_info_stamp = float(self.bridge.last_camera_info_stamp)
        nearest_imu_delta = self._nearest_imu_delta_for_image()
        issues = [
            issue
            for issue in (
                self.bridge.last_image_stamp_issue,
                self.bridge.last_imu_stamp_issue,
                self.bridge.last_camera_info_stamp_issue,
            )
            if issue and issue != "ok"
        ]
        parts = []
        if nearest_imu_delta is not None:
            parts.append(f"最近IMU偏差={nearest_imu_delta:+.4f}s")
        if image_stamp > 0.0 and camera_info_stamp > 0.0:
            parts.append(f"图像-内参={image_stamp - camera_info_stamp:+.4f}s")
        if image_stamp > 0.0 and imu_stamp > 0.0:
            latest_delta = image_stamp - imu_stamp
        else:
            latest_delta = None

        max_delta = max(
            abs(nearest_imu_delta) if nearest_imu_delta is not None else float("inf"),
            abs(image_stamp - camera_info_stamp)
            if image_stamp > 0.0 and camera_info_stamp > 0.0
            else float("inf"),
        )

        if not parts:
            text = "等待三路 header.stamp"
            state = "warn"
        else:
            text = "  ".join(parts)
            state = "ok" if max_delta <= 0.05 else "warn"
        if issues:
            text += "  |  " + "；".join(issues)
            state = "bad"
        tooltip = text
        if latest_delta is not None:
            tooltip += (
                f"\n最新图像-最新IMU={latest_delta:+.4f}s；"
                "图像 15Hz、IMU 500Hz 时该值会随 GUI 刷新抖动，判定以最近IMU偏差为准。"
            )
        self.values["timestamp_diag"].set_state(text, state, tooltip)

    def _make_preview_pixmap(self, image: QImage, source: str, age: float) -> QPixmap:
        del age
        stats = self.bridge.direct_sensor.stats()
        image_rate = max(self.bridge.image_rate_hz(), float(stats.image_rate_hz))
        target_size = self.image_preview.size()
        target_width = max(1, target_size.width())
        target_height = max(1, target_size.height())
        cache_key = (
            int(getattr(self.bridge, "last_image_seq", 0)),
            target_width,
            target_height,
            source,
        )
        if self._preview_cache_key == cache_key and self._preview_cache_pixmap is not None:
            return self._preview_cache_pixmap

        canvas = QPixmap(target_width, target_height)
        canvas.fill(QColor("#0f172a"))

        available_width = max(1, target_width - 16)
        available_height = max(1, target_height - 16)
        frame = QPixmap.fromImage(image).scaled(
            available_width,
            available_height,
            Qt.KeepAspectRatio,
            Qt.SmoothTransformation,
        )
        frame_x = (target_width - frame.width()) // 2
        frame_y = (target_height - frame.height()) // 2
        frame_rect = QRect(frame_x, frame_y, frame.width(), frame.height())

        painter = QPainter(canvas)
        painter.setRenderHint(QPainter.Antialiasing)
        painter.drawPixmap(frame_rect.topLeft(), frame)
        painter.setPen(QPen(QColor("#e4e7ec"), 1))
        painter.drawRect(frame_rect.adjusted(0, 0, -1, -1))

        label_width = min(max(240, frame.width() - 16), 620)
        label_rect = QRect(frame_x + 8, frame_y + 8, label_width, 28)
        label_bg = QColor(16, 24, 40, 190)
        painter.fillRect(label_rect, label_bg)
        painter.setPen(QColor("#ffffff"))
        painter.setFont(QFont("Sans Serif", 9, QFont.Medium))
        painter.drawText(
            label_rect.adjusted(8, 0, -8, 0),
            Qt.AlignVCenter | Qt.AlignLeft,
            f"{source}  preview={image.width()}x{image.height()}  "
            f"capture={image_rate:.1f}Hz",
        )
        painter.end()
        self._preview_cache_key = cache_key
        self._preview_cache_pixmap = canvas
        return canvas

    def _update_visuals(self) -> None:
        now = time.monotonic()

        image_stale = self.bridge.last_image is None or now - self.bridge.last_image_wall > 2.0
        if image_stale:
            self.image_preview.clear()
            self.image_preview.setText("等待图像")
            if self.bridge.last_image_error:
                self.image_info.set_state(self.bridge.last_image_error, "bad")
            else:
                self.image_info.set_state(self.bridge.last_image_info, "warn")
        else:
            age = now - self.bridge.last_image_wall
            source = self._preview_source_label()
            stats = self.bridge.direct_sensor.stats()
            image_rate = max(self.bridge.image_rate_hz(), float(stats.image_rate_hz))
            display_period = 1.0 / max(float(self.bridge.gui_display_rate_hz), 1e-6)
            if now - self._last_preview_paint_wall >= display_period:
                self.image_preview.setPixmap(
                    self._make_preview_pixmap(self.bridge.last_image, source, age)
                )
                self._last_preview_paint_wall = now
            image_detail = (
                f"{source}  {self.bridge.last_image_info}  "
                f"capture={image_rate:.1f}Hz  "
                f"preview={self.bridge.image_preview_rate_hz:.1f}Hz  "
                f"detect={self.bridge.tag_detection_rate_hz:.1f}Hz@{self.bridge.tag_detection_scale:.2f}x  "
                f"display={self.bridge.gui_display_rate_hz:.1f}Hz@{self.bridge.preview_scale:.2f}x  "
                f"cube_pose={self.bridge.cube_pose_rate_hz():.1f}Hz  "
                f"录制预览  延迟 {age:.2f}s"
            )
            self.image_info.set_state(
                f"{source}  图像 {image_rate:.1f}Hz  Pose {self.bridge.cube_pose_rate_hz():.1f}Hz  延迟 {age:.2f}s",
                "ok",
                image_detail,
            )

        imu_stale = not self.bridge.imu_samples or now - self.bridge.last_imu_wall > 2.0
        self.imu_plot.set_samples(self.bridge.imu_samples)
        if imu_stale:
            self.imu_accel_value.set_state("-", "warn")
            self.imu_gyro_value.set_state("-", "warn")
            self.imu_norm_value.set_state("-", "warn")
            self.imu_info.set_state(self.bridge.last_imu_info, "warn")
        else:
            age = now - self.bridge.last_imu_wall
            source = self._preview_source_label()
            stats = self.bridge.direct_sensor.stats()
            imu_rate = max(self.bridge.imu_rate_hz(), float(stats.imu_rate_hz))
            self.imu_accel_value.set_state(self.bridge.last_imu_accel_g_info, "ok")
            self.imu_gyro_value.set_state(self.bridge.last_imu_gyro_deg_info, "ok")
            self.imu_norm_value.set_state(self.bridge.last_imu_norm_info, "ok")
            imu_detail = (
                f"{source}  {self.bridge.last_imu_info}  hz={imu_rate:.1f}  延迟 {age:.2f}s"
            )
            self.imu_info.set_state(
                f"{source}  IMU {imu_rate:.1f}Hz  延迟 {age:.2f}s",
                "ok",
                imu_detail,
            )

    def _update_recorder(self) -> None:
        status = self.bridge.last_status
        stale = status is None or time.monotonic() - self.bridge.last_status_wall > 3.0
        bag_save_path, bag_save_state, bag_save_tooltip = self._bag_path_for_display(status)
        if stale:
            self.values["recording"].set_state("等待直连记录器状态", "warn")
            bag_path = self.bridge.bag_path or "-"
            self.values["bag_path"].set_state(
                self._compact_path(bag_path),
                "neutral",
                os.path.abspath(os.path.expanduser(str(bag_path))),
            )
            self.values["bag_save_path"].set_state(
                bag_save_path,
                bag_save_state,
                bag_save_tooltip,
            )
            self.values["message_counts"].set_state("-", "neutral")
            self.values["record_time"].set_state("-", "neutral")
        else:
            stop_pending = (
                status.recording
                and self.stop_record_request_wall > 0.0
                and time.monotonic() - self.stop_record_request_wall <= 10.0
            )
            self.values["recording"].set_state(
                "停止中：正在关闭数据集" if stop_pending else "录制中" if status.recording else "空闲",
                "warn" if stop_pending else "ok" if status.recording else "warn",
            )
            status_bag_path = status.bag_path or "-"
            self.values["bag_path"].set_state(
                self._compact_path(status_bag_path),
                "neutral",
                os.path.abspath(os.path.expanduser(str(status_bag_path))),
            )
            self.values["bag_save_path"].set_state(
                bag_save_path,
                bag_save_state,
                bag_save_tooltip,
            )
            cube_pose_count = int(getattr(status, "cube_pose_count", 0))
            cube_pose_status_count = int(getattr(status, "cube_pose_status_count", 0))
            elapsed_for_rate = max(float(status.elapsed_sec), 1e-6)
            image_record_rate = float(status.image_count) / elapsed_for_rate if status.recording else 0.0
            cube_pose_record_rate = float(cube_pose_count) / elapsed_for_rate if status.recording else 0.0
            message_counts = (
                f"image={status.image_count}   imu={status.imu_count}   "
                f"camera_info={status.camera_info_count}   cube_pose={cube_pose_count}   "
                f"cube_pose_status={cube_pose_status_count}   "
                f"rec_hz image={image_record_rate:.1f} cube_pose={cube_pose_record_rate:.1f}"
            )
            self.values["message_counts"].set_state(
                self._compact_text(message_counts, 88),
                "ok"
                if status.image_count > 0
                and status.imu_count > 0
                and status.camera_info_count > 0
                and (not self.bridge.enable_cube_detection or cube_pose_count > 0)
                and (not status.recording or image_record_rate >= 8.0)
                else "warn",
                message_counts,
            )
            if status.target_duration_sec > 0.0:
                self.values["record_time"].set_state(
                    f"{status.elapsed_sec:.1f}s / {status.target_duration_sec:.1f}s",
                    "ok" if status.recording else "neutral",
                )
            elif status.recording:
                self.values["record_time"].set_state(f"{status.elapsed_sec:.1f}s / 手动停止", "ok")
            else:
                self.values["record_time"].set_state(f"上次 {status.elapsed_sec:.1f}s", "neutral")
        self._update_action_timing(None if stale else status)
        self._update_formal_collection_progress(None if stale else status)
        self.values["service_message"].set_state(
            self._compact_text(self.bridge.last_service_message, 120),
            "neutral",
            self.bridge.last_service_message,
        )

    def _reset_motion_guide(self, phase_mode: str = "detect") -> None:
        self.motion_phase = 0
        self.motion_phase_start_wall = time.monotonic()
        self.motion_last_imu_stamp = None
        self.motion_phase_abs_angle = np.zeros(3, dtype=float)
        self.motion_phase_signed_angle = np.zeros(3, dtype=float)
        self.motion_latest_gyro = np.zeros(3, dtype=float)
        self.motion_stable_start_wall = None
        self.motion_phase_positions.clear()
        self.recording_wall_start = 0.0
        self._reset_phase_workflow(phase_mode)

    def _advance_motion_phase(self) -> None:
        self.motion_phase = min(self.motion_phase + 1, 6)
        self.motion_phase_start_wall = time.monotonic()
        self.motion_last_imu_stamp = None
        self.motion_phase_abs_angle = np.zeros(3, dtype=float)
        self.motion_phase_signed_angle = np.zeros(3, dtype=float)
        self.motion_stable_start_wall = None
        self.motion_phase_positions.clear()

    def _consume_motion_imu(self) -> None:
        samples = list(self.bridge.imu_samples)
        if not samples:
            return
        for sample in samples:
            stamp = float(sample[0])
            if self.motion_last_imu_stamp is None:
                self.motion_last_imu_stamp = stamp
                self.motion_latest_gyro = np.array(sample[4:7], dtype=float)
                continue
            if stamp <= self.motion_last_imu_stamp:
                continue
            dt = max(0.0, min(stamp - self.motion_last_imu_stamp, 0.05))
            gyro = np.array(sample[4:7], dtype=float)
            self.motion_phase_abs_angle += np.abs(gyro) * dt
            self.motion_phase_signed_angle += gyro * dt
            self.motion_latest_gyro = gyro
            self.motion_last_imu_stamp = stamp

    def _append_motion_pose(self) -> None:
        pose = self.bridge.last_pose
        if pose is None or time.monotonic() - self.bridge.last_pose_wall > 2.0:
            return
        self.motion_phase_positions.append(
            np.array([pose.translation.x, pose.translation.y, pose.translation.z], dtype=float)
        )

    def _motion_position_span(self) -> float:
        if len(self.motion_phase_positions) < 2:
            return 0.0
        points = np.vstack(self.motion_phase_positions)
        return float(np.linalg.norm(np.max(points, axis=0) - np.min(points, axis=0)))

    @staticmethod
    def _progress_text(value: float) -> str:
        return f"{max(0.0, min(1.0, value)) * 100.0:.0f}%"

    def _recording_motion_hint(self, status, pose_stale: bool) -> tuple[str, str]:
        del pose_stale
        if status is None:
            self.motion_phase_progress = 0.0
            return (
                "采集前：确认左侧 IMU 波形在动、中间视频正常刷新、CameraInfo 已 ready。"
                "点击一键采集后，右侧会先做每段姿态检测，再累计 150s 正式数据。",
                "warn",
            )

        if not status.recording:
            self.motion_phase_progress = 0.0
            return (
                "准备就绪：开始录制后，保持相机固定不动，只移动手里的 IMU 模块。"
                "动作要慢、连续，视频里能看到正在录即可。",
                "ok",
            )

        if self.current_action_time_text:
            guide_state = (
                "warn"
                if self.phase_workflow_mode == "detect" and self.current_phase_detection[0] != "ok"
                else "ok"
            )
            return (
                "按分段采集流程执行：\n"
                f"{self.current_action_time_text}\n"
                "切换规则：每段先判断 Cube pose/IMU 是否合格，变绿后自动采集；本段计满再进入下一段检测。"
                "如果视频模糊或动作不对，先放慢并重新做当前段。",
                guide_state,
            )

        self._consume_motion_imu()
        self._append_motion_pose()

        now = time.monotonic()
        elapsed = max(0.0, now - self.motion_phase_start_wall)
        gyro_deg = self.motion_latest_gyro * RAD_TO_DEG
        gyro_norm = float(np.linalg.norm(gyro_deg))

        if self.motion_phase == 0:
            if gyro_norm < 5.0:
                if self.motion_stable_start_wall is None:
                    self.motion_stable_start_wall = now
                stable_sec = now - self.motion_stable_start_wall
            else:
                self.motion_stable_start_wall = None
                stable_sec = 0.0
            if stable_sec >= 2.0:
                self.motion_phase_progress = 1.0
                self._advance_motion_phase()
                return "阶段 1 完成。下一步：绕红色 +X 轴慢速往返，+X 指向 ID2/right。", "ok"
            self.motion_phase_progress = stable_sec / 2.0
            return (
                "阶段 1/6：请将带 Tag 的 IMU 模块放在画面中央，保持静止 2 秒。"
                f"静止进度 {self._progress_text(stable_sec / 2.0)}，当前 |w|={gyro_norm:.1f} deg/s。",
                "ok" if stable_sec > 0.5 else "warn",
            )

        if self.motion_phase in (1, 2, 3):
            axis_index = self.motion_phase - 1
            axis_name = ("X", "Y", "Z")[axis_index]
            axis_angle_deg = float(self.motion_phase_abs_angle[axis_index] * RAD_TO_DEG)
            total_angle_deg = float(np.sum(self.motion_phase_abs_angle) * RAD_TO_DEG)
            dominance = axis_angle_deg / max(total_angle_deg, 1e-6)
            progress = axis_angle_deg / 60.0
            if axis_angle_deg >= 60.0 and dominance >= 0.55:
                self.motion_phase_progress = 1.0
                self._advance_motion_phase()
                next_text = (
                    "请绕绿色 +Y 轴慢速往返，+Y 指向 ID3/bottom。"
                    if axis_name == "X"
                    else "请绕蓝色 +Z 轴慢速往返，+Z 指向 Cube 内。"
                    if axis_name == "Y"
                    else "请做小幅平移 + 三轴组合旋转。"
                )
                return f"阶段 {axis_index + 2} 完成。下一步：{next_text}", "ok"
            state = "ok" if dominance >= 0.45 or total_angle_deg < 8.0 else "warn"
            self.motion_phase_progress = progress
            axis_hint = {
                "X": "红色 +X，指向 ID2/right",
                "Y": "绿色 +Y，指向 ID3/bottom",
                "Z": "蓝色 +Z，指向 Cube 内",
            }[axis_name]
            return (
                f"阶段 {axis_index + 2}/6：请绕{axis_hint}慢速往返 ±30°，不要甩动。"
                f"{axis_name}轴累计 {axis_angle_deg:.0f}° / 60°，主导占比 {dominance * 100.0:.0f}%，"
                f"进度 {self._progress_text(progress)}。",
                state,
            )

        if self.motion_phase == 4:
            axis_angles_deg = self.motion_phase_abs_angle * RAD_TO_DEG
            min_axis_angle = float(np.min(axis_angles_deg))
            position_span = self._motion_position_span()
            progress = min(min_axis_angle / 25.0, position_span / 0.04, elapsed / 5.0)
            if min_axis_angle >= 25.0 and position_span >= 0.04 and elapsed >= 5.0:
                self.motion_phase_progress = 1.0
                self._advance_motion_phase()
                return "阶段 5 完成。最后：保持 Cube 在视野中央，做慢速八字/斜向运动。", "ok"
            self.motion_phase_progress = progress
            return (
                "阶段 5/6：请做小幅平移 + 三轴组合旋转，Cube 始终留在画面内。"
                f"三轴最小累计 {min_axis_angle:.0f}° / 25°，平移范围 {position_span:.3f}m / 0.040m，"
                f"进度 {self._progress_text(progress)}。",
                "ok" if elapsed >= 2.0 else "warn",
            )

        if self.motion_phase == 5:
            progress = elapsed / 8.0
            if elapsed >= 8.0:
                self.motion_phase_progress = 1.0
                self._advance_motion_phase()
                return "阶段 6 完成：动作覆盖较充分。可以继续补充不同姿态，或等待录制自动结束。", "ok"
            self.motion_phase_progress = progress
            return (
                "阶段 6/6：请保持 Cube 在视野中央，做慢速八字/斜向运动，最后回到中央静止。"
                f"进度 {self._progress_text(progress)}。",
                "ok",
            )

        self.motion_phase_progress = 1.0
        return "运动向导已完成：继续保持视频清晰，补充不同姿态，等待录制结束。", "ok"

    def _update_motion_hint(self) -> None:
        status = self.bridge.last_status
        recording = bool(status is not None and status.recording)
        if recording and not self.motion_was_recording:
            self._reset_motion_guide("detect")
        elif not recording and self.motion_was_recording:
            self._reset_motion_guide("idle")
        self.motion_was_recording = recording
        hint, state = self._recording_motion_hint(status, False)
        self.current_motion_hint = hint
        self.motion_hint.set_state(hint, state)
        self.motion_guide.set_guide(
            self.scheduled_motion_phase if recording else self.motion_phase,
            recording,
            True,
            self.scheduled_motion_progress if recording else self.motion_phase_progress,
            self.current_action_time_text.split("\n", 2)[0],
            False,
        )

    def _update_tip(self) -> None:
        status = self.bridge.last_status
        now = time.monotonic()

        if status is None or not self.bridge.direct_sensor.is_started():
            detail = self.bridge.direct_sensor_status or self.bridge.direct_sensor.error
            self.tip.set_state(
                "GUI 已运行。请点击“启动直连采集”打开 Orbbec 相机和 Cube 串口 IMU。"
                + (f" 当前状态：{detail}" if detail else ""),
                "warn",
            )
        elif status.image_publishers == 0 or status.imu_publishers == 0 or status.camera_info_publishers == 0:
            missing = []
            if status.image_publishers == 0:
                missing.append("图像")
            if status.imu_publishers == 0:
                missing.append("串口 IMU")
            if status.camera_info_publishers == 0:
                missing.append("CameraInfo")
            vision_ok = self._image_stream_ready(now) and self._camera_info_ready(now)
            if vision_ok and missing == ["串口 IMU"]:
                self.tip.set_state(
                    "视觉原始画面已稳定显示；正式采集仍缺 Cube 串口 IMU，不会使用 Orbbec 相机内置 IMU。",
                    "bad",
                )
            else:
                self.tip.set_state(
                    "直连输入未就绪：" + "、".join(missing) + "。请检查相机、串口和权限。",
                    "bad",
                )
        elif not self._image_stream_ready(now) or not self._imu_stream_ready(now) or not self._camera_info_ready(now):
            self.tip.set_state(
                "直连输入已启动，但图像、串口 IMU 或 CameraInfo 还未达到稳定阈值；等待频率变绿后再开始录制。",
                "warn",
            )
        elif status.recording:
            self.tip.set_state(self.current_motion_hint, "ok")
        elif self.pending_auto_record:
            missing = self._readiness_missing_reasons()
            if missing:
                wait_text = "一键采集已触发，正在等待：" + "、".join(missing)
            else:
                wait_text = "一键采集已触发，关键数据已 ready，正在发送开始录制请求。"
            self.tip.set_state(wait_text, "warn")
        else:
            self.tip.set_state("直连输入在线，原始视频稳定显示。可以点击“一键开始采集”。", "ok")

    def closeEvent(self, event) -> None:  # noqa: N802 - Qt API name.
        self.timer.stop()
        self._stop_runtime_nodes()
        self.bridge.shutdown()
        event.accept()


def main() -> int:
    rclpy.init(args=sys.argv)
    print(
        "[calibration_gui] python="
        f"{sys.executable}, DISPLAY={os.environ.get('DISPLAY')}, "
        f"WAYLAND_DISPLAY={os.environ.get('WAYLAND_DISPLAY')}, "
        f"QT_QPA_PLATFORM={os.environ.get('QT_QPA_PLATFORM')}",
        flush=True,
    )
    if not os.environ.get("DISPLAY") and not os.environ.get("WAYLAND_DISPLAY"):
        print(
            "[calibration_gui] No DISPLAY/WAYLAND_DISPLAY found. "
            "Run on the desktop session, enable X11 forwarding, or use command-line services.",
            flush=True,
        )
    app = QApplication(remove_ros_args(args=sys.argv))
    bridge = RosBridge()
    window = CalibrationGui(bridge)
    window.show()
    exit_code = app.exec_()
    rclpy.shutdown()
    return exit_code


if __name__ == "__main__":
    raise SystemExit(main())
