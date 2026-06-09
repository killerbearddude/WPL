/* wpl_debug.c - debug overlay API stubs. */

#include "wpl/wpl_debug.h"

WplResult
wpl_debug_overlay_set_stats(const WplDebugStats* stats)
{
  if (stats == NULL)
    return WPL_RESULT_INVALID_ARGUMENT;

  return WPL_RESULT_UNSUPPORTED;
}
