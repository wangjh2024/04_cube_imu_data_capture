#!/usr/bin/env python3
"""Lightweight pose-IMU extrinsic calibration for the head-camera / hand-IMU setup.

Inputs:
  pose CSV: timestamp, tx, ty, tz, qx, qy, qz, qw for T_Cam_Tag or T_Cam_Cube
  IMU CSV: timestamp plus gyro and accelerometer samples

Output:
  T_Tag_IMU, mapping IMU-frame points into the AprilTag/Cube frame:
  p_tag = T_Tag_IMU * p_imu

This is a practical initializer/checker, not a full Kalibr replacement. Rotation is
estimated from visual angular velocity and gyro. Translation is estimated from
visual acceleration + accelerometer only when the motion excites it sufficiently.
"""

from __future__ import annotations

import argparse
import csv
import math
from dataclasses import dataclass
from pathlib import Path
from typing import Iterable

import numpy as np


@dataclass
class PoseSeries:
    t: np.ndarray
    p_cam_tag: np.ndarray
    r_cam_tag: np.ndarray


@dataclass
class ImuSeries:
    t: np.ndarray
    gyro: np.ndarray
    accel: np.ndarray | None


def normalize_name(name: str) -> str:
    return "".join(ch for ch in name.lower() if ch.isalnum())


def find_column(header: list[str], candidates: Iterable[str]) -> int | None:
    normalized = {normalize_name(name): idx for idx, name in enumerate(header)}
    for candidate in candidates:
        idx = normalized.get(normalize_name(candidate))
        if idx is not None:
            return idx
    return None


def read_csv_matrix(path: Path) -> tuple[list[str] | None, np.ndarray]:
    first_line = path.read_text(encoding="utf-8").splitlines()[0]
    has_header = any(ch.isalpha() for ch in first_line)
    with path.open("r", encoding="utf-8", newline="") as input_file:
        if has_header:
            reader = csv.reader(input_file)
            header = next(reader)
            rows = [[float(item) for item in row] for row in reader if row and not row[0].startswith("#")]
            return header, np.asarray(rows, dtype=float)
        rows = [
            [float(item) for item in row]
            for row in csv.reader(input_file)
            if row and not row[0].startswith("#")
        ]
        return None, np.asarray(rows, dtype=float)


def quaternion_xyzw_to_rotation(q_xyzw: np.ndarray) -> np.ndarray:
    q = np.asarray(q_xyzw, dtype=float)
    norm = float(np.linalg.norm(q))
    if norm < 1e-12:
        raise ValueError("zero quaternion in pose CSV")
    x, y, z, w = q / norm
    return np.array(
        [
            [1 - 2 * (y * y + z * z), 2 * (x * y - z * w), 2 * (x * z + y * w)],
            [2 * (x * y + z * w), 1 - 2 * (x * x + z * z), 2 * (y * z - x * w)],
            [2 * (x * z - y * w), 2 * (y * z + x * w), 1 - 2 * (x * x + y * y)],
        ],
        dtype=float,
    )


def rotation_to_quaternion_xyzw(rotation: np.ndarray) -> np.ndarray:
    trace = float(np.trace(rotation))
    if trace > 0.0:
        s = math.sqrt(trace + 1.0) * 2.0
        q = np.array(
            [
                (rotation[2, 1] - rotation[1, 2]) / s,
                (rotation[0, 2] - rotation[2, 0]) / s,
                (rotation[1, 0] - rotation[0, 1]) / s,
                0.25 * s,
            ]
        )
    else:
        idx = int(np.argmax(np.diag(rotation)))
        if idx == 0:
            s = math.sqrt(max(1.0 + rotation[0, 0] - rotation[1, 1] - rotation[2, 2], 1e-12)) * 2.0
            q = np.array([
                0.25 * s,
                (rotation[0, 1] + rotation[1, 0]) / s,
                (rotation[0, 2] + rotation[2, 0]) / s,
                (rotation[2, 1] - rotation[1, 2]) / s,
            ])
        elif idx == 1:
            s = math.sqrt(max(1.0 + rotation[1, 1] - rotation[0, 0] - rotation[2, 2], 1e-12)) * 2.0
            q = np.array([
                (rotation[0, 1] + rotation[1, 0]) / s,
                0.25 * s,
                (rotation[1, 2] + rotation[2, 1]) / s,
                (rotation[0, 2] - rotation[2, 0]) / s,
            ])
        else:
            s = math.sqrt(max(1.0 + rotation[2, 2] - rotation[0, 0] - rotation[1, 1], 1e-12)) * 2.0
            q = np.array([
                (rotation[0, 2] + rotation[2, 0]) / s,
                (rotation[1, 2] + rotation[2, 1]) / s,
                0.25 * s,
                (rotation[1, 0] - rotation[0, 1]) / s,
            ])
    return q / np.linalg.norm(q)


