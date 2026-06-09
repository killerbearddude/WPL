#ifndef WPL_WINDOW_H
#define WPL_WINDOW_H

#include <stdbool.h>

#include "wpl_result.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Opaque platform window handle. The concrete backend state is private. */
typedef struct WplWindow WplWindow;

typedef struct WplWindowDesc {
  const char* title;
  int width;
  int height;
  bool resizable;
} WplWindowDesc;

WplResult wpl_create_window(const WplWindowDesc* desc, WplWindow** out_window);
void wpl_destroy_window(WplWindow* window);
bool wpl_window_should_close(const WplWindow* window);
WplResult wpl_window_request_close(WplWindow* window);
WplResult wpl_begin_frame(WplWindow* window);
WplResult wpl_pump_events(WplWindow* window);
WplResult wpl_end_frame(WplWindow* window);
int wpl_window_width(const WplWindow* window);
int wpl_window_height(const WplWindow* window);

#ifdef __cplusplus
}
#endif

#endif /* WPL_WINDOW_H */
