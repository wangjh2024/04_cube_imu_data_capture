#!/usr/bin/env python3
"""Kalibr static-chain solver: T_Cube_IMU = inv(T_Cam_Cube) * T_Cam_IMU.

This only applies when T_Cam_IMU is already a valid camera-to-IMU extrinsic for
the same rigid body model, such as the Kalibr camera-IMU calibration result.

Convention used everywhere in this script:
T_A_B maps a point expressed in frame B into frame A, p_A = T_A_B * p_B.
"""

from __future__ import annotations

import argparse
import json
import math
import re
import sys
from pathlib import Path
from typing import Any, Iterable

import numpy as np

try:
    import yaml
except ImportError:  # pragma: no cover - handled at runtime for non-YAML inputs
    yaml = None


CAM_IMU_KEYS = (
    "T_Cam_IMU",
    "T_cam_imu",
    "T_camera_imu",
    "T_cam0_imu",
    "T_cam_imu0",
    "cam0.T_cam_imu",
    "cam0.T_cam_imu0",
)

CAM_CUBE_KEYS = (
    "T_Cam_Cube",
    "T_cam_cube",
    "T_camera_cube",
    "T_Cam_Tag",
    "T_cam_tag",
    "T",
    "transform",
)

ROTATION_KEYS = ("R", "rotation", "rotation_matrix", "rot", "R_Cube_IMU", "R_Cam_IMU")
TRANSLATION_KEYS = ("t", "translation", "position", "p", "t_Cube_IMU", "t_Cam_IMU")
QUATERNION_KEYS = (
    "q",
    "quat",
    "quaternion",
    "quaternion_xyzw",
    "orientation",
    "q_xyzw",
)


class TransformParseError(RuntimeError):
    pass


if yaml is not None:

    class OpenCvYamlLoader(yaml.SafeLoader):
        pass

    def _construct_unknown_yaml(loader: Any, tag_suffix: str, node: Any) -> Any:
        del tag_suffix
        if isinstance(node, yaml.MappingNode):
            return loader.construct_mapping(node)
        if isinstance(node, yaml.SequenceNode):
            return loader.construct_sequence(node)
        return loader.construct_scalar(node)

    OpenCvYamlLoader.add_multi_constructor("", _construct_unknown_yaml)


def _strip_opencv_yaml_header(text: str) -> str:
    lines = []
    for line in text.splitlines():
        if line.startswith("%YAML:"):
            continue
        lines.append(line)
    return "\n".join(lines)


def load_data(path: Path) -> Any:
    if not path.exists():
        raise TransformParseError(f"Input file not found: {path}")
    text = path.read_text(encoding="utf-8")
    stripped = text.strip()

    if not stripped:
        raise TransformParseError(f"{path} is empty")

    if stripped[0] in "[{":
        try:
            return json.loads(stripped)
        except json.JSONDecodeError:
            pass

    if yaml is not None:
        try:
            return yaml.load(_strip_opencv_yaml_header(text), Loader=OpenCvYamlLoader)
        except Exception:
            pass

    numbers = parse_numbers(stripped)
    if len(numbers) in (7, 12, 16):
        return numbers

    raise TransformParseError(
        f"Cannot parse {path}. Provide YAML/JSON, a flat 4x4 matrix, "
        "or quaternion+translation values."
    )


def parse_numbers(text: str) -> list[float]:
    pattern = r"[-+]?(?:\d+\.\d*|\.\d+|\d+)(?:[eE][-+]?\d+)?"
    return [float(item) for item in re.findall(pattern, text)]


def normalize_key(key: str) -> str:
    return key.lower().replace("-", "_")


def dict_get_case_insensitive(data: dict[str, Any], keys: Iterable[str]) -> Any | None:
    wanted = {normalize_key(key): key for key in keys}
    for key, value in data.items():
        if normalize_key(str(key)) in wanted:
            return value
    return None


def resolve_dotted_path(data: Any, path: str) -> Any | None:
    current = data
    for part in path.split("."):
        if not isinstance(current, dict):
            return None
        found = None
        for key, value in current.items():
            if normalize_key(str(key)) == normalize_key(part):
                found = value
                break
        if found is None:
            return None
        current = found
    return current


