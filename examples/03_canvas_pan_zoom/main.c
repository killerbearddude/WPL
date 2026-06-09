/* main.c - Canvas math pan/zoom example using public WPL APIs only. */

#include <wpl/wpl.h>

#include <stdio.h>

static int
wpl_check(WplResult result)
{
  return result == WPL_RESULT_OK ? 0 : 1;
}

static WplResult
canvas_point_to_screen(const WplCanvasView* view,
                       float x,
                       float y,
                       WplVec2* out_screen)
{
  return wpl_canvas_to_screen(view, (WplVec2){x, y}, out_screen);
}

static WplResult
draw_canvas_line(WplDrawList* draw,
                 const WplCanvasView* view,
                 float ax,
                 float ay,
                 float bx,
                 float by,
                 WplColor color,
                 float thickness)
{
  WplVec2 a = {0};
  WplVec2 b = {0};
  WplResult result;

  result = canvas_point_to_screen(view, ax, ay, &a);
  if (result != WPL_RESULT_OK)
    return result;

  result = canvas_point_to_screen(view, bx, by, &b);
  if (result != WPL_RESULT_OK)
    return result;

  return wpl_draw_line(draw, a, b, color, thickness);
}

static WplResult
draw_canvas_rect(WplDrawList* draw,
                 const WplCanvasView* view,
                 WplRect canvas_rect,
                 WplColor color)
{
  WplVec2 a = {0};
  WplVec2 b = {0};
  WplResult result;
  WplRect screen_rect;

  result = canvas_point_to_screen(view,
                                  canvas_rect.x,
                                  canvas_rect.y,
                                  &a);
  if (result != WPL_RESULT_OK)
    return result;

  result = canvas_point_to_screen(view,
                                  canvas_rect.x + canvas_rect.w,
                                  canvas_rect.y + canvas_rect.h,
                                  &b);
  if (result != WPL_RESULT_OK)
    return result;

  screen_rect.x = a.x;
  screen_rect.y = a.y;
  screen_rect.w = b.x - a.x;
  screen_rect.h = b.y - a.y;
  return wpl_draw_rect(draw, screen_rect, color);
}

static WplResult
draw_canvas_grid(WplDrawList* draw, const WplCanvasView* view)
{
  WplResult result;
  int i;

  for (i = -20; i <= 20; i++)
    {
      WplColor major = {0.20f, 0.22f, 0.26f, 1.0f};
      WplColor axis = {0.50f, 0.55f, 0.65f, 1.0f};
      WplColor color = (i == 0) ? axis : major;
      float thickness = (i == 0) ? 2.0f : 1.0f;
      float p = (float)i * 50.0f;

      result = draw_canvas_line(draw,
                                view,
                                p,
                                -1000.0f,
                                p,
                                1000.0f,
                                color,
                                thickness);
      if (result != WPL_RESULT_OK)
        return result;

      result = draw_canvas_line(draw,
                                view,
                                -1000.0f,
                                p,
                                1000.0f,
                                p,
                                color,
                                thickness);
      if (result != WPL_RESULT_OK)
        return result;
    }

  return WPL_RESULT_OK;
}

