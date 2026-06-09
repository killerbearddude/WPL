#!/usr/bin/env sh
set -eu

cat <<'MSG'
WPL manual graphical smoke tests

These require a Linux X11/XWayland session.

Commands:

./build/examples/00_empty_window/wpl_empty_window
./build/examples/01_input_snapshot/wpl_input_snapshot
./build/examples/02_draw_primitives/wpl_draw_primitives
./build/examples/03_canvas_pan_zoom/wpl_canvas_pan_zoom
./build/examples/04_debug_overlay/wpl_debug_overlay
./build/examples/05_input_replay/wpl_input_replay record /tmp/wpl_input.replay
./build/examples/05_input_replay/wpl_input_replay playback /tmp/wpl_input.replay

Run with --run to execute them sequentially.
MSG

if [ "${1:-}" = "--run" ]; then
  ./build/examples/00_empty_window/wpl_empty_window
  ./build/examples/01_input_snapshot/wpl_input_snapshot
  ./build/examples/02_draw_primitives/wpl_draw_primitives
  ./build/examples/03_canvas_pan_zoom/wpl_canvas_pan_zoom
  ./build/examples/04_debug_overlay/wpl_debug_overlay
  ./build/examples/05_input_replay/wpl_input_replay record /tmp/wpl_input.replay
  ./build/examples/05_input_replay/wpl_input_replay playback /tmp/wpl_input.replay
fi