def recursive_find_key(data: Any, keys: Iterable[str]) -> Any | None:
    for key in keys:
        if "." in key:
            value = resolve_dotted_path(data, key)
            if value is not None:
                return value

    if isinstance(data, dict):
        value = dict_get_case_insensitive(data, keys)
        if value is not None:
            return value
        for child in data.values():
            found = recursive_find_key(child, keys)
            if found is not None:
                return found
    elif isinstance(data, list):
        for child in data:
            found = recursive_find_key(child, keys)
            if found is not None:
                return found
    return None


def vector_from_any(value: Any, expected_len: int, name: str) -> np.ndarray:
    if isinstance(value, dict):
        if expected_len == 3:
            keys = ("x", "y", "z")
        elif expected_len == 4:
            keys = ("x", "y", "z", "w")
        else:
            raise TransformParseError(f"Unsupported vector length for {name}: {expected_len}")
        try:
            return np.array([float(value[key]) for key in keys], dtype=float)
        except KeyError as exc:
            raise TransformParseError(f"{name} dict is missing key {exc}") from exc

    array = np.array(value, dtype=float).reshape(-1)
    if array.size != expected_len:
        raise TransformParseError(f"{name} must contain {expected_len} values, got {array.size}")
    return array


def quaternion_xyzw_to_rotation(q_xyzw: np.ndarray) -> np.ndarray:
    norm = float(np.linalg.norm(q_xyzw))
    if norm < 1e-12:
        raise TransformParseError("Quaternion norm is zero")
    x, y, z, w = q_xyzw / norm
    return np.array(
        [
            [1.0 - 2.0 * (y * y + z * z), 2.0 * (x * y - z * w), 2.0 * (x * z + y * w)],
            [2.0 * (x * y + z * w), 1.0 - 2.0 * (x * x + z * z), 2.0 * (y * z - x * w)],
            [2.0 * (x * z - y * w), 2.0 * (y * z + x * w), 1.0 - 2.0 * (x * x + y * y)],
        ],
        dtype=float,
    )


def rotation_to_quaternion_xyzw(rotation: np.ndarray) -> np.ndarray:
    trace = float(np.trace(rotation))
    if trace > 0.0:
        s = math.sqrt(trace + 1.0) * 2.0
        w = 0.25 * s
        x = (rotation[2, 1] - rotation[1, 2]) / s
        y = (rotation[0, 2] - rotation[2, 0]) / s
        z = (rotation[1, 0] - rotation[0, 1]) / s
    else:
        diag = np.diag(rotation)
        idx = int(np.argmax(diag))
        if idx == 0:
            s = math.sqrt(1.0 + rotation[0, 0] - rotation[1, 1] - rotation[2, 2]) * 2.0
            w = (rotation[2, 1] - rotation[1, 2]) / s
            x = 0.25 * s
            y = (rotation[0, 1] + rotation[1, 0]) / s
            z = (rotation[0, 2] + rotation[2, 0]) / s
        elif idx == 1:
            s = math.sqrt(1.0 + rotation[1, 1] - rotation[0, 0] - rotation[2, 2]) * 2.0
            w = (rotation[0, 2] - rotation[2, 0]) / s
            x = (rotation[0, 1] + rotation[1, 0]) / s
            y = 0.25 * s
            z = (rotation[1, 2] + rotation[2, 1]) / s
        else:
            s = math.sqrt(1.0 + rotation[2, 2] - rotation[0, 0] - rotation[1, 1]) * 2.0
            w = (rotation[1, 0] - rotation[0, 1]) / s
            x = (rotation[0, 2] + rotation[2, 0]) / s
            y = (rotation[1, 2] + rotation[2, 1]) / s
            z = 0.25 * s
    q = np.array([x, y, z, w], dtype=float)
    return q / np.linalg.norm(q)


def transform_from_rotation_translation(rotation: np.ndarray, translation: np.ndarray) -> np.ndarray:
    transform = np.eye(4, dtype=float)
    transform[:3, :3] = rotation
    transform[:3, 3] = translation
    return transform


