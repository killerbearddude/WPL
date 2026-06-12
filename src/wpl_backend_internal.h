#ifndef WPL_BACKEND_INTERNAL_H
#define WPL_BACKEND_INTERNAL_H

/*
 * Private backend interface shared by Linux display backend implementations.
 *
 * This header is intentionally not installed as public API.  It describes the
 * backend services consumed by WPL public entry points without exposing X11,
 * Wayland, or other native handles in public headers.
 */

#include <stdbool.h>

#include "wpl/wpl_draw.h"
#include "wpl/wpl_result.h"
#include "wpl/wpl_window.h"

typedef struct WplBackendVTable {
  const char* name;

  WplResult (*create_window)(const WplWindowDesc* desc,
                             WplWindow** out_window);
  void (*destroy_window)(WplWindow* window);
  bool (*window_should_close)(const WplWindow* window);
  WplResult (*window_request_close)(WplWindow* window);
  WplResult (*set_cursor_shape)(WplWindow* window, WplCursorShape shape);

  WplResult (*begin_frame)(WplWindow* window);
  WplResult (*pump_events)(WplWindow* window);
  WplResult (*end_frame)(WplWindow* window);

  int (*window_width)(const WplWindow* window);
  int (*window_height)(const WplWindow* window);
  float (*window_delta_time)(const WplWindow* window);

  WplResult (*submit_draw_list)(WplWindow* window, const WplDrawList* list);
} WplBackendVTable;

#endif /* WPL_BACKEND_INTERNAL_H */
