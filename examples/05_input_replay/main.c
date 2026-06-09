/* main.c - Input replay record/playback example. */

#include <wpl/wpl.h>

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#define WPL_REPLAY_EXAMPLE_DRAW_CAPACITY 4096u

static const char*
result_name(WplResult result)
{
  switch (result)
    {
    case WPL_RESULT_OK: return "WPL_RESULT_OK";
    case WPL_RESULT_ERROR: return "WPL_RESULT_ERROR";
    case WPL_RESULT_INVALID_ARGUMENT: return "WPL_RESULT_INVALID_ARGUMENT";
    case WPL_RESULT_OUT_OF_MEMORY: return "WPL_RESULT_OUT_OF_MEMORY";
    case WPL_RESULT_PLATFORM_ERROR: return "WPL_RESULT_PLATFORM_ERROR";
    case WPL_RESULT_UNSUPPORTED: return "WPL_RESULT_UNSUPPORTED";
    case WPL_RESULT_CAPACITY_EXCEEDED: return "WPL_RESULT_CAPACITY_EXCEEDED";
    case WPL_RESULT_IO_ERROR: return "WPL_RESULT_IO_ERROR";
    case WPL_RESULT_PARSE_ERROR: return "WPL_RESULT_PARSE_ERROR";
    case WPL_RESULT_TRUNCATED: return "WPL_RESULT_TRUNCATED";
    }

  return "WPL_RESULT_UNKNOWN";
}

static int
check_result(WplResult result, const char* operation)
{
  if (result == WPL_RESULT_OK)
    return 0;

  fprintf(stderr, "%s failed: %s\n", operation, result_name(result));
  return 1;
}

static void
print_usage(const char* argv0)
{
  fprintf(stderr,
          "usage:\n"
          "  %s record <path>\n"
          "  %s playback <path>\n"
          "aliases:\n"
          "  %s r <path>\n"
          "  %s p <path>\n",
          argv0,
          argv0,
          argv0,
          argv0);
}

static WplResult
append_text(WplDrawList* draw, float x, float y, const char* text, WplColor color)
{
  return wpl_draw_text(draw, (WplVec2){x, y}, text, color);
}

static WplResult
append_format(WplDrawList* draw,
              float x,
              float y,
              WplColor color,
              const char* format,
              ...)
{
  char line[256];
  va_list args;
  int written;

  va_start(args, format);
  written = vsnprintf(line, sizeof line, format, args);
  va_end(args);

  if (written < 0)
    return WPL_RESULT_ERROR;

  return append_text(draw, x, y, line, color);
}

static WplResult
append_input_lines(WplDrawList* draw,
                   const WplInputState* input,
                   float x,
                   float y,
                   float replay_dt,
                   bool show_replay_dt)
{
  const WplColor text = {0.86f, 0.88f, 0.92f, 1.0f};
  const WplColor accent = {0.95f, 0.85f, 0.30f, 1.0f};
  WplResult result;

  result = append_format(draw,
                         x,
                         y,
                         text,
                         "Mouse: %.1f, %.1f",
                         input->mouse.position.x,
                         input->mouse.position.y);
  if (result != WPL_RESULT_OK)
    return result;

  result = append_format(draw,
                         x,
                         y + 12.0f,
                         text,
                         "Delta: %.1f, %.1f",
                         input->mouse.delta.x,
                         input->mouse.delta.y);
  if (result != WPL_RESULT_OK)
    return result;

  result = append_format(draw,
                         x,
                         y + 24.0f,
                         text,
                         "Buttons: L=%d M=%d R=%d",
                         input->mouse.button_down[WPL_MOUSE_BUTTON_LEFT] ? 1 : 0,
                         input->mouse.button_down[WPL_MOUSE_BUTTON_MIDDLE] ? 1 : 0,
                         input->mouse.button_down[WPL_MOUSE_BUTTON_RIGHT] ? 1 : 0);
  if (result != WPL_RESULT_OK)
    return result;

  result = append_format(draw,
                         x,
                         y + 36.0f,
                         text,
                         "Wheel: %.1f",
                         input->mouse.wheel_delta);
  if (result != WPL_RESULT_OK)
    return result;

  result = append_format(draw,
                         x,
                         y + 48.0f,
                         text,
                         "Keys: A=%d Space=%d Esc=%d",
                         input->keyboard.key_down[WPL_KEY_A] ? 1 : 0,
                         input->keyboard.key_down[WPL_KEY_SPACE] ? 1 : 0,
                         input->keyboard.key_down[WPL_KEY_ESCAPE] ? 1 : 0);
  if (result != WPL_RESULT_OK)
    return result;

  result = append_format(draw,
                         x,
                         y + 60.0f,
                         text,
                         "Mods: Shift=%d Ctrl=%d Alt=%d",
                         input->keyboard.shift_down ? 1 : 0,
                         input->keyboard.ctrl_down ? 1 : 0,
                         input->keyboard.alt_down ? 1 : 0);
  if (result != WPL_RESULT_OK)
    return result;

  if (show_replay_dt)
    {
      result = append_format(draw,
                             x,
                             y + 72.0f,
                             accent,
                             "Replay dt: %.3f ms",
                             replay_dt * 1000.0f);
      if (result != WPL_RESULT_OK)
        return result;
    }

  return WPL_RESULT_OK;
}