def matrix_from_any(value: Any, label: str) -> np.ndarray:
    if value is None:
        raise TransformParseError(f"No transform found for {label}")

    if isinstance(value, str):
        value = parse_numbers(value)

    if isinstance(value, dict):
        if {"rows", "cols", "data"}.issubset({str(key) for key in value.keys()}):
            rows = int(value["rows"])
            cols = int(value["cols"])
            data = np.array(value["data"], dtype=float)
            if rows * cols != data.size:
                raise TransformParseError(f"{label} OpenCV matrix has inconsistent rows/cols/data")
            return matrix_from_any(data.reshape(rows, cols), label)

        transform_value = dict_get_case_insensitive(value, ("transform", "matrix", "T"))
        if transform_value is not None and transform_value is not value:
            try:
                return matrix_from_any(transform_value, label)
            except TransformParseError:
                pass

        rotation_value = dict_get_case_insensitive(value, ROTATION_KEYS)
        translation_value = dict_get_case_insensitive(value, TRANSLATION_KEYS)
        quaternion_value = dict_get_case_insensitive(value, QUATERNION_KEYS)

        if translation_value is not None and (rotation_value is not None or quaternion_value is not None):
            translation = vector_from_any(translation_value, 3, f"{label}.translation")
            if rotation_value is not None:
                rotation = np.array(rotation_value, dtype=float)
                if rotation.size != 9:
                    raise TransformParseError(f"{label}.rotation must contain 9 values")
                rotation = rotation.reshape(3, 3)
            else:
                quaternion = vector_from_any(quaternion_value, 4, f"{label}.quaternion")
                rotation = quaternion_xyzw_to_rotation(quaternion)
            return transform_from_rotation_translation(rotation, translation)

    array = np.array(value, dtype=float)
    if array.shape == (4, 4):
        return array
    if array.shape == (3, 4):
        transform = np.eye(4, dtype=float)
        transform[:3, :] = array
        return transform

    flat = array.reshape(-1)
    if flat.size == 16:
        return flat.reshape(4, 4)
    if flat.size == 12:
        transform = np.eye(4, dtype=float)
        transform[:3, :] = flat.reshape(3, 4)
        return transform
    if flat.size == 7:
        translation = flat[:3]
        quaternion = flat[3:]
        return transform_from_rotation_translation(quaternion_xyzw_to_rotation(quaternion), translation)

    raise TransformParseError(f"Cannot convert {label} to a 4x4 transform")


def extract_transform(data: Any, preferred_keys: Iterable[str], explicit_key: str | None, label: str) -> np.ndarray:
    if explicit_key:
        value = resolve_dotted_path(data, explicit_key)
        if value is None:
            raise TransformParseError(f"Key '{explicit_key}' not found for {label}")
        return matrix_from_any(value, label)

    direct_candidates = []
    if isinstance(data, dict):
        direct_candidates.append(data)
        found = recursive_find_key(data, preferred_keys)
        if found is not None:
            direct_candidates.insert(0, found)
    else:
        direct_candidates.append(data)

    last_error: Exception | None = None
    for candidate in direct_candidates:
        try:
            return matrix_from_any(candidate, label)
        except Exception as exc:
            last_error = exc

    raise TransformParseError(f"Failed to extract {label}: {last_error}")


def invert_transform(transform: np.ndarray) -> np.ndarray:
    rotation = transform[:3, :3]
    translation = transform[:3, 3]
    inverse = np.eye(4, dtype=float)
    inverse[:3, :3] = rotation.T
    inverse[:3, 3] = -rotation.T @ translation
    return inverse


def validate_transform(transform: np.ndarray) -> dict[str, float | bool]:
    rotation = transform[:3, :3]
    det = float(np.linalg.det(rotation))
    orthonormal_error = float(np.linalg.norm(rotation.T @ rotation - np.eye(3), ord="fro"))
    last_row_error = float(np.linalg.norm(transform[3, :] - np.array([0.0, 0.0, 0.0, 1.0])))
    valid = (
        abs(det - 1.0) < 1e-4
        and orthonormal_error < 1e-4
        and last_row_error < 1e-9
    )
    return {
        "determinant": det,
        "orthonormal_error": orthonormal_error,
        "last_row_error": last_row_error,
        "valid": valid,
    }


def format_matrix_rows(matrix: np.ndarray, indent: str = "") -> str:
    rows = []
    for row in matrix:
        rows.append(indent + "[" + ", ".join(f"{value:.12g}" for value in row) + "]")
    return "\n".join(rows)