def log_so3(rotation: np.ndarray) -> np.ndarray:
    cos_angle = np.clip((float(np.trace(rotation)) - 1.0) * 0.5, -1.0, 1.0)
    angle = math.acos(cos_angle)
    vee = np.array(
        [
            rotation[2, 1] - rotation[1, 2],
            rotation[0, 2] - rotation[2, 0],
            rotation[1, 0] - rotation[0, 1],
        ],
        dtype=float,
    )
    if angle < 1e-9:
        return 0.5 * vee
    return angle / (2.0 * math.sin(angle)) * vee


def skew(v: np.ndarray) -> np.ndarray:
    return np.array(
        [[0.0, -v[2], v[1]], [v[2], 0.0, -v[0]], [-v[1], v[0], 0.0]],
        dtype=float,
    )


def load_pose_csv(path: Path) -> PoseSeries:
    header, data = read_csv_matrix(path)
    if data.ndim != 2 or data.shape[0] < 3:
        raise ValueError("pose CSV needs at least 3 rows")
    if header is None:
        if data.shape[1] < 8:
            raise ValueError("pose CSV without header must be: t,tx,ty,tz,qx,qy,qz,qw")
        cols = [0, 1, 2, 3, 4, 5, 6, 7]
    else:
        candidates = [
            ("timestamp", "time", "t", "stamp"),
            ("tx", "x", "position_x", "translation_x"),
            ("ty", "y", "position_y", "translation_y"),
            ("tz", "z", "position_z", "translation_z"),
            ("qx", "orientation_x", "quaternion_x"),
            ("qy", "orientation_y", "quaternion_y"),
            ("qz", "orientation_z", "quaternion_z"),
            ("qw", "orientation_w", "quaternion_w"),
        ]
        cols = []
        for names in candidates:
            idx = find_column(header, names)
            if idx is None:
                raise ValueError(f"pose CSV missing one of columns {names}")
            cols.append(idx)

    t = data[:, cols[0]]
    p = data[:, cols[1:4]]
    q = data[:, cols[4:8]]
    order = np.argsort(t)
    t = t[order]
    p = p[order]
    q = q[order]
    keep = np.concatenate([[True], np.diff(t) > 1e-9])
    r = np.stack([quaternion_xyzw_to_rotation(item) for item in q[keep]], axis=0)
    return PoseSeries(t=t[keep], p_cam_tag=p[keep], r_cam_tag=r)


def load_imu_csv(path: Path, order: str, gyro_unit: str, accel_unit: str) -> ImuSeries:
    header, data = read_csv_matrix(path)
    if data.ndim != 2 or data.shape[0] < 3:
        raise ValueError("IMU CSV needs at least 3 rows")
    if header is None:
        if data.shape[1] < 7:
            raise ValueError("IMU CSV without header must have 7 columns")
        if order == "gyro-first":
            cols = (0, [1, 2, 3], [4, 5, 6])
        else:
            cols = (0, [4, 5, 6], [1, 2, 3])
    else:
        t_col = find_column(header, ("timestamp", "time", "t", "stamp"))
        gyro_cols = [
            find_column(header, names)
            for names in (
                ("wx", "gx", "gyro_x", "gyroscope_x", "angular_velocity_x", "angular_velocity.x", "omega_x"),
                ("wy", "gy", "gyro_y", "gyroscope_y", "angular_velocity_y", "angular_velocity.y", "omega_y"),
                ("wz", "gz", "gyro_z", "gyroscope_z", "angular_velocity_z", "angular_velocity.z", "omega_z"),
            )
        ]
        accel_cols = [
            find_column(header, names)
            for names in (
                ("ax", "accel_x", "accelerometer_x", "linear_acceleration_x", "linear_acceleration.x"),
                ("ay", "accel_y", "accelerometer_y", "linear_acceleration_y", "linear_acceleration.y"),
                ("az", "accel_z", "accelerometer_z", "linear_acceleration_z", "linear_acceleration.z"),
            )
        ]
        if t_col is None or any(idx is None for idx in gyro_cols):
            raise ValueError("IMU CSV must contain timestamp and gyro xyz columns")
        cols = (t_col, gyro_cols, None if any(idx is None for idx in accel_cols) else accel_cols)

    t = data[:, cols[0]]
    gyro = data[:, cols[1]]
    accel = None if cols[2] is None else data[:, cols[2]]
    if gyro_unit == "deg/s":
        gyro = np.deg2rad(gyro)
    if accel is not None and accel_unit == "g":
        accel = accel * 9.80665
    order_idx = np.argsort(t)
    keep = np.concatenate([[True], np.diff(t[order_idx]) > 1e-9])
    order_idx = order_idx[keep]
    return ImuSeries(t=t[order_idx], gyro=gyro[order_idx], accel=None if accel is None else accel[order_idx])


