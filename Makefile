.PHONY: help check check-project check-env check-data data-strict capture-manifest build launch-gui launch-sim status

PYTHON ?= python3
DATASET ?= output_bag
PACKAGE ?= cube_imu_calibration

help:
	@printf "%s\n" "04 Cube IMU Data Capture"
	@printf "%s\n" ""
	@printf "%s\n" "Targets:"
	@printf "%s\n" "  make check            Run project/config checks and non-strict data check"
	@printf "%s\n" "  make check-project    Check ROS2 package, docs, configs, and generated dirs"
	@printf "%s\n" "  make check-env        Check ROS2/Python import environment"
	@printf "%s\n" "  make check-data       Validate DATASET=output_bag if present"
	@printf "%s\n" "  make data-strict      Validate DATASET=output_bag and fail if missing"
	@printf "%s\n" "  make capture-manifest Write DATASET/capture_manifest.json"
	@printf "%s\n" "  make build            colcon build cube_imu_calibration"
	@printf "%s\n" "  make launch-gui       Launch formal capture GUI"
	@printf "%s\n" "  make launch-sim       Launch simulation GUI"
	@printf "%s\n" "  make status           Show git status"

check: check-project check-data

check-project:
	$(PYTHON) scripts/check_project.py

check-env:
	$(PYTHON) scripts/check_project.py --check-env

check-data:
	$(PYTHON) scripts/check_capture_output.py --dataset $(DATASET) --allow-missing

data-strict:
	$(PYTHON) scripts/check_capture_output.py --dataset $(DATASET)

capture-manifest:
	$(PYTHON) scripts/check_capture_output.py --dataset $(DATASET) --write-manifest

build:
	colcon build --packages-select $(PACKAGE) --cmake-args -DCMAKE_BUILD_TYPE=Release

launch-gui:
	bash -lc 'source install/setup.bash && ros2 launch $(PACKAGE) gui_launch.py'

launch-sim:
	bash -lc 'source install/setup.bash && ros2 launch $(PACKAGE) sim_demo_launch.py'

status:
	git status --short --branch
