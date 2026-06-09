/* main.c - Minimal smoke example for clear and filled rectangle rendering. */

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

      result = wpl_draw_rect(draw,
                             (WplRect){100.0f, 100.0f, 300.0f, 180.0f},
                             (WplColor){0.15f, 0.35f, 0.80f, 1.0f});
      if (wpl_check(result) != 0)
        goto cleanup;

      result = wpl_draw_rect(draw,
                             (WplRect){220.0f, 160.0f, 300.0f, 180.0f},
                             (WplColor){0.90f, 0.25f, 0.15f, 0.50f});
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
