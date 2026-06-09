/* main.c - Minimal debug overlay example. */

#include <wpl/wpl.h>

static int
wpl_check(WplResult result)
{
  return result == WPL_RESULT_OK ? 0 : 1;
}

int
main(void)
{
  WplWindowDesc desc = {
    .title = "WPL Debug Overlay",
    .width = 1280,
    .height = 720,
    .resizable = true
  };
  WplWindow* window = NULL;
  WplDrawList* draw = NULL;
  WplResult result;
  int exit_code = 1;

  result = wpl_create_window(&desc, &window);
  if (wpl_check(result) != 0)
    return 1;

  result = wpl_create_draw_list(64u, &draw);
  if (wpl_check(result) != 0)
    goto cleanup;

  while (!wpl_window_should_close(window))
    {
      WplInputState input;
      WplDebugStats stats;
      float dt;
      float frame_ms;
      float fps;

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

      dt = wpl_window_delta_time(window);
      frame_ms = dt > 0.0f ? dt * 1000.0f : 0.0f;
      fps = dt > 0.0f ? 1.0f / dt : 0.0f;

      result = wpl_draw_list_clear(draw);
      if (wpl_check(result) != 0)
        goto cleanup;

      result = wpl_draw_clear(draw, (WplColor){0.05f, 0.05f, 0.06f, 1.0f});
      if (wpl_check(result) != 0)
        goto cleanup;

      result = wpl_draw_rect(draw,
                             (WplRect){80.0f, 120.0f, 260.0f, 160.0f},
                             (WplColor){0.20f, 0.35f, 0.80f, 1.0f});
      if (wpl_check(result) != 0)
        goto cleanup;

      result = wpl_draw_circle(draw,
                               (WplVec2){470.0f, 200.0f},
                               80.0f,
                               (WplColor){0.90f, 0.30f, 0.20f, 0.65f});
      if (wpl_check(result) != 0)
        goto cleanup;

      stats.fps = fps;
      stats.frame_ms = frame_ms;
      stats.window_width = wpl_window_width(window);
      stats.window_height = wpl_window_height(window);
      stats.draw_command_count = wpl_draw_list_count(draw);
      stats.backend_name = "linux_x11";

      result = wpl_debug_draw_overlay(draw, &stats, &input);
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
