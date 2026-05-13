#!/usr/bin/env python3
"""Check inputs and print the Kalibr -> T_Cube_IMU command sequence."""

from __future__ import annotations

import argparse
from pathlib import Path

try:
    import yaml
except ImportError:  # pragma: no cover - reported at runtime
    yaml = None


def read_rosbag2_topics(bag_path: Path) -> list[str]:
    metadata_path = bag_path / "metadata.yaml" if bag_path.is_dir() else bag_path.parent / "metadata.yaml"
    if yaml is None or not metadata_path.exists():
        return []
    try:
        data = yaml.safe_load(metadata_path.read_text(encoding="utf-8")) or {}
    except Exception:
        return []
    topics = []
    for item in data.get("rosbag2_bagfile_information", {}).get("topics_with_message_count", []):
        metadata = item.get("topic_metadata", {})
        name = metadata.get("name")
        if name:
            topics.append(str(name))
    return sorted(set(topics))


def check_file(label: str, path: Path | None, required: bool = True) -> bool:
    if path is None:
        print(f"[{'MISS' if required else 'SKIP'}] {label}: 未提供")
        return not required
    if path.exists():
        print(f"[ OK ] {label}: {path}")
        return True
    print(f"[MISS] {label}: {path}")
    return False


def shell_quote(path: Path | str) -> str:
    text = str(path)
    if not text:
        return "''"
    if all(ch.isalnum() or ch in "/._:=+-" for ch in text):
        return text
    return "'" + text.replace("'", "'\"'\"'") + "'"


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Validate Kalibr workflow files and print next commands."
    )
    parser.add_argument("--bag", required=True, type=Path, help="rosbag2 directory or converted ROS1 bag.")
    parser.add_argument("--cam", required=True, type=Path, help="Kalibr camera YAML input.")
    parser.add_argument("--imu", required=True, type=Path, help="Kalibr IMU YAML input.")
    parser.add_argument("--target", required=True, type=Path, help="Kalibr calibration target YAML.")
    parser.add_argument("--cam-cube", type=Path, default=Path("cam_cube_pose.yaml"), help="GUI-saved T_Cam_Cube YAML.")
    parser.add_argument(
        "--kalibr-result",
        type=Path,
        default=Path("kalibr_result.yaml"),
        help="Kalibr output file containing T_Cam_IMU, e.g. camchain-imucam-*.yaml.",
    )
    parser.add_argument("--image-topic", default="/camera/image_raw")
    parser.add_argument("--imu-topic", default="/imu")
    parser.add_argument("--output", type=Path, default=Path("cube_imu_extrinsics.txt"))
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    bag = args.bag.expanduser()
    cam = args.cam.expanduser()
    imu = args.imu.expanduser()
    target = args.target.expanduser()
    cam_cube = args.cam_cube.expanduser()
    kalibr_result = args.kalibr_result.expanduser()

    print("== 输入文件检查 ==")
    ok = True
    ok &= check_file("bag", bag)
    ok &= check_file("camera yaml", cam)
    ok &= check_file("imu yaml", imu)
    ok &= check_file("target yaml", target)
    check_file("T_Cam_Cube yaml", cam_cube, required=False)
    check_file("Kalibr result yaml", kalibr_result, required=False)

    topics = read_rosbag2_topics(bag)
    if topics:
        print("\n== rosbag2 topics ==")
        for topic in topics:
            mark = "*" if topic in (args.image_topic, args.imu_topic) else " "
            print(f" {mark} {topic}")
        if args.image_topic not in topics or args.imu_topic not in topics:
            ok = False
            print(f"[MISS] bag 中未同时发现 {args.image_topic} 和 {args.imu_topic}")

    print("\n== 下一步命令 ==")
    print("1. 跑 Kalibr:")
    print(
        "kalibr_calibrate_imu_camera "
        f"--bag {shell_quote(bag)} "
        f"--cam {shell_quote(cam)} "
        f"--imu {shell_quote(imu)} "
        f"--target {shell_quote(target)} "
        f"--topics {args.image_topic} {args.imu_topic}"
    )
    print("\n2. GUI 保存 T_Cam_Cube 后，计算 T_Cube_IMU:")
    print(
        "ros2 run cube_imu_calibration solve_extrinsics.py "
        f"--cam-imu {shell_quote(kalibr_result)} "
        "--cam-imu-key cam0.T_cam_imu "
        f"--cam-cube {shell_quote(cam_cube)} "
        f"--output {shell_quote(args.output)}"
    )

    if not ok:
        print("\n检查未完全通过：先补齐缺失文件或修正 topic 名称。")
        return 2
    print("\n检查通过：可以按上面命令继续。")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
