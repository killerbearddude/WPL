#!/usr/bin/env sh
set -eu

repo_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
build_dir=${WPL_SANITIZE_BUILD_DIR:-"$repo_dir/build-sanitize"}
cc=${CC:-cc}

cmake -S "$repo_dir" -B "$build_dir" \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_C_COMPILER="$cc" \
  -DWPL_ENABLE_SANITIZERS=ON
cmake --build "$build_dir" --parallel
ctest --test-dir "$build_dir" --output-on-failure
