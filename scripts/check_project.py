#!/usr/bin/env python3
"""Check ROS2 capture project structure, configs, and generated-output hygiene."""

from __future__ import annotations

import argparse
import importlib
import subprocess
from pathlib import Path
from typing import Any

import yaml


PROJECT_ROOT = Path(__file__).resolve().parents[1]

REQUIRED_PATHS = [
    "README.md",
    "Makefile",
    "configs/capture.yaml",
    "docs/标准采集流程.md",
    "docs/数据契约.md",
    "docs/多智能体作业规范.md",
    "docs/交付检查清单.md",
    "scripts/doctor.sh",
    "scripts/check_project.py",
    "scripts/check_capture_output.py",
    "cube_imu_calibration/CMakeLists.txt",
    "cube_imu_calibration/package.xml",
    "cube_imu_calibration/config/apriltag_params.yaml",
    "cube_imu_calibration/launch/gui_launch.py",
    "cube_imu_calibration/launch/runtime_launch.py",
    "cube_imu_calibration/launch/sim_demo_launch.py",
    "cube_imu_calibration/src/scripts/calibration_gui.py",
]

OPTIONAL_IMPORTS = ["cv2", "numpy", "PyQt5", "rclpy", "sensor_msgs", "yaml"]
GENERATED_DIRS = [
    "build",
    "install",
    "log",
    "Log",
    "logs",
    "output_bag",
    "data",
    "outputs",
    ".venv",
]
GENERATED_GLOBS = ["output_bag_*"]


class Checker:
    def __init__(self) -> None:
        self.errors: list[str] = []
        self.warnings: list[str] = []

    def ok(self, message: str) -> None:
        print(f"[OK] {message}")

    def warn(self, message: str) -> None:
        self.warnings.append(message)
        print(f"[WARN] {message}")

    def fail(self, message: str) -> None:
        self.errors.append(message)
        print(f"[FAIL] {message}")

    def require(self, condition: bool, message: str) -> None:
        if condition:
            self.ok(message)
        else:
            self.fail(message)


def load_yaml(path: Path) -> dict[str, Any]:
    data = yaml.safe_load(path.read_text(encoding="utf-8")) or {}
    if not isinstance(data, dict):
        raise ValueError(f"YAML root must be a mapping: {path}")
    return data


def check_required_paths(checker: Checker) -> None:
    print("\n[01] Project structure")
    for relative in REQUIRED_PATHS:
        checker.require((PROJECT_ROOT / relative).exists(), relative)


def check_capture_config(checker: Checker) -> None:
    print("\n[02] Capture config")
    config_path = PROJECT_ROOT / "configs" / "capture.yaml"
    if not config_path.exists():
        checker.fail("configs/capture.yaml")
        return
    config = load_yaml(config_path)
    checker.require(config.get("schema_version") == 1, "capture.yaml schema_version")
    checker.require(config.get("project", {}).get("type") == "ros2_capture", "project.type")
    checker.require(bool(config.get("paths", {}).get("ros_config")), "paths.ros_config")
    checker.require(bool(config.get("capture", {}).get("cube_layout")), "capture.cube_layout")
    checker.require(bool(config.get("quality", {})), "quality thresholds")


def check_ros_params(checker: Checker) -> None:
    print("\n[03] ROS params")
    path = PROJECT_ROOT / "cube_imu_calibration" / "config" / "apriltag_params.yaml"
    if not path.exists():
        checker.fail("apriltag_params.yaml")
        return
    data = load_yaml(path)
    gui = data.get("calibration_gui", {}).get("ros__parameters", {})
    recorder = data.get("data_recorder_node", {}).get("ros__parameters", {})
    real_sensor = data.get("real_sensor_node", {}).get("ros__parameters", {})

    checker.require(gui.get("enable_cube_detection") is True, "GUI cube detection enabled")
    checker.require(gui.get("cube_layout") == "left_hand_5face", "GUI left_hand_5face layout")
    checker.require(int(gui.get("target_tag_id", 0)) == -1, "GUI accepts cube IDs 0-4")
    checker.require(gui.get("imu_source") == "cube_serial", "GUI IMU source cube_serial")
    checker.require(gui.get("camera_output_encoding") == "mono8", "GUI mono8 image output")
    checker.require(float(gui.get("record_duration_sec", 0.0)) >= 120.0, "GUI duration >=120s")
    checker.require(float(gui.get("tag_detection_rate_hz", 0.0)) >= 8.0, "tag detection rate")
    checker.require(recorder.get("bag_path") == "output_bag", "recorder bag_path output_bag")
    checker.require(real_sensor.get("imu_source") == "cube_serial", "real sensor cube_serial")
    checker.require(float(real_sensor.get("serial_imu_rate_hz", 0.0)) >= 200.0, "serial IMU rate")


def check_generated_dirs(checker: Checker) -> None:
    print("\n[04] Generated dirs")
    for name in GENERATED_DIRS:
        path = PROJECT_ROOT / name
        if path.exists():
            checker.warn(f"{name}/ exists and is treated as generated/runtime output")
        else:
            checker.ok(f"{name}/ not present")
    for pattern in GENERATED_GLOBS:
        matches = sorted(PROJECT_ROOT.glob(pattern))
        if matches:
            checker.warn(
                f"{pattern} exists and is treated as generated/runtime output "
                f"({len(matches)} dirs)"
            )
        else:
            checker.ok(f"{pattern} not present")

    old_entry = PROJECT_ROOT / ".venv" / "bin" / "imu-cube-qt"
    if old_entry.exists():
        first_line = old_entry.read_text(encoding="utf-8", errors="ignore").splitlines()[:1]
        if first_line and str(PROJECT_ROOT) not in first_line[0]:
            checker.warn("legacy .venv entry point points outside this project")


def command_exists(name: str) -> bool:
    completed = subprocess.run(
        ["bash", "-lc", f"command -v {name}"],
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    return completed.returncode == 0


def check_environment(checker: Checker) -> None:
    print("\n[05] Environment")
    for command in ("ros2", "colcon"):
        checker.require(command_exists(command), f"command {command}")
    for module in OPTIONAL_IMPORTS:
        try:
            importlib.import_module(module)
        except Exception as exc:  # noqa: BLE001 - show exact dependency issue.
            checker.fail(f"import {module}: {exc}")
        else:
            checker.ok(f"import {module}")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check-env", action="store_true")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    checker = Checker()
    check_required_paths(checker)
    check_capture_config(checker)
    check_ros_params(checker)
    check_generated_dirs(checker)
    if args.check_env:
        check_environment(checker)

    if checker.errors:
        print(f"\nFAILED: {len(checker.errors)} issue(s)")
        return 1
    if checker.warnings:
        print(f"\nPASSED with {len(checker.warnings)} warning(s)")
        return 0
    print("\nPASSED")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
