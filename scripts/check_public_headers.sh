#!/usr/bin/env sh
set -eu

repo_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
build_dir=${1:-build}

case "$build_dir" in
  /*) build_path=$build_dir ;;
  *) build_path=$repo_dir/$build_dir ;;
esac

if [ ! -d "$build_path" ]; then
  cmake -S "$repo_dir" -B "$build_path" -DCMAKE_BUILD_TYPE=Debug
fi

cmake --build "$build_path" --target wpl_test_public_headers --parallel

if [ -d "$build_path/CMakeFiles/wpl_test_public_headers_cpp.dir" ]; then
  cmake --build "$build_path" --target wpl_test_public_headers_cpp --parallel
fi

ctest --test-dir "$build_path" --output-on-failure -R public
