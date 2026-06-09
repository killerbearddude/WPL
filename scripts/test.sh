#!/usr/bin/env sh
set -eu

repo_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)

if [ ! -d "$repo_dir/build" ]; then
  "$repo_dir/scripts/build.sh"
fi

ctest --test-dir "$repo_dir/build" --output-on-failure

"$repo_dir/scripts/check_no_backend_leaks.sh"
