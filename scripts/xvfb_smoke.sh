#!/usr/bin/env sh
set -eu

repo_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
build_dir=${WPL_XVFB_BUILD_DIR:-"$repo_dir/build-xvfb"}

cmake -S "$repo_dir" -B "$build_dir" -DCMAKE_BUILD_TYPE=Debug
cmake --build "$build_dir" --parallel

if ! command -v xvfb-run >/dev/null 2>&1; then
  echo "xvfb-run not found; install xvfb to run the headless X11 smoke test" >&2
  exit 127
fi

xvfb-run -a ctest \
  --test-dir "$build_dir" \
  --output-on-failure \
  -R '^wpl_test_window_api$'
