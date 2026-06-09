/* wpl_linux_x11_renderer.c - Linux/X11 presentation placeholders. */

#include "wpl_linux_x11_internal.h"

#include "wpl/wpl_draw.h"

WplResult
wpl_linux_x11_present_frame(WplWindow* window)
{
  if (window == NULL)
    return WPL_RESULT_INVALID_ARGUMENT;

  if (window->width <= 0 || window->height <= 0)
    return WPL_RESULT_OK;

  if (window->display != NULL)
    XFlush(window->display);

  return WPL_RESULT_OK;
}

WplResult
wpl_submit_draw_list(WplWindow* window, const WplDrawList* list)
{
  if (window == NULL || list == NULL)
    return WPL_RESULT_INVALID_ARGUMENT;

  /* Draw command execution and XImage presentation are deferred. */
  return WPL_RESULT_UNSUPPORTED;
}
