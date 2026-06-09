/* wpl_canvas.c - canvas coordinate helper API stubs. */

#include "wpl/wpl_canvas.h"

WplResult
wpl_screen_to_canvas(const WplCanvasView* view,
                     WplVec2 screen,
                     WplVec2* out_canvas)
{
  (void)view;
  (void)screen;

  if (out_canvas == NULL)
    return WPL_RESULT_INVALID_ARGUMENT;

  out_canvas->x = 0.0f;
  out_canvas->y = 0.0f;
  return WPL_RESULT_UNSUPPORTED;
}

WplResult
wpl_canvas_to_screen(const WplCanvasView* view,
                     WplVec2 canvas,
                     WplVec2* out_screen)
{
  (void)view;
  (void)canvas;

  if (out_screen == NULL)
    return WPL_RESULT_INVALID_ARGUMENT;

  out_screen->x = 0.0f;
  out_screen->y = 0.0f;
  return WPL_RESULT_UNSUPPORTED;
}

WplResult
wpl_canvas_pan_by(WplCanvasView* view, WplVec2 screen_delta)
{
  (void)screen_delta;

  if (view == NULL)
    return WPL_RESULT_INVALID_ARGUMENT;

  return WPL_RESULT_UNSUPPORTED;
}

WplResult
wpl_canvas_zoom_around(WplCanvasView* view,
                       WplVec2 cursor_screen,
                       float zoom_factor)
{
  (void)cursor_screen;
  (void)zoom_factor;

  if (view == NULL)
    return WPL_RESULT_INVALID_ARGUMENT;

  return WPL_RESULT_UNSUPPORTED;
}
