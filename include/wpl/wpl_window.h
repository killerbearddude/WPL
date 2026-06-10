#ifndef WPL_WINDOW_H
#define WPL_WINDOW_H

#include <stdbool.h>

#include "wpl_result.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Opaque platform window handle. The concrete backend state is private. */
typedef struct WplWindow WplWindow;

/* Basic window creation parameters for the Linux/X11 v0.1 backend. */
typedef struct WplWindowDesc {
  const char* title;
  int width;
  int height;
  bool resizable;
} WplWindowDesc;

typedef enum WplCursorShape {
  WPL_CURSOR_ARROW = 0,
  WPL_CURSOR_HAND,
  WPL_CURSOR_CROSSHAIR,
  WPL_CURSOR_MOVE,
  WPL_CURSOR_NOT_ALLOWED
} WplCursorShape;

/* Create one Linux/X11 window. v0.1 supports only one active window. */
WplResult wpl_create_window(const WplWindowDesc* desc, WplWindow** out_window);

/* Destroy a window and release backend-owned resources. Accepts NULL. */
void wpl_destroy_window(WplWindow* window);

/* Return true for NULL or for a window that has received/requested close. */
bool wpl_window_should_close(const WplWindow* window);

/* Mark the window as closing without sending a backend event. */
WplResult wpl_window_request_close(WplWindow* window);

/* Set the platform cursor shape for this window.
   The public API exposes only portable WPL cursor shapes; backend cursor
   handles and cursor font constants remain private. */
WplResult wpl_set_cursor_shape(WplWindow* window, WplCursorShape shape);

/* Begin a frame: compute delta time and reset transient input fields. */
WplResult wpl_begin_frame(WplWindow* window);

/* Pump all pending platform events into the current frame state. */
WplResult wpl_pump_events(WplWindow* window);

/* End a frame and present backend-owned framebuffer state when available. */
WplResult wpl_end_frame(WplWindow* window);

/* Return current window dimensions, or zero for NULL. */
int wpl_window_width(const WplWindow* window);
int wpl_window_height(const WplWindow* window);

#ifdef __cplusplus
}
#endif

#endif /* WPL_WINDOW_H */
