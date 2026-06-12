#ifndef WPL_LINUX_X11_INTERNAL_H
#define WPL_LINUX_X11_INTERNAL_H

/*
 * Linux/X11 backend-private boundary.
 *
 * Xlib and XKB types live here, behind the public WplWindow opaque handle.
 * Public headers under include/wpl/ must never include this file or expose
 * backend-specific handles, atoms, events, key symbols, or visuals.
 */

#include <stdbool.h>
#include <stdint.h>

#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/XKBlib.h>

#include "wpl/wpl_draw.h"
#include "wpl/wpl_input.h"
#include "wpl/wpl_result.h"
#include "wpl/wpl_window.h"

typedef struct WplBackendVTable WplBackendVTable;

typedef struct WplLinuxX11RenderClip {
  int x0;
  int y0;
  int x1;
  int y1;
} WplLinuxX11RenderClip;

struct WplWindow {
  Display* display;
  Window window;
  GC gc;
  Atom wm_delete_window;

  Cursor cursors[WPL_CURSOR_NOT_ALLOWED + 1];
  bool cursors_created[WPL_CURSOR_NOT_ALLOWED + 1];
  WplCursorShape current_cursor_shape;

  int width;
  int height;

  bool should_close;
  bool has_focus;

  WplInputState input;
  bool mouse_position_initialized;

  double last_time;
  float delta_time;

  uint32_t* framebuffer;
  int framebuffer_width;
  int framebuffer_height;

  XImage* ximage;
  uint32_t* ximage_pixels;

  WplLinuxX11RenderClip active_clip;
  WplLinuxX11RenderClip* clip_stack;
  size_t clip_stack_capacity;
  size_t clip_stack_depth;

  bool xkb_detectable_auto_repeat_enabled;
};

const WplBackendVTable* wpl_linux_x11_backend(void);

WplResult wpl_linux_x11_create_window(const WplWindowDesc* desc,
                                      WplWindow** out_window);
void wpl_linux_x11_destroy_window(WplWindow* window);
bool wpl_linux_x11_window_should_close(const WplWindow* window);
WplResult wpl_linux_x11_window_request_close(WplWindow* window);
WplResult wpl_linux_x11_set_cursor_shape(WplWindow* window,
                                         WplCursorShape shape);
WplResult wpl_linux_x11_begin_frame(WplWindow* window);
WplResult wpl_linux_x11_pump_events(WplWindow* window);
WplResult wpl_linux_x11_end_frame(WplWindow* window);
int wpl_linux_x11_window_width(const WplWindow* window);
int wpl_linux_x11_window_height(const WplWindow* window);
float wpl_linux_x11_window_delta_time(const WplWindow* window);
WplResult wpl_linux_x11_submit_draw_list(WplWindow* window,
                                         const WplDrawList* list);

void wpl_linux_x11_reset_transient_input(WplWindow* window);
void wpl_linux_x11_clear_input_down_state(WplWindow* window);
void wpl_linux_x11_init_detectable_auto_repeat(WplWindow* window);

void wpl_linux_x11_input_reset_transients(WplInputState* input);
void wpl_linux_x11_input_clear_down_state(WplInputState* input);
void wpl_linux_x11_input_press_mouse_button(WplInputState* input,
                                           WplMouseButton button);
void wpl_linux_x11_input_release_mouse_button(WplInputState* input,
                                             WplMouseButton button);
void wpl_linux_x11_input_press_key(WplInputState* input, WplKey key);
void wpl_linux_x11_input_release_key(WplInputState* input, WplKey key);

void wpl_linux_x11_handle_motion(WplWindow* window,
                                 const XMotionEvent* event);
void wpl_linux_x11_handle_enter(WplWindow* window,
                                const XCrossingEvent* event);
void wpl_linux_x11_handle_leave(WplWindow* window,
                                const XCrossingEvent* event);
void wpl_linux_x11_handle_button_press(WplWindow* window,
                                       const XButtonEvent* event);
void wpl_linux_x11_handle_button_release(WplWindow* window,
                                         const XButtonEvent* event);
void wpl_linux_x11_handle_key_press(WplWindow* window,
                                    XKeyEvent* event);
void wpl_linux_x11_handle_key_release(WplWindow* window,
                                      XKeyEvent* event);

void wpl_linux_x11_destroy_renderer_resources(WplWindow* window);
WplResult wpl_linux_x11_present_frame(WplWindow* window);

#if defined(WPL_LINUX_X11_RENAME_WINDOW_PUBLIC_SYMBOLS)
#define wpl_create_window wpl_linux_x11_create_window
#define wpl_destroy_window wpl_linux_x11_destroy_window
#define wpl_window_should_close wpl_linux_x11_window_should_close
#define wpl_window_request_close wpl_linux_x11_window_request_close
#define wpl_set_cursor_shape wpl_linux_x11_set_cursor_shape
#define wpl_begin_frame wpl_linux_x11_begin_frame
#define wpl_pump_events wpl_linux_x11_pump_events
#define wpl_end_frame wpl_linux_x11_end_frame
#define wpl_window_width wpl_linux_x11_window_width
#define wpl_window_height wpl_linux_x11_window_height
#define wpl_window_delta_time wpl_linux_x11_window_delta_time
#endif

#if defined(WPL_LINUX_X11_RENAME_RENDERER_PUBLIC_SYMBOLS)
#define wpl_submit_draw_list wpl_linux_x11_submit_draw_list
#endif

#endif /* WPL_LINUX_X11_INTERNAL_H */
