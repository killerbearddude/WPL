/* wpl_linux_x11_window.c - Linux/X11 window lifecycle API stubs. */

#include "wpl_linux_x11_internal.h"

#include <stddef.h>

WplResult
wpl_create_window(const WplWindowDesc* desc, WplWindow** out_window)
{
  (void)desc;

  if (out_window == NULL)
    return WPL_RESULT_INVALID_ARGUMENT;

  *out_window = NULL;
  return WPL_RESULT_UNSUPPORTED;
}

void
wpl_destroy_window(WplWindow* window)
{
  (void)window;
}

bool
wpl_window_should_close(const WplWindow* window)
{
  return window == NULL ? true : false;
}

WplResult
wpl_window_request_close(WplWindow* window)
{
  if (window == NULL)
    return WPL_RESULT_INVALID_ARGUMENT;

  return WPL_RESULT_UNSUPPORTED;
}

WplResult
wpl_begin_frame(WplWindow* window)
{
  if (window == NULL)
    return WPL_RESULT_INVALID_ARGUMENT;

  return WPL_RESULT_UNSUPPORTED;
}

WplResult
wpl_pump_events(WplWindow* window)
{
  if (window == NULL)
    return WPL_RESULT_INVALID_ARGUMENT;

  return WPL_RESULT_UNSUPPORTED;
}

WplResult
wpl_end_frame(WplWindow* window)
{
  if (window == NULL)
    return WPL_RESULT_INVALID_ARGUMENT;

  return WPL_RESULT_UNSUPPORTED;
}

int
wpl_window_width(const WplWindow* window)
{
  (void)window;
  return 0;
}

int
wpl_window_height(const WplWindow* window)
{
  (void)window;
  return 0;
}