static WplResult
render_input_visualization(WplDrawList* draw,
                           const WplInputState* input,
                           const char* mode,
                           const char* path,
                           bool playback_ended,
                           unsigned long long playback_frame,
                           float replay_dt,
                           bool show_replay_dt)
{
  const WplColor background = {0.04f, 0.045f, 0.055f, 1.0f};
  const WplColor panel = {0.0f, 0.0f, 0.0f, 0.55f};
  const WplColor title = {0.96f, 0.97f, 1.0f, 1.0f};
  const WplColor text = {0.78f, 0.80f, 0.84f, 1.0f};
  const WplColor cursor = {0.20f, 0.80f, 1.0f, 0.85f};
  const WplColor delta = {1.0f, 0.75f, 0.25f, 1.0f};
  WplVec2 mouse = input->mouse.position;
  WplVec2 delta_end = {
    input->mouse.position.x + input->mouse.delta.x * 10.0f,
    input->mouse.position.y + input->mouse.delta.y * 10.0f
  };
  WplResult result;

  result = wpl_draw_clear(draw, background);
  if (result != WPL_RESULT_OK)
    return result;

  result = wpl_draw_rect(draw, (WplRect){16.0f, 16.0f, 560.0f, 170.0f}, panel);
  if (result != WPL_RESULT_OK)
    return result;

  result = wpl_draw_rect_outline(draw,
                                 (WplRect){16.0f, 16.0f, 560.0f, 170.0f},
                                 (WplColor){0.35f, 0.40f, 0.48f, 1.0f},
                                 2.0f);
  if (result != WPL_RESULT_OK)
    return result;

  result = append_format(draw, 32.0f, 32.0f, title, "WPL input replay - %s", mode);
  if (result != WPL_RESULT_OK)
    return result;

  if (strcmp(mode, "RECORD") == 0)
    result = append_format(draw, 32.0f, 48.0f, text, "Saving to: %.180s", path);
  else
    result = append_format(draw, 32.0f, 48.0f, text, "Playing: %.180s", path);
  if (result != WPL_RESULT_OK)
    return result;

  if (strcmp(mode, "RECORD") == 0)
    {
      result = append_text(draw,
                           32.0f,
                           64.0f,
                           "Move mouse, click, scroll, press A/Space.",
                           text);
      if (result != WPL_RESULT_OK)
        return result;

      result = append_text(draw,
                           32.0f,
                           76.0f,
                           "Press Esc or close the window to save and exit.",
                           text);
      if (result != WPL_RESULT_OK)
        return result;
    }
  else
    {
      result = append_text(draw,
                           32.0f,
                           64.0f,
                           "Press Esc or close the window to exit playback.",
                           text);
      if (result != WPL_RESULT_OK)
        return result;

      if (playback_ended)
        result = append_text(draw,
                             32.0f,
                             76.0f,
                             "Replay ended. Press Esc or close the window.",
                             (WplColor){1.0f, 0.72f, 0.28f, 1.0f});
      else
        result = append_format(draw,
                               32.0f,
                               76.0f,
                               text,
                               "Replay frame: %llu",
                               playback_frame);
      if (result != WPL_RESULT_OK)
        return result;
    }

  result = append_input_lines(draw, input, 32.0f, 104.0f, replay_dt, show_replay_dt);
  if (result != WPL_RESULT_OK)
    return result;

  result = wpl_draw_circle(draw, mouse, 12.0f, cursor);
  if (result != WPL_RESULT_OK)
    return result;

  result = wpl_draw_circle(draw, mouse, 4.0f, (WplColor){1.0f, 1.0f, 1.0f, 1.0f});
  if (result != WPL_RESULT_OK)
    return result;

  result = wpl_draw_line(draw, mouse, delta_end, delta, 3.0f);
  if (result != WPL_RESULT_OK)
    return result;

  if (input->mouse.button_down[WPL_MOUSE_BUTTON_LEFT])
    {
      result = wpl_draw_rect(draw,
                             (WplRect){620.0f, 96.0f, 90.0f, 38.0f},
                             (WplColor){0.20f, 0.80f, 0.35f, 0.90f});
      if (result != WPL_RESULT_OK)
        return result;
    }

  if (input->mouse.button_down[WPL_MOUSE_BUTTON_MIDDLE])
    {
      result = wpl_draw_rect(draw,
                             (WplRect){728.0f, 96.0f, 90.0f, 38.0f},
                             (WplColor){0.80f, 0.70f, 0.20f, 0.90f});
      if (result != WPL_RESULT_OK)
        return result;
    }

  if (input->mouse.button_down[WPL_MOUSE_BUTTON_RIGHT])
    {
      result = wpl_draw_rect(draw,
                             (WplRect){836.0f, 96.0f, 90.0f, 38.0f},
                             (WplColor){0.90f, 0.25f, 0.25f, 0.90f});
      if (result != WPL_RESULT_OK)
        return result;
    }

  result = wpl_draw_rect_outline(draw,
                                 (WplRect){620.0f, 96.0f, 90.0f, 38.0f},
                                 (WplColor){0.80f, 0.82f, 0.88f, 1.0f},
                                 2.0f);
  if (result != WPL_RESULT_OK)
    return result;

  result = wpl_draw_rect_outline(draw,
                                 (WplRect){728.0f, 96.0f, 90.0f, 38.0f},
                                 (WplColor){0.80f, 0.82f, 0.88f, 1.0f},
                                 2.0f);
  if (result != WPL_RESULT_OK)
    return result;

  result = wpl_draw_rect_outline(draw,
                                 (WplRect){836.0f, 96.0f, 90.0f, 38.0f},
                                 (WplColor){0.80f, 0.82f, 0.88f, 1.0f},
                                 2.0f);
  if (result != WPL_RESULT_OK)
    return result;

  result = append_text(draw, 650.0f, 110.0f, "L", title);
  if (result != WPL_RESULT_OK)
    return result;

  result = append_text(draw, 758.0f, 110.0f, "M", title);
  if (result != WPL_RESULT_OK)
    return result;

  result = append_text(draw, 866.0f, 110.0f, "R", title);
  if (result != WPL_RESULT_OK)
    return result;

  return WPL_RESULT_OK;
}

