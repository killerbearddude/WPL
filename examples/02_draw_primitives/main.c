/* main.c - Minimal smoke example for basic software rendering primitives. */

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
    .title = "WPL Draw Primitives",
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

  result = wpl_create_draw_list(16u, &draw);
  if (wpl_check(result) != 0)
    goto cleanup;

  while (!wpl_window_should_close(window))
    {
      WplInputState input;

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

      result = wpl_draw_list_clear(draw);
      if (wpl_check(result) != 0)
        goto cleanup;

      result = wpl_draw_clear(draw, (WplColor){0.05f, 0.05f, 0.06f, 1.0f});
      if (wpl_check(result) != 0)
        goto cleanup;

      result = wpl_draw_text(draw,
                             (WplVec2){80.0f, 40.0f},
                             "WPL software renderer",
                             (WplColor){0.95f, 0.95f, 0.95f, 1.0f});
      if (wpl_check(result) != 0)
        goto cleanup;

      result = wpl_draw_rect(draw,
                             (WplRect){80.0f, 80.0f, 280.0f, 160.0f},
                             (WplColor){0.15f, 0.35f, 0.80f, 1.0f});
      if (wpl_check(result) != 0)
        goto cleanup;

      result = wpl_draw_rect(draw,
                             (WplRect){200.0f, 140.0f, 280.0f, 160.0f},
                             (WplColor){0.90f, 0.25f, 0.15f, 0.50f});
      if (wpl_check(result) != 0)
        goto cleanup;

      result = wpl_draw_rect_outline(draw,
                                     (WplRect){60.0f,
                                               60.0f,
                                               460.0f,
                                               280.0f},
                                     (WplColor){0.90f,
                                                0.90f,
                                                0.90f,
                                                1.0f},
                                     4.0f);
      if (wpl_check(result) != 0)
        goto cleanup;

      result = wpl_draw_line(draw,
                             (WplVec2){80.0f, 380.0f},
                             (WplVec2){520.0f, 520.0f},
                             (WplColor){0.20f, 0.90f, 0.40f, 1.0f},
                             8.0f);
      if (wpl_check(result) != 0)
        goto cleanup;

      result = wpl_draw_line(draw,
                             (WplVec2){620.0f, 400.0f},
                             (WplVec2){920.0f, 400.0f},
                             (WplColor){0.35f, 0.65f, 1.0f, 1.0f},
                             2.0f);
      if (wpl_check(result) != 0)
        goto cleanup;

      result = wpl_draw_line(draw,
                             (WplVec2){620.0f, 430.0f},
                             (WplVec2){620.0f, 560.0f},
                             (WplColor){1.0f, 0.65f, 0.35f, 1.0f},
                             2.0f);
      if (wpl_check(result) != 0)
        goto cleanup;

      result = wpl_draw_circle(draw,
                               (WplVec2){640.0f, 220.0f},
                               90.0f,
                               (WplColor){0.90f, 0.80f, 0.20f, 0.75f});
      if (wpl_check(result) != 0)
        goto cleanup;

      result = wpl_draw_circle(draw,
                               (WplVec2){760.0f, 260.0f},
                               70.0f,
                               (WplColor){0.20f, 0.80f, 0.90f, 0.55f});
      if (wpl_check(result) != 0)
        goto cleanup;

      result = wpl_draw_text(draw,
                             (WplVec2){620.0f, 340.0f},
                             "ASCII 32-126\nnewline supported",
                             (WplColor){0.95f, 0.85f, 0.25f, 1.0f});
      if (wpl_check(result) != 0)
        goto cleanup;

      result = wpl_draw_text(draw,
                             (WplVec2){80.0f, 620.0f},
                             "Esc closes. Resize remains safe. 0-9 !?+-*/",
                             (WplColor){0.75f, 0.78f, 0.82f, 1.0f});
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
