#!/usr/bin/env sh
set -eu

repo_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)

cmake -S "$repo_dir" -B "$repo_dir/build" -DCMAKE_BUILD_TYPE=Debug
cmake --build "$repo_dir/build" --target wpl_test_public_headers
ctest --test-dir "$repo_dir/build" --output-on-failure -R public_headers