def visual_angular_velocity(pose: PoseSeries) -> tuple[np.ndarray, np.ndarray]:
    mid_times = 0.5 * (pose.t[:-1] + pose.t[1:])
    omegas = []
    for idx in range(len(mid_times)):
        dt = float(pose.t[idx + 1] - pose.t[idx])
        delta_r = pose.r_cam_tag[idx].T @ pose.r_cam_tag[idx + 1]
        omegas.append(log_so3(delta_r) / max(dt, 1e-9))
    return mid_times, np.asarray(omegas, dtype=float)


def interp_vectors(query_t: np.ndarray, sample_t: np.ndarray, values: np.ndarray) -> np.ndarray:
    return np.stack([np.interp(query_t, sample_t, values[:, axis]) for axis in range(values.shape[1])], axis=1)


def kabsch_map(source: np.ndarray, target: np.ndarray) -> np.ndarray:
    # Return R such that target ~= R * source.
    h = target.T @ source
    u, _, vt = np.linalg.svd(h)
    correction = np.eye(3)
    correction[2, 2] = np.sign(np.linalg.det(u @ vt))
    return u @ correction @ vt


def solve_rotation(
    pose: PoseSeries,
    imu: ImuSeries,
    offset: float,
    min_omega: float,
    estimate_bias: bool,
) -> tuple[np.ndarray, np.ndarray, float, int]:
    visual_t, omega_tag = visual_angular_velocity(pose)
    imu_t = visual_t + offset
    valid = (imu_t >= imu.t[0]) & (imu_t <= imu.t[-1])
    omega_tag = omega_tag[valid]
    imu_t = imu_t[valid]
    gyro = interp_vectors(imu_t, imu.t, imu.gyro)
    excited = (np.linalg.norm(omega_tag, axis=1) >= min_omega) & (np.linalg.norm(gyro, axis=1) >= min_omega)
    omega_tag = omega_tag[excited]
    gyro = gyro[excited]
    if len(omega_tag) < 8:
        raise ValueError("not enough rotational excitation samples")

    if estimate_bias:
        centered_tag = omega_tag - np.mean(omega_tag, axis=0)
        centered_gyro = gyro - np.mean(gyro, axis=0)
        r_tag_imu = kabsch_map(centered_gyro, centered_tag)
        bias = np.mean(gyro - (r_tag_imu.T @ omega_tag.T).T, axis=0)
    else:
        r_tag_imu = kabsch_map(gyro, omega_tag)
        bias = np.zeros(3)

    residual = omega_tag - (r_tag_imu @ (gyro - bias).T).T
    rmse = float(np.sqrt(np.mean(np.sum(residual * residual, axis=1))))
    return r_tag_imu, bias, rmse, len(omega_tag)