static int
create_window_and_draw_list(WplWindow** out_window, WplDrawList** out_draw)
{
  WplWindowDesc desc = {
    .title = "WPL Input Replay",
    .width = 1280,
    .height = 720,
    .resizable = true
  };
  WplResult result;

  result = wpl_create_window(&desc, out_window);
  if (check_result(result, "wpl_create_window") != 0)
    return 1;

  result = wpl_create_draw_list(WPL_REPLAY_EXAMPLE_DRAW_CAPACITY, out_draw);
  if (check_result(result, "wpl_create_draw_list") != 0)
    return 1;

  return 0;
}

static int
record_mode(const char* path)
{
  WplWindow* window = NULL;
  WplDrawList* draw = NULL;
  WplReplayRecorder* recorder = NULL;
  WplResult result;
  int exit_code = 1;

  if (create_window_and_draw_list(&window, &draw) != 0)
    goto cleanup;

  result = wpl_replay_recorder_create(&recorder);
  if (check_result(result, "wpl_replay_recorder_create") != 0)
    goto cleanup;

  result = wpl_replay_recorder_begin(recorder);
  if (check_result(result, "wpl_replay_recorder_begin") != 0)
    goto cleanup;

  while (!wpl_window_should_close(window))
    {
      WplInputState input;
      float dt;

      result = wpl_begin_frame(window);
      if (check_result(result, "wpl_begin_frame") != 0)
        goto cleanup;

      result = wpl_pump_events(window);
      if (check_result(result, "wpl_pump_events") != 0)
        goto cleanup;

      input = wpl_get_input(window);
      dt = wpl_window_delta_time(window);

      result = wpl_replay_recorder_record_frame(recorder, &input, dt);
      if (check_result(result, "wpl_replay_recorder_record_frame") != 0)
        goto cleanup;

      if (input.keyboard.key_pressed[WPL_KEY_ESCAPE])
        {
          result = wpl_window_request_close(window);
          if (check_result(result, "wpl_window_request_close") != 0)
            goto cleanup;
        }

      result = wpl_draw_list_clear(draw);
      if (check_result(result, "wpl_draw_list_clear") != 0)
        goto cleanup;

      result = render_input_visualization(draw,
                                          &input,
                                          "RECORD",
                                          path,
                                          false,
                                          0ull,
                                          dt,
                                          false);
      if (check_result(result, "render_input_visualization") != 0)
        goto cleanup;

      result = wpl_submit_draw_list(window, draw);
      if (check_result(result, "wpl_submit_draw_list") != 0)
        goto cleanup;

      result = wpl_end_frame(window);
      if (check_result(result, "wpl_end_frame") != 0)
        goto cleanup;
    }

  result = wpl_replay_recorder_save(recorder, path);
  if (check_result(result, "wpl_replay_recorder_save") != 0)
    goto cleanup;

  exit_code = 0;

cleanup:
  wpl_replay_recorder_destroy(recorder);
  wpl_destroy_draw_list(draw);
  wpl_destroy_window(window);
  return exit_code;
}