static WplResult
draw_scene(WplDrawList* draw,
           const WplCanvasView* view,
           const WplInputState* input,
           WplVec2 cursor_canvas)
{
  char line[256];
  WplVec2 origin = {0};
  WplResult result;

  result = wpl_draw_clear(draw, (WplColor){0.04f, 0.045f, 0.055f, 1.0f});
  if (result != WPL_RESULT_OK)
    return result;

  result = draw_canvas_grid(draw, view);
  if (result != WPL_RESULT_OK)
    return result;

  result = canvas_point_to_screen(view, 0.0f, 0.0f, &origin);
  if (result != WPL_RESULT_OK)
    return result;

  result = wpl_draw_circle(draw,
                           origin,
                           6.0f,
                           (WplColor){1.0f, 0.80f, 0.20f, 1.0f});
  if (result != WPL_RESULT_OK)
    return result;

  result = draw_canvas_rect(draw,
                            view,
                            (WplRect){-220.0f, -120.0f, 160.0f, 90.0f},
                            (WplColor){0.20f, 0.35f, 0.85f, 0.80f});
  if (result != WPL_RESULT_OK)
    return result;

  result = draw_canvas_rect(draw,
                            view,
                            (WplRect){80.0f, 60.0f, 220.0f, 140.0f},
                            (WplColor){0.90f, 0.25f, 0.18f, 0.55f});
  if (result != WPL_RESULT_OK)
    return result;

  result = wpl_draw_rect(draw,
                         (WplRect){8.0f, 8.0f, 430.0f, 110.0f},
                         (WplColor){0.0f, 0.0f, 0.0f, 0.65f});
  if (result != WPL_RESULT_OK)
    return result;

  result = wpl_draw_text(draw,
                         (WplVec2){16.0f, 16.0f},
                         "WPL canvas pan/zoom\nLMB drag pans. Wheel zooms around cursor. Esc closes.",
                         (WplColor){0.95f, 0.96f, 0.98f, 1.0f});
  if (result != WPL_RESULT_OK)
    return result;

  (void)snprintf(line,
                 sizeof(line),
                 "screen: %.1f, %.1f  canvas: %.2f, %.2f",
                 input->mouse.position.x,
                 input->mouse.position.y,
                 cursor_canvas.x,
                 cursor_canvas.y);
  result = wpl_draw_text(draw,
                         (WplVec2){16.0f, 72.0f},
                         line,
                         (WplColor){0.75f, 0.82f, 0.90f, 1.0f});
  if (result != WPL_RESULT_OK)
    return result;

  (void)snprintf(line,
                 sizeof(line),
                 "pan: %.1f, %.1f  zoom: %.3f px/unit",
                 view->pan.x,
                 view->pan.y,
                 view->zoom);
  return wpl_draw_text(draw,
                       (WplVec2){16.0f, 92.0f},
                       line,
                       (WplColor){0.75f, 0.82f, 0.90f, 1.0f});
}

int
main(void)
{
  WplWindowDesc desc = {
    .title = "WPL Canvas Pan/Zoom",
    .width = 1280,
    .height = 720,
    .resizable = true
  };
  WplCanvasView view = {
    .pan = {640.0f, 360.0f},
    .zoom = 1.0f,
    .min_zoom = 0.125f,
    .max_zoom = 8.0f
  };
  WplWindow* window = NULL;
  WplDrawList* draw = NULL;
  WplResult result;
  int exit_code = 1;

  result = wpl_create_window(&desc, &window);
  if (wpl_check(result) != 0)
    return 1;

  result = wpl_create_draw_list(256u, &draw);
  if (wpl_check(result) != 0)
    goto cleanup;

  while (!wpl_window_should_close(window))
    {
      WplInputState input;
      WplVec2 cursor_canvas = {0};

      result = wpl_begin_frame(window);
      if (wpl_check(result) != 0)
        goto cleanup;

      result = wpl_pump_events(window);
      if (wpl_check(result) != 0)
        goto cleanup;

      input = wpl_get_input(window);
      if (input.keyboard.key_pressed[WPL_KEY_ESCAPE])
        {
          result = wpl_window_request_close(window);
          if (wpl_check(result) != 0)
            goto cleanup;
        }

      if (input.mouse.button_down[WPL_MOUSE_BUTTON_LEFT])
        {
          result = wpl_canvas_pan_by(&view, input.mouse.delta);
          if (wpl_check(result) != 0)
            goto cleanup;
        }

      if (input.mouse.wheel_delta > 0.0f)
        {
          result = wpl_canvas_zoom_around(&view,
                                          input.mouse.position,
                                          1.10f);
          if (wpl_check(result) != 0)
            goto cleanup;
        }
      else if (input.mouse.wheel_delta < 0.0f)
        {
          result = wpl_canvas_zoom_around(&view,
                                          input.mouse.position,
                                          1.0f / 1.10f);
          if (wpl_check(result) != 0)
            goto cleanup;
        }

      result = wpl_screen_to_canvas(&view, input.mouse.position, &cursor_canvas);
      if (wpl_check(result) != 0)
        goto cleanup;

      result = wpl_draw_list_clear(draw);
      if (wpl_check(result) != 0)
        goto cleanup;

      result = draw_scene(draw, &view, &input, cursor_canvas);
      if (wpl_check(result) != 0)
        goto cleanup;

      result = wpl_submit_draw_list(window, draw);
      if (wpl_check(result) != 0)
        goto cleanup;

      result = wpl_end_frame(window);
      if (wpl_check(result) != 0)
        goto cleanup;
    }

  exit_code = 0;

cleanup:
  wpl_destroy_draw_list(draw);
  wpl_destroy_window(window);
  return exit_code;
}
