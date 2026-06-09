/* wpl_linux_x11_window.c - Linux/X11 window lifecycle implementation. */

#include "wpl_linux_x11_internal.h"

#include "wpl/wpl_base.h"
#include "wpl/wpl_time.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

static WplWindow* wpl_linux_x11_active_window;

static void
wpl_linux_x11_set_fixed_size_hints(WplWindow* window, int width, int height)
{
  XSizeHints* hints;

  if (window == NULL || window->display == NULL || window->window == 0)
    return;

  hints = XAllocSizeHints();
  if (hints == NULL)
    return;

  hints->flags = PMinSize | PMaxSize;
  hints->min_width = width;
  hints->max_width = width;
  hints->min_height = height;
  hints->max_height = height;

  XSetWMNormalHints(window->display, window->window, hints);
  XFree(hints);
}

static void
wpl_linux_x11_destroy_resources(WplWindow* window)
{
  if (window == NULL)
    return;

  if (window->ximage != NULL)
    {
      XDestroyImage(window->ximage);
      window->ximage = NULL;
      window->ximage_pixels = NULL;
    }

  free(window->framebuffer);
  window->framebuffer = NULL;
  window->framebuffer_width = 0;
  window->framebuffer_height = 0;

  if (window->display != NULL && window->gc != 0)
    {
      XFreeGC(window->display, window->gc);
      window->gc = 0;
    }

  if (window->display != NULL && window->window != 0)
    {
      XDestroyWindow(window->display, window->window);
      window->window = 0;
    }

  if (window->display != NULL)
    {
      XCloseDisplay(window->display);
      window->display = NULL;
    }
}

static WplResult
wpl_linux_x11_validate_window_desc(const WplWindowDesc* desc,
                                  WplWindow** out_window)
{
  if (out_window == NULL)
    return WPL_RESULT_INVALID_ARGUMENT;

  *out_window = NULL;

  if (desc == NULL || desc->title == NULL)
    return WPL_RESULT_INVALID_ARGUMENT;

  if (desc->width <= 0 || desc->height <= 0)
    return WPL_RESULT_INVALID_ARGUMENT;

  if (desc->width > WPL_MAX_WINDOW_DIMENSION
      || desc->height > WPL_MAX_WINDOW_DIMENSION)
    return WPL_RESULT_UNSUPPORTED;

  if (wpl_linux_x11_active_window != NULL)
    return WPL_RESULT_UNSUPPORTED;

  return WPL_RESULT_OK;
}

WplResult
wpl_create_window(const WplWindowDesc* desc, WplWindow** out_window)
{
  const long event_mask = (ExposureMask | StructureNotifyMask | KeyPressMask
                           | KeyReleaseMask | ButtonPressMask
                           | ButtonReleaseMask | PointerMotionMask
                           | FocusChangeMask | EnterWindowMask
                           | LeaveWindowMask);
  WplResult validation;
  WplWindow* window;
  int screen;
  unsigned long black_pixel;
  unsigned long white_pixel;

  validation = wpl_linux_x11_validate_window_desc(desc, out_window);
  if (validation != WPL_RESULT_OK)
    return validation;

  window = (WplWindow*)calloc(1u, sizeof(*window));
  if (window == NULL)
    return WPL_RESULT_OUT_OF_MEMORY;

  window->display = XOpenDisplay(NULL);
  if (window->display == NULL)
    {
      free(window);
      return WPL_RESULT_PLATFORM_ERROR;
    }

  screen = DefaultScreen(window->display);
  black_pixel = BlackPixel(window->display, screen);
  white_pixel = WhitePixel(window->display, screen);

  window->window = XCreateSimpleWindow(window->display,
                                       RootWindow(window->display, screen),
                                       0,
                                       0,
                                       (unsigned int)desc->width,
                                       (unsigned int)desc->height,
                                       0,
                                       black_pixel,
                                       white_pixel);
  if (window->window == 0)
    {
      wpl_linux_x11_destroy_resources(window);
      free(window);
      return WPL_RESULT_PLATFORM_ERROR;
    }

  window->gc = XCreateGC(window->display, window->window, 0, NULL);
  if (window->gc == 0)
    {
      wpl_linux_x11_destroy_resources(window);
      free(window);
      return WPL_RESULT_PLATFORM_ERROR;
    }

  if (XStoreName(window->display, window->window, desc->title) == 0)
    {
      wpl_linux_x11_destroy_resources(window);
      free(window);
      return WPL_RESULT_PLATFORM_ERROR;
    }

  window->wm_delete_window = XInternAtom(window->display,
                                         "WM_DELETE_WINDOW",
                                         False);
  if (window->wm_delete_window == None)
    {
      wpl_linux_x11_destroy_resources(window);
      free(window);
      return WPL_RESULT_PLATFORM_ERROR;
    }

  if (XSetWMProtocols(window->display,
                      window->window,
                      &window->wm_delete_window,
                      1) == 0)
    {
      wpl_linux_x11_destroy_resources(window);
      free(window);
      return WPL_RESULT_PLATFORM_ERROR;
    }

  XSelectInput(window->display, window->window, event_mask);

  if (!desc->resizable)
    wpl_linux_x11_set_fixed_size_hints(window, desc->width, desc->height);

  window->width = desc->width;
  window->height = desc->height;
  window->should_close = false;
  window->has_focus = false;
  window->last_time = wpl_time_seconds();
  window->delta_time = 0.0f;
  window->framebuffer = NULL;
  window->framebuffer_width = 0;
  window->framebuffer_height = 0;
  window->ximage = NULL;
  window->ximage_pixels = NULL;
  window->xkb_detectable_auto_repeat_enabled = false;

  wpl_linux_x11_init_detectable_auto_repeat(window);

  XMapWindow(window->display, window->window);
  XFlush(window->display);

  wpl_linux_x11_active_window = window;
  *out_window = window;
  return WPL_RESULT_OK;
}

