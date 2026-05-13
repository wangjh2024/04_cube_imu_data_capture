# 工程顺序流规范

本文档定义本项目的标准工作流。核心原则：先确认项目形态，再确认入口，再建环境，再运行，再产出数据，再提交代码。

## 1. 项目形态确认

进入工程根目录：

```bash
cd /home/wangjh/code/04_cube_imu_data_capture
```

执行：

```bash
bash scripts/doctor.sh
```

判定规则：

- 有 `pyproject.toml` 和 `imu_cube_calib/`：按非 ROS Python 应用运行。
- 只有 `cube_imu_calibration/`、`build/`、`install/`：说明还停留在旧 ROS/colcon 残留状态。
- `.venv` 中入口指向其他目录：说明虚拟环境是从旧工程复制来的，必须重建。

## 2. 源码恢复

非 ROS 标准源码必须位于当前仓库根目录：

```text
imu_cube_calib/
pyproject.toml
```

不要依赖 `.venv` 里的 editable 旧路径。`.venv` 是可删除、可重建的环境，不是源码。

## 3. 环境重建

标准命令：

```bash
rm -rf .venv
python3 -m venv .venv
source .venv/bin/activate
python -m pip install -U pip
python -m pip install -e ".[vision,qt,dev,mcap]"
```

重建后检查入口：

```bash
which python
which imu-cube-qt
which imu-cube-calib
python -c "import imu_cube_calib; print(imu_cube_calib.__file__)"
```

这些路径都应该落在 `/home/wangjh/code/04_cube_imu_data_capture` 下。

## 4. 运行入口

GUI：

```bash
imu-cube-qt
```

命令行：

```bash
imu-cube-calib --help
```

如果入口报 `ModuleNotFoundError` 或路径指向旧目录，回到第 3 步重建环境。

## 5. 数据产出

标准产物目录：

```text
data/
outputs/
output_bag/
```

数据命名建议：

```text
data/session_YYYYMMDD_HHMMSS/
outputs/session_YYYYMMDD_HHMMSS/
```

原始数据、图像帧、bag、mcap、db3、临时 CSV 默认不提交 Git。

## 6. 提交前检查

提交前固定执行：

```bash
bash scripts/doctor.sh
git status --short
```

只提交：

- 源码
- 配置模板
- 文档
- 测试
- 小型示例数据

不提交：

- `.venv/`
- `build/`
- `install/`
- `log/`
- `output_bag/`
- 大体积采集数据

## 7. 标准故障处理

遇到运行问题按顺序排查：

1. 当前目录是否正确。
2. 源码是否存在。
3. 虚拟环境是否指向当前目录。
4. 依赖是否安装完成。
5. 输入数据路径是否存在。
6. 输出目录是否可写。

禁止先猜命令。先跑 `scripts/doctor.sh`，让状态说话。
