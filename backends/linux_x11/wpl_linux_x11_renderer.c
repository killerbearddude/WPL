/* wpl_linux_x11_renderer.c - Linux/X11 software presentation API stubs. */

#include "wpl/wpl_draw.h"

WplResult
wpl_submit_draw_list(WplWindow* window, const WplDrawList* list)
{
  if (window == NULL || list == NULL)
    return WPL_RESULT_INVALID_ARGUMENT;

  return WPL_RESULT_UNSUPPORTED;
}