def estimate_translation(
    pose: PoseSeries,
    imu: ImuSeries,
    r_tag_imu: np.ndarray,
    offset: float,
) -> tuple[np.ndarray, np.ndarray, float, bool, str]:
    if imu.accel is None:
        return np.zeros(3), np.array([0.0, 0.0, -9.80665]), math.nan, False, "IMU CSV has no accel columns"
    if len(pose.t) < 8:
        return np.zeros(3), np.array([0.0, 0.0, -9.80665]), math.nan, False, "pose CSV too short"

    velocity = np.gradient(pose.p_cam_tag, pose.t, axis=0, edge_order=2)
    accel_cam_tag = np.gradient(velocity, pose.t, axis=0, edge_order=2)
    visual_t, omega_mid = visual_angular_velocity(pose)
    omega = interp_vectors(pose.t, visual_t, omega_mid)
    alpha = np.gradient(omega, pose.t, axis=0, edge_order=2)
    imu_t = pose.t + offset
    valid = (imu_t >= imu.t[0]) & (imu_t <= imu.t[-1])
    valid[:2] = False
    valid[-2:] = False
    if np.count_nonzero(valid) < 10:
        return np.zeros(3), np.array([0.0, 0.0, -9.80665]), math.nan, False, "not enough overlap"

    accel_imu = interp_vectors(imu_t[valid], imu.t, imu.accel)
    a_rows = []
    b_rows = []
    for local_idx, idx in enumerate(np.flatnonzero(valid)):
        r_cam_tag = pose.r_cam_tag[idx]
        motion_matrix = skew(alpha[idx]) + skew(omega[idx]) @ skew(omega[idx])
        a_rows.append(np.hstack([r_cam_tag @ motion_matrix, -np.eye(3)]))
        b_rows.append(r_cam_tag @ r_tag_imu @ accel_imu[local_idx] - accel_cam_tag[idx])

    a = np.vstack(a_rows)
    b = np.concatenate(b_rows)
    excitation = float(np.linalg.cond(a.T @ a))
    if not np.isfinite(excitation) or excitation > 1e10:
        return np.zeros(3), np.array([0.0, 0.0, -9.80665]), math.nan, False, "translation poorly observable"

    solution, *_ = np.linalg.lstsq(a, b, rcond=None)
    residual = a @ solution - b
    rmse = float(np.sqrt(np.mean(residual * residual)))
    return solution[:3], solution[3:6], rmse, True, "estimated from accel + visual acceleration"


def invert_transform(transform: np.ndarray) -> np.ndarray:
    out = np.eye(4)
    out[:3, :3] = transform[:3, :3].T
    out[:3, 3] = -out[:3, :3] @ transform[:3, 3]
    return out


def matrix_rows(matrix: np.ndarray, indent: str = "  ") -> str:
    return "\n".join(indent + "[" + ", ".join(f"{v:.12g}" for v in row) + "]" for row in matrix)


def opencv_matrix(name: str, matrix: np.ndarray) -> str:
    data = ", ".join(f"{v:.12g}" for v in matrix.reshape(-1))
    return f"{name}: !!opencv-matrix\n  rows: 4\n  cols: 4\n  dt: d\n  data: [{data}]"