def format_opencv_matrix(name: str, matrix: np.ndarray) -> str:
    flat = ", ".join(f"{value:.12g}" for value in matrix.reshape(-1))
    return (
        f"{name}: !!opencv-matrix\n"
        "  rows: 4\n"
        "  cols: 4\n"
        "  dt: d\n"
        f"  data: [{flat}]"
    )


def build_report(t_cube_imu: np.ndarray, include_inverse: bool) -> str:
    rotation = t_cube_imu[:3, :3]
    translation = t_cube_imu[:3, 3]
    quaternion = rotation_to_quaternion_xyzw(rotation)
    validation = validate_transform(t_cube_imu)

    lines = [
        "# cube_imu_calibration extrinsic result",
        "# Convention: T_A_B maps coordinates from frame B to frame A.",
        "# Result: T_Cube_IMU = inv(T_Cam_Cube) * T_Cam_IMU",
        "",
        "T_Cube_IMU_matrix:",
        format_matrix_rows(t_cube_imu, indent="  "),
        "",
        "R_Cube_IMU:",
        format_matrix_rows(rotation, indent="  "),
        "",
        "t_Cube_IMU: [" + ", ".join(f"{value:.12g}" for value in translation) + "]",
        "q_Cube_IMU_xyzw: [" + ", ".join(f"{value:.12g}" for value in quaternion) + "]",
        "",
        f"det_R_Cube_IMU: {validation['determinant']:.12g}",
        f"orthonormal_error: {validation['orthonormal_error']:.12g}",
        f"last_row_error: {validation['last_row_error']:.12g}",
        f"rigid_transform_valid: {str(validation['valid']).lower()}",
        "",
        "# VINS-Fusion/OpenCV-YAML style copy block:",
        format_opencv_matrix("T_Cube_IMU", t_cube_imu),
        "",
        "# OpenVINS/common YAML style copy block:",
        "T_Cube_IMU:",
        format_matrix_rows(t_cube_imu, indent="  "),
    ]

    if include_inverse:
        t_imu_cube = invert_transform(t_cube_imu)
        lines.extend(
            [
                "",
                "# Inverse, for configs that expect T_IMU_Cube instead:",
                format_opencv_matrix("T_IMU_Cube", t_imu_cube),
                "",
                "T_IMU_Cube:",
                format_matrix_rows(t_imu_cube, indent="  "),
            ]
        )

    return "\n".join(lines) + "\n"


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Compute Cube-to-IMU extrinsics from Kalibr T_Cam_IMU and AprilTag T_Cam_Cube."
    )
    parser.add_argument("--cam-imu", required=True, type=Path, help="File containing T_Cam_IMU.")
    parser.add_argument("--cam-cube", required=True, type=Path, help="File containing T_Cam_Cube.")
    parser.add_argument("--cam-imu-key", default=None, help="Optional dotted key for T_Cam_IMU.")
    parser.add_argument("--cam-cube-key", default=None, help="Optional dotted key for T_Cam_Cube.")
    parser.add_argument(
        "--output",
        type=Path,
        default=Path("cube_imu_extrinsics.txt"),
        help="Output txt file path.",
    )
    parser.add_argument(
        "--no-inverse",
        action="store_true",
        help="Do not include T_IMU_Cube in the output file.",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()

    try:
        cam_imu_data = load_data(args.cam_imu)
        cam_cube_data = load_data(args.cam_cube)

        t_cam_imu = extract_transform(cam_imu_data, CAM_IMU_KEYS, args.cam_imu_key, "T_Cam_IMU")
        t_cam_cube = extract_transform(cam_cube_data, CAM_CUBE_KEYS, args.cam_cube_key, "T_Cam_Cube")
    except TransformParseError as exc:
        print(f"solve_extrinsics.py: {exc}", file=sys.stderr)
        print(
            "提示：--cam-imu 应指向 Kalibr 输出的 camchain-imucam*.yaml；"
            "--cam-cube 应指向 GUI 保存的 cam_cube_pose.yaml。",
            file=sys.stderr,
        )
        return 2

    t_cube_imu = invert_transform(t_cam_cube) @ t_cam_imu
    report = build_report(t_cube_imu, include_inverse=not args.no_inverse)

    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(report, encoding="utf-8")
    print(report)
    print(f"Saved result to: {args.output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
