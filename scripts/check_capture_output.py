#!/usr/bin/env python3
"""Validate cube_imu_direct_dataset_v1 output and optionally write a manifest."""

from __future__ import annotations

import argparse
import csv
import json
from datetime import datetime, timezone
from pathlib import Path
from typing import Any

import yaml


PROJECT_ROOT = Path(__file__).resolve().parents[1]

REQUIRED_FILES = [
    "metadata.yaml",
    "image_timestamps.csv",
    "imu.csv",
    "tag_pose.csv",
    "camera_info.yaml",
]

CSV_HEADERS = {
    "image_timestamps.csv": ["timestamp", "filename", "width", "height", "encoding", "seq"],
    "imu.csv": ["timestamp", "ax", "ay", "az", "gx", "gy", "gz", "index"],
    "tag_pose.csv": [
        "timestamp",
        "tx",
        "ty",
        "tz",
        "qx",
        "qy",
        "qz",
        "qw",
        "tag_count",
        "reproj_error",
        "tag_ids",
    ],
}


def now_iso() -> str:
    return datetime.now(timezone.utc).isoformat(timespec="seconds")


def read_thresholds() -> dict[str, float]:
    config_path = PROJECT_ROOT / "configs" / "capture.yaml"
    if not config_path.exists():
        return {}
    data = yaml.safe_load(config_path.read_text(encoding="utf-8")) or {}
    quality = data.get("quality", {})
    return {key: float(value) for key, value in quality.items() if isinstance(value, (int, float))}


def csv_stats(path: Path) -> dict[str, Any]:
    rows = 0
    first_t = None
    last_t = None
    monotonic = True
    last_seen = None
    with path.open("r", encoding="utf-8", newline="") as handle:
        reader = csv.DictReader(handle)
        for row in reader:
            rows += 1
            timestamp = float(row["timestamp"])
            if first_t is None:
                first_t = timestamp
            if last_seen is not None and timestamp < last_seen:
                monotonic = False
            last_seen = timestamp
            last_t = timestamp
    duration = 0.0 if first_t is None or last_t is None else max(last_t - first_t, 0.0)
    rate = 0.0 if rows < 2 or duration <= 0.0 else (rows - 1) / duration
    return {
        "rows": rows,
        "first_timestamp": first_t,
        "last_timestamp": last_t,
        "duration_sec": duration,
        "rate_hz": rate,
        "monotonic": monotonic,
    }


def check_dataset(dataset: Path, allow_missing: bool) -> tuple[bool, dict[str, Any]]:
    report: dict[str, Any] = {
        "schema_version": 1,
        "dataset": str(dataset),
        "checked_at": now_iso(),
        "passed": True,
        "checks": [],
        "stats": {},
    }
    if not dataset.exists():
        message = f"dataset missing: {dataset}"
        report["checks"].append({"name": "dataset_exists", "passed": allow_missing, "message": message})
        report["passed"] = bool(allow_missing)
        return bool(allow_missing), report

    for name in REQUIRED_FILES:
        path = dataset / name
        passed = path.exists()
        report["checks"].append({"name": f"{name} exists", "passed": passed})
        report["passed"] = report["passed"] and passed

    metadata_path = dataset / "metadata.yaml"
    if metadata_path.exists():
        metadata = yaml.safe_load(metadata_path.read_text(encoding="utf-8")) or {}
        passed = metadata.get("format") == "cube_imu_direct_dataset_v1"
        report["checks"].append({"name": "metadata format", "passed": passed})
        report["passed"] = report["passed"] and passed

    for filename, expected_header in CSV_HEADERS.items():
        path = dataset / filename
        if not path.exists():
            continue
        with path.open("r", encoding="utf-8", newline="") as handle:
            actual_header = next(csv.reader(handle), [])
        passed = actual_header == expected_header
        report["checks"].append({"name": f"{filename} header", "passed": passed})
        report["passed"] = report["passed"] and passed

        stats = csv_stats(path)
        report["stats"][filename] = stats
        report["checks"].append({"name": f"{filename} rows", "passed": stats["rows"] > 0})
        report["checks"].append({"name": f"{filename} monotonic", "passed": stats["monotonic"]})
        report["passed"] = report["passed"] and stats["rows"] > 0 and stats["monotonic"]

    thresholds = read_thresholds()
    stats = report["stats"]
    metric_map = {
        "min_image_count": ("image_timestamps.csv", "rows", ">="),
        "min_imu_count": ("imu.csv", "rows", ">="),
        "min_pose_count": ("tag_pose.csv", "rows", ">="),
        "min_image_rate_hz": ("image_timestamps.csv", "rate_hz", ">="),
        "min_imu_rate_hz": ("imu.csv", "rate_hz", ">="),
        "min_pose_rate_hz": ("tag_pose.csv", "rate_hz", ">="),
    }
    for threshold_key, (filename, metric, operator) in metric_map.items():
        if threshold_key not in thresholds or filename not in stats:
            continue
        actual = float(stats[filename][metric])
        expected = thresholds[threshold_key]
        passed = actual >= expected
        report["checks"].append(
            {
                "name": threshold_key,
                "metric": f"{filename}.{metric}",
                "operator": operator,
                "expected": expected,
                "actual": actual,
                "passed": passed,
            }
        )
        report["passed"] = report["passed"] and passed

    if "image_timestamps.csv" in stats and "tag_pose.csv" in stats:
        image_rows = max(int(stats["image_timestamps.csv"]["rows"]), 1)
        pose_rows = int(stats["tag_pose.csv"]["rows"])
        detection_rate = pose_rows / image_rows
        report["stats"]["tag_detection_rate"] = detection_rate
        expected = thresholds.get("min_tag_detection_rate")
        if expected is not None:
            passed = detection_rate >= expected
            report["checks"].append(
                {
                    "name": "min_tag_detection_rate",
                    "operator": ">=",
                    "expected": expected,
                    "actual": detection_rate,
                    "passed": passed,
                }
            )
            report["passed"] = report["passed"] and passed

    return bool(report["passed"]), report


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--dataset", type=Path, default=Path("output_bag"))
    parser.add_argument("--allow-missing", action="store_true")
    parser.add_argument("--write-manifest", action="store_true")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    dataset = args.dataset if args.dataset.is_absolute() else PROJECT_ROOT / args.dataset
    passed, report = check_dataset(dataset, args.allow_missing)

    for check in report["checks"]:
        label = "OK" if check["passed"] else "FAIL"
        message = check.get("message", check["name"])
        print(f"[{label}] {message}")

    if args.write_manifest and dataset.exists():
        manifest_path = dataset / "capture_manifest.json"
        manifest_path.write_text(json.dumps(report, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
        print(f"Manifest: {manifest_path}")

    print("PASSED" if passed else "FAILED")
    return 0 if passed else 1


if __name__ == "__main__":
    raise SystemExit(main())
