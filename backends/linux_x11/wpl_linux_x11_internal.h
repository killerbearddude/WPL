#ifndef WPL_LINUX_X11_INTERNAL_H
#define WPL_LINUX_X11_INTERNAL_H

/*
 * Linux/X11 backend-private boundary.
 *
 * Xlib types and concrete backend ownership live here, behind the public
 * WplWindow opaque handle. Public headers under include/wpl/ must never
 * include this file or expose these backend-specific types.
 */

#include <stdbool.h>
#include <stdint.h>

#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "wpl/wpl_input.h"
#include "wpl/wpl_result.h"
#include "wpl/wpl_window.h"

struct WplWindow {
  Display* display;
  Window window;
  GC gc;
  Atom wm_delete_window;

  int width;
  int height;

  bool should_close;
  bool has_focus;

  WplInputState input;

  double last_time;
  float delta_time;

  uint32_t* framebuffer;
  int framebuffer_width;
  int framebuffer_height;

  XImage* ximage;
  uint32_t* ximage_pixels;

  bool xkb_detectable_auto_repeat_enabled;
};

void wpl_linux_x11_reset_transient_input(WplInputState* input);
void wpl_linux_x11_clear_held_input(WplInputState* input);
WplResult wpl_linux_x11_present_frame(WplWindow* window);

#endif /* WPL_LINUX_X11_INTERNAL_H */
