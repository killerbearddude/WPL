#ifndef WPL_DEBUG_H
#define WPL_DEBUG_H

#include <stddef.h>

#include "wpl_draw.h"
#include "wpl_input.h"
#include "wpl_result.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct WplDebugStats {
  float fps;
  float frame_ms;
  int window_width;
  int window_height;
  size_t draw_command_count;
  const char* backend_name;
} WplDebugStats;

typedef struct WplDebugLine {
  const char* label;
  const char* value;
} WplDebugLine;

WplResult wpl_debug_draw_overlay(WplDrawList* list,
                                 const WplDebugStats* stats,
                                 const WplInputState* input);
WplResult wpl_debug_draw_overlay_ex(WplDrawList* list,
                                    const WplDebugStats* stats,
                                    const WplInputState* input,
                                    const WplDebugLine* lines,
                                    size_t line_count);

#ifdef __cplusplus
}
#endif

#endif /* WPL_DEBUG_H */
