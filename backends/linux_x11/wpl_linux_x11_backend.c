/* wpl_linux_x11_backend.c - Linux/X11 private backend vtable. */

#include "wpl_linux_x11_internal.h"

static const WplBackendVTable wpl_linux_x11_vtable = {
  "linux_x11",
  wpl_create_window,
  wpl_destroy_window,
  wpl_window_should_close,
  wpl_window_request_close,
  wpl_set_cursor_shape,
  wpl_begin_frame,
  wpl_pump_events,
  wpl_end_frame,
  wpl_window_width,
  wpl_window_height,
  wpl_window_delta_time,
  wpl_submit_draw_list
};

const WplBackendVTable*
wpl_linux_x11_backend(void)
{
  return &wpl_linux_x11_vtable;
}
