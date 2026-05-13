# 04 Cube IMU Data Capture

本仓库按“非 ROS 应用工程”管理。当前目标是把 Cube/IMU 数据采集与标定流程工程化、规范化、标准化：代码入口固定，依赖可复现，数据产物不混入源码，运行顺序可检查。

## 当前状态

当前目录中仍保留了旧 ROS/colcon 产物：

- `cube_imu_calibration/`
- `build/`
- `install/`
- `log/`

同时，`.venv` 里存在旧的非 ROS 命令入口 `imu-cube-qt` 和 `imu-cube-calib`，但它们指向已经不存在的旧路径 `/home/wangjh/code/05imu_cube_calib`。因此当前标准入口应先恢复为本仓库自己的 Python 包源码，再重新安装虚拟环境。

标准非 ROS 源码结构应包含：

```text
pyproject.toml
imu_cube_calib/
  __init__.py
  cli.py
  qt_ui.py
tests/
scripts/
docs/
```

## 标准运行顺序

先运行工程体检：

```bash
cd /home/wangjh/code/04_cube_imu_data_capture
bash scripts/doctor.sh
```

如果体检提示缺少 `pyproject.toml` 或 `imu_cube_calib/`，先把非 ROS 源码恢复到当前仓库根目录。

源码恢复后，按固定顺序重建环境并运行：

```bash
cd /home/wangjh/code/04_cube_imu_data_capture
rm -rf .venv
python3 -m venv .venv
source .venv/bin/activate
python -m pip install -U pip
python -m pip install -e ".[vision,qt,dev,mcap]"
bash scripts/doctor.sh
imu-cube-qt
```

命令行标定入口：

```bash
imu-cube-calib --help
```

## 工程规则

- 源码只放在 `imu_cube_calib/`、`scripts/`、`tests/`、`docs/` 等工程目录。
- 原始采集数据、bag、mcap、CSV、图片帧输出放在 `data/`、`outputs/`、`output_bag/`，默认不进 Git。
- `build/`、`install/`、`log/` 是生成产物，不作为标准源码维护。
- 每次改代码后，先跑 `bash scripts/doctor.sh`，再运行 GUI 或命令行。
- 提交前执行 `git status --short`，确认没有把缓存、构建目录、大体积数据误提交。

更多顺序流见 [docs/PROJECT_FLOW.md](docs/PROJECT_FLOW.md)。