void
wpl_destroy_window(WplWindow* window)
{
  if (window == NULL)
    return;

  if (wpl_linux_x11_active_window == window)
    wpl_linux_x11_active_window = NULL;

  wpl_linux_x11_destroy_resources(window);
  memset(window, 0, sizeof(*window));
  free(window);
}

bool
wpl_window_should_close(const WplWindow* window)
{
  if (window == NULL)
    return true;

  return window->should_close;
}

WplResult
wpl_window_request_close(WplWindow* window)
{
  if (window == NULL)
    return WPL_RESULT_INVALID_ARGUMENT;

  window->should_close = true;
  return WPL_RESULT_OK;
}

WplResult
wpl_begin_frame(WplWindow* window)
{
  double now;

  if (window == NULL)
    return WPL_RESULT_INVALID_ARGUMENT;

  wpl_linux_x11_reset_transient_input(window);

  now = wpl_time_seconds();
  if (window->last_time > 0.0 && now >= window->last_time)
    window->delta_time = (float)(now - window->last_time);
  else
    window->delta_time = 0.0f;

  window->last_time = now;
  return WPL_RESULT_OK;
}

WplResult
wpl_pump_events(WplWindow* window)
{
  XEvent event;

  if (window == NULL)
    return WPL_RESULT_INVALID_ARGUMENT;

  while (XPending(window->display) > 0)
    {
      XNextEvent(window->display, &event);

      switch (event.type)
        {
        case ClientMessage:
          if (event.xclient.window == window->window
              && (Atom)event.xclient.data.l[0] == window->wm_delete_window)
            window->should_close = true;
          break;

        case ConfigureNotify:
          if (event.xconfigure.width > 0 && event.xconfigure.height > 0)
            {
              window->width = event.xconfigure.width;
              window->height = event.xconfigure.height;
            }
          else
            {
              window->width = 0;
              window->height = 0;
            }
          break;

        case Expose:
          break;

        case MotionNotify:
          wpl_linux_x11_handle_motion(window, &event.xmotion);
          break;

        case ButtonPress:
          wpl_linux_x11_handle_button_press(window, &event.xbutton);
          break;

        case ButtonRelease:
          wpl_linux_x11_handle_button_release(window, &event.xbutton);
          break;

        case KeyPress:
          wpl_linux_x11_handle_key_press(window, &event.xkey);
          break;

        case KeyRelease:
          wpl_linux_x11_handle_key_release(window, &event.xkey);
          break;

        case FocusIn:
          window->has_focus = true;
          break;

        case FocusOut:
          window->has_focus = false;
          wpl_linux_x11_clear_input_down_state(window);
          break;

        case EnterNotify:
          break;

        case LeaveNotify:
          break;

        default:
          break;
        }
    }

  return WPL_RESULT_OK;
}

WplResult
wpl_end_frame(WplWindow* window)
{
  if (window == NULL)
    return WPL_RESULT_INVALID_ARGUMENT;

  return wpl_linux_x11_present_frame(window);
}

int
wpl_window_width(const WplWindow* window)
{
  if (window == NULL)
    return 0;

  return window->width;
}

int
wpl_window_height(const WplWindow* window)
{
  if (window == NULL)
    return 0;

  return window->height;
}

float
wpl_window_delta_time(const WplWindow* window)
{
  if (window == NULL)
    return 0.0f;

  return window->delta_time;
}
