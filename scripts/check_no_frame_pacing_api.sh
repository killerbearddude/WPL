#!/usr/bin/env sh
set -eu

repo_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)

# Phase 3c boundary check: WPL currently has no public frame-pacing API.
# If one is intentionally introduced later, update docs/timing_frame_contract.md
# and this script in the same patch.
if grep -RInE '(^|[^A-Za-z0-9_])wpl_(sleep|yield|wait_events|wait_for_events|frame_limit|frame_limiter|limit_frame|set_target_fps|target_fps|set_frame_rate|set_vsync|vsync|present_interval)([^A-Za-z0-9_]|$)' \
  "$repo_dir/include/wpl"; then
  echo "unexpected public frame-pacing API surface detected" >&2
  exit 1
fi