static int
playback_mode(const char* path)
{
  WplWindow* window = NULL;
  WplDrawList* draw = NULL;
  WplReplayPlayer* player = NULL;
  WplInputState replay_input = {0};
  float replay_dt = 0.0f;
  bool playback_ended = false;
  bool has_frame = false;
  unsigned long long playback_frame = 0ull;
  WplResult result;
  int exit_code = 1;

  result = wpl_replay_player_create(&player);
  if (check_result(result, "wpl_replay_player_create") != 0)
    goto cleanup;

  result = wpl_replay_player_load(player, path);
  if (check_result(result, "wpl_replay_player_load") != 0)
    goto cleanup;

  if (create_window_and_draw_list(&window, &draw) != 0)
    goto cleanup;

  while (!wpl_window_should_close(window))
    {
      WplInputState live_input;

      result = wpl_begin_frame(window);
      if (check_result(result, "wpl_begin_frame") != 0)
        goto cleanup;

      result = wpl_pump_events(window);
      if (check_result(result, "wpl_pump_events") != 0)
        goto cleanup;

      live_input = wpl_get_input(window);
      if (live_input.keyboard.key_pressed[WPL_KEY_ESCAPE])
        {
          result = wpl_window_request_close(window);
          if (check_result(result, "wpl_window_request_close") != 0)
            goto cleanup;
        }

      if (!playback_ended)
        {
          WplInputState next_input = {0};
          float next_dt = 0.0f;

          result = wpl_replay_player_next(player, &next_input, &next_dt, &has_frame);
          if (check_result(result, "wpl_replay_player_next") != 0)
            goto cleanup;

          if (has_frame)
            {
              replay_input = next_input;
              replay_dt = next_dt;
              playback_frame++;
            }
          else
            {
              playback_ended = true;
            }
        }

      result = wpl_draw_list_clear(draw);
      if (check_result(result, "wpl_draw_list_clear") != 0)
        goto cleanup;

      result = render_input_visualization(draw,
                                          &replay_input,
                                          "PLAYBACK",
                                          path,
                                          playback_ended,
                                          playback_frame,
                                          replay_dt,
                                          true);
      if (check_result(result, "render_input_visualization") != 0)
        goto cleanup;

      result = wpl_submit_draw_list(window, draw);
      if (check_result(result, "wpl_submit_draw_list") != 0)
        goto cleanup;

      result = wpl_end_frame(window);
      if (check_result(result, "wpl_end_frame") != 0)
        goto cleanup;
    }

  exit_code = 0;

cleanup:
  wpl_replay_player_destroy(player);
  wpl_destroy_draw_list(draw);
  wpl_destroy_window(window);
  return exit_code;
}

int
main(int argc, char** argv)
{
  const char* mode;
  const char* path;

  if (argc != 3)
    {
      print_usage(argc > 0 ? argv[0] : "wpl_input_replay");
      return 2;
    }

  mode = argv[1];
  path = argv[2];

  if (path == NULL || path[0] == '\0')
    {
      print_usage(argv[0]);
      return 2;
    }

  if (strcmp(mode, "record") == 0 || strcmp(mode, "r") == 0)
    return record_mode(path);

  if (strcmp(mode, "playback") == 0 || strcmp(mode, "p") == 0)
    return playback_mode(path);

  print_usage(argv[0]);
  return 2;
}