def build_report(
    t_tag_imu: np.ndarray,
    offset: float,
    gyro_bias: np.ndarray,
    gyro_rmse: float,
    gyro_samples: int,
    translation_status: str,
    translation_rmse: float,
    gravity_cam: np.ndarray,
) -> str:
    r = t_tag_imu[:3, :3]
    t = t_tag_imu[:3, 3]
    q = rotation_to_quaternion_xyzw(r)
    det = float(np.linalg.det(r))
    inverse = invert_transform(t_tag_imu)
    lines = [
        "# cube_imu_calibration pose-IMU result",
        "# Convention: T_A_B maps coordinates from frame B to frame A.",
        "# Result: T_Tag_IMU maps IMU frame points into AprilTag/Cube frame.",
        "# Method: visual angular velocity + IMU gyro; translation is lightweight accel LS initial value.",
        "",
        "T_Tag_IMU_matrix:",
        matrix_rows(t_tag_imu),
        "",
        "R_Tag_IMU:",
        matrix_rows(r),
        "",
        "t_Tag_IMU: [" + ", ".join(f"{v:.12g}" for v in t) + "]",
        "q_Tag_IMU_xyzw: [" + ", ".join(f"{v:.12g}" for v in q) + "]",
        "",
        f"time_offset_used_sec: {offset:.9g}",
        "gyro_bias_rad_s: [" + ", ".join(f"{v:.12g}" for v in gyro_bias) + "]",
        f"gyro_alignment_rmse_rad_s: {gyro_rmse:.12g}",
        f"gyro_alignment_samples: {gyro_samples}",
        f"det_R_Tag_IMU: {det:.12g}",
        f"translation_status: {translation_status}",
        f"translation_rmse_m_s2: {translation_rmse:.12g}",
        "estimated_gravity_camera_m_s2: [" + ", ".join(f"{v:.12g}" for v in gravity_cam) + "]",
        "",
        "# VINS-Fusion/OpenCV-YAML style copy block:",
        opencv_matrix("T_Tag_IMU", t_tag_imu),
        "",
        "# OpenVINS/common YAML style copy block:",
        "T_Tag_IMU:",
        matrix_rows(t_tag_imu),
        "",
        "# Inverse, for configs that expect T_IMU_Tag:",
        opencv_matrix("T_IMU_Tag", inverse),
        "",
        "T_IMU_Tag:",
        matrix_rows(inverse),
    ]
    return "\n".join(lines) + "\n"


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Estimate T_Tag_IMU from AprilTag/Cube pose CSV and IMU CSV."
    )
    parser.add_argument("--pose-csv", required=True, type=Path, help="CSV: timestamp,tx,ty,tz,qx,qy,qz,qw for T_Cam_Tag/Cube.")
    parser.add_argument("--imu-csv", required=True, type=Path, help="CSV containing timestamp, gyro xyz, and optional accel xyz.")
    parser.add_argument("--imu-order", choices=("gyro-first", "accel-first"), default="gyro-first", help="Column order when IMU CSV has no header.")
    parser.add_argument("--gyro-unit", choices=("rad/s", "deg/s"), default="rad/s")
    parser.add_argument("--accel-unit", choices=("m/s2", "g"), default="m/s2")
    parser.add_argument("--time-offset-min", type=float, default=0.0, help="Minimum offset added to pose timestamps for IMU lookup.")
    parser.add_argument("--time-offset-max", type=float, default=0.0, help="Maximum offset added to pose timestamps for IMU lookup.")
    parser.add_argument("--time-offset-step", type=float, default=0.002)
    parser.add_argument("--min-omega", type=float, default=0.05, help="Minimum angular speed in rad/s used for rotation alignment.")
    parser.add_argument("--no-estimate-gyro-bias", action="store_true")
    parser.add_argument("--translation-mode", choices=("estimate", "manual", "zero"), default="estimate")
    parser.add_argument("--manual-translation", nargs=3, type=float, metavar=("X", "Y", "Z"), default=None, help="Manual t_Tag_IMU in meters.")
    parser.add_argument("--output", type=Path, default=Path("tag_imu_extrinsics.txt"))
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    pose = load_pose_csv(args.pose_csv)
    imu = load_imu_csv(args.imu_csv, args.imu_order, args.gyro_unit, args.accel_unit)

    if args.time_offset_max < args.time_offset_min:
        raise ValueError("--time-offset-max must be >= --time-offset-min")
    if args.time_offset_max == args.time_offset_min:
        offsets = np.array([args.time_offset_min])
    else:
        step = max(args.time_offset_step, 1e-6)
        offsets = np.arange(args.time_offset_min, args.time_offset_max + 0.5 * step, step)

    best = None
    for offset in offsets:
        try:
            result = solve_rotation(
                pose,
                imu,
                float(offset),
                args.min_omega,
                estimate_bias=not args.no_estimate_gyro_bias,
            )
        except ValueError:
            continue
        if best is None or result[2] < best[3]:
            best = (float(offset), *result)
    if best is None:
        raise RuntimeError("failed to align visual angular velocity and IMU gyro; collect richer rotations")

    offset, r_tag_imu, gyro_bias, gyro_rmse, gyro_samples = best

    if args.translation_mode == "manual":
        if args.manual_translation is None:
            raise ValueError("--translation-mode manual requires --manual-translation X Y Z")
        t_tag_imu = np.asarray(args.manual_translation, dtype=float)
        gravity_cam = np.array([0.0, 0.0, -9.80665])
        translation_rmse = math.nan
        translation_status = "manual translation supplied"
    elif args.translation_mode == "zero":
        t_tag_imu = np.zeros(3)
        gravity_cam = np.array([0.0, 0.0, -9.80665])
        translation_rmse = math.nan
        translation_status = "zero translation supplied"
    else:
        t_tag_imu, gravity_cam, translation_rmse, ok, reason = estimate_translation(
            pose, imu, r_tag_imu, offset
        )
        translation_status = reason if ok else reason + "; using zero translation"

    transform = np.eye(4)
    transform[:3, :3] = r_tag_imu
    transform[:3, 3] = t_tag_imu
    report = build_report(
        transform,
        offset,
        gyro_bias,
        gyro_rmse,
        gyro_samples,
        translation_status,
        translation_rmse,
        gravity_cam,
    )
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(report, encoding="utf-8")
    print(report)
    print(f"Saved result to: {args.output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
