#ifndef WPL_DEBUG_H
#define WPL_DEBUG_H

#include <stddef.h>

#include "wpl_result.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct WplDebugStats {
  float frame_time_ms;
  size_t draw_command_count;
  size_t draw_command_capacity;
} WplDebugStats;

/* Reserved hook for later debug overlay command generation. */
WplResult wpl_debug_overlay_set_stats(const WplDebugStats* stats);

#ifdef __cplusplus
}
#endif

#endif /* WPL_DEBUG_H */
