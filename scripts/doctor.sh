#!/usr/bin/env bash
set -u

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

ok=0
warn=0

pass() {
  printf '[OK] %s\n' "$1"
}

fail() {
  printf '[FAIL] %s\n' "$1"
  ok=1
}

notice() {
  printf '[WARN] %s\n' "$1"
  warn=1
}

printf 'Project root: %s\n\n' "$ROOT"

if [ -f "pyproject.toml" ]; then
  pass "pyproject.toml exists"
else
  fail "pyproject.toml is missing; non-ROS Python package source is not restored yet"
fi

if [ -d "imu_cube_calib" ]; then
  pass "imu_cube_calib/ source package exists"
else
  fail "imu_cube_calib/ is missing; .venv entry points cannot be trusted"
fi

if [ -d ".venv" ]; then
  pass ".venv exists"
  if [ -x ".venv/bin/python" ]; then
    py="$(".venv/bin/python" -c 'import sys; print(sys.executable)' 2>/dev/null || true)"
    case "$py" in
      "$ROOT"/*) pass ".venv python points to this project" ;;
      "") notice ".venv python could not be queried" ;;
      *) fail ".venv python points outside this project: $py" ;;
    esac
  else
    fail ".venv/bin/python is missing or not executable"
  fi

  for exe in imu-cube-qt imu-cube-calib; do
    if [ -x ".venv/bin/$exe" ]; then
      first_line="$(sed -n '1p' ".venv/bin/$exe")"
      case "$first_line" in
        "#!"$ROOT" "*) pass "$exe shebang points to this project" ;;
        "#!"$ROOT/*) pass "$exe shebang points to this project" ;;
        "#!"*) fail "$exe shebang points outside this project: $first_line" ;;
        *) notice "$exe has no shebang" ;;
      esac
    else
      notice ".venv/bin/$exe does not exist yet"
    fi
  done
else
  notice ".venv does not exist; create it with python3 -m venv .venv"
fi

for d in build install log output_bag; do
  if [ -e "$d" ]; then
    notice "$d/ exists and should be treated as generated/runtime output"
  fi
done

if [ -d "cube_imu_calibration" ]; then
  notice "cube_imu_calibration/ exists; this is legacy ROS layout unless intentionally kept"
fi

if command -v git >/dev/null 2>&1 && git rev-parse --is-inside-work-tree >/dev/null 2>&1; then
  dirty_count="$(git status --short | wc -l)"
  if [ "$dirty_count" -eq 0 ]; then
    pass "git worktree is clean"
  else
    notice "git has $dirty_count changed/untracked path(s); review before commit"
  fi
fi

printf '\n'
if [ "$ok" -ne 0 ]; then
  printf 'Result: project is not runnable as standardized non-ROS package yet.\n'
  printf 'Next: restore pyproject.toml and imu_cube_calib/, then rebuild .venv.\n'
  exit 1
fi

if [ "$warn" -ne 0 ]; then
  printf 'Result: runnable checks passed, but warnings need review.\n'
else
  printf 'Result: project structure looks standardized.\n'
fi
