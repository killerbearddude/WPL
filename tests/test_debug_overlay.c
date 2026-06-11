#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "wpl/wpl_debug.h"
#include "wpl/wpl_draw.h"
#include "wpl/wpl_input.h"
#include "wpl/wpl_result.h"

#define WPL_TEST_DEBUG_OVERLAY_COMMAND_COUNT 10u

static WplDebugStats
wpl_test_stats(void)
{
  WplDebugStats stats;
  stats.fps = 60.0f;
  stats.frame_ms = 16.67f;
  stats.window_width = 1280;
  stats.window_height = 720;
  stats.draw_command_count = 4;
  stats.backend_name = "test_backend";
  return stats;
}

static WplInputState
wpl_test_input(void)
{
  WplInputState input;
  memset(&input, 0, sizeof(input));

  input.mouse.position.x = 10.0f;
  input.mouse.position.y = 20.0f;
  input.mouse.delta.x = 1.0f;
  input.mouse.delta.y = -2.0f;
  input.mouse.button_down[WPL_MOUSE_BUTTON_LEFT] = true;
  input.mouse.wheel_delta = 1.0f;

  return input;
}

static void
test_invalid_arguments(void)
{
  WplDrawList* list = NULL;
  WplDebugStats stats = wpl_test_stats();
  WplInputState input = wpl_test_input();
  WplDebugLine line = {"Nodes", "42"};
  WplDebugLine invalid_label = {NULL, "42"};
  WplDebugLine invalid_value = {"Nodes", NULL};

  assert(wpl_create_draw_list(WPL_TEST_DEBUG_OVERLAY_COMMAND_COUNT + 1u, &list) ==
         WPL_RESULT_OK);

  assert(wpl_debug_draw_overlay(NULL, &stats, &input) ==
         WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_debug_draw_overlay(list, NULL, &input) ==
         WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_debug_draw_overlay(list, &stats, NULL) ==
         WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_debug_draw_overlay_ex(NULL, &stats, &input, NULL, 0u) ==
         WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_debug_draw_overlay_ex(list, NULL, &input, NULL, 0u) ==
         WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_debug_draw_overlay_ex(list, &stats, NULL, NULL, 0u) ==
         WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_debug_draw_overlay_ex(list, &stats, &input, NULL, 1u) ==
         WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_debug_draw_overlay_ex(list, &stats, &input, &invalid_label, 1u) ==
         WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_debug_draw_overlay_ex(list, &stats, &input, &invalid_value, 1u) ==
         WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_draw_list_count(list) == 0u);

  assert(wpl_debug_draw_overlay_ex(list, &stats, &input, &line, 1u) ==
         WPL_RESULT_OK);
  assert(wpl_draw_list_count(list) == WPL_TEST_DEBUG_OVERLAY_COMMAND_COUNT + 1u);

  wpl_destroy_draw_list(list);
}

static void
test_backend_name_null_succeeds(void)
{
  WplDrawList* list = NULL;
  WplDebugStats stats = wpl_test_stats();
  WplInputState input = wpl_test_input();

  stats.backend_name = NULL;

  assert(wpl_create_draw_list(WPL_TEST_DEBUG_OVERLAY_COMMAND_COUNT, &list) ==
         WPL_RESULT_OK);

  assert(wpl_debug_draw_overlay(list, &stats, &input) == WPL_RESULT_OK);
  assert(wpl_draw_list_count(list) == WPL_TEST_DEBUG_OVERLAY_COMMAND_COUNT);

  wpl_destroy_draw_list(list);
}

static void
test_insufficient_capacity_does_not_mutate_count(void)
{
  WplDrawList* list = NULL;
  WplDebugStats stats = wpl_test_stats();
  WplInputState input = wpl_test_input();
  WplDebugLine lines[2] = {
    {"Nodes", "42"},
    {"Links", "17"}
  };
  size_t before = 0;

  assert(wpl_create_draw_list(WPL_TEST_DEBUG_OVERLAY_COMMAND_COUNT - 1u, &list) ==
         WPL_RESULT_OK);

  before = wpl_draw_list_count(list);
  assert(wpl_debug_draw_overlay(list, &stats, &input) ==
         WPL_RESULT_CAPACITY_EXCEEDED);
  assert(wpl_draw_list_count(list) == before);

  wpl_destroy_draw_list(list);

  assert(wpl_create_draw_list(WPL_TEST_DEBUG_OVERLAY_COMMAND_COUNT + 1u, &list) ==
         WPL_RESULT_OK);

  before = wpl_draw_list_count(list);
  assert(wpl_debug_draw_overlay_ex(list, &stats, &input, lines, 2u) ==
         WPL_RESULT_CAPACITY_EXCEEDED);
  assert(wpl_draw_list_count(list) == before);

  wpl_destroy_draw_list(list);
}

static void
test_success_appends_expected_count(void)
{
  WplDrawList* list = NULL;
  WplDebugStats stats = wpl_test_stats();
  WplInputState input = wpl_test_input();
  WplDebugLine lines[3] = {
    {"Nodes", "42"},
    {"Links", "17"},
    {"Mode", "dragging-link"}
  };

  assert(wpl_create_draw_list(WPL_TEST_DEBUG_OVERLAY_COMMAND_COUNT, &list) ==
         WPL_RESULT_OK);

  assert(wpl_debug_draw_overlay(list, &stats, &input) == WPL_RESULT_OK);
  assert(wpl_draw_list_count(list) == WPL_TEST_DEBUG_OVERLAY_COMMAND_COUNT);
  assert(wpl_draw_list_capacity(list) == WPL_TEST_DEBUG_OVERLAY_COMMAND_COUNT);

  wpl_destroy_draw_list(list);

  assert(wpl_create_draw_list(WPL_TEST_DEBUG_OVERLAY_COMMAND_COUNT, &list) ==
         WPL_RESULT_OK);

  assert(wpl_debug_draw_overlay_ex(list, &stats, &input, NULL, 0u) ==
         WPL_RESULT_OK);
  assert(wpl_draw_list_count(list) == WPL_TEST_DEBUG_OVERLAY_COMMAND_COUNT);

  wpl_destroy_draw_list(list);

  assert(wpl_create_draw_list(WPL_TEST_DEBUG_OVERLAY_COMMAND_COUNT + 3u, &list) ==
         WPL_RESULT_OK);

  assert(wpl_debug_draw_overlay_ex(list, &stats, &input, lines, 1u) ==
         WPL_RESULT_OK);
  assert(wpl_draw_list_count(list) == WPL_TEST_DEBUG_OVERLAY_COMMAND_COUNT + 1u);

  assert(wpl_draw_list_clear(list) == WPL_RESULT_OK);
  assert(wpl_debug_draw_overlay_ex(list, &stats, &input, lines, 3u) ==
         WPL_RESULT_OK);
  assert(wpl_draw_list_count(list) == WPL_TEST_DEBUG_OVERLAY_COMMAND_COUNT + 3u);

  wpl_destroy_draw_list(list);
}

static void
test_overlay_preserves_existing_commands(void)
{
  WplDrawList* list = NULL;
  WplDebugStats stats = wpl_test_stats();
  WplInputState input = wpl_test_input();
  size_t before = 0;

  assert(wpl_create_draw_list(WPL_TEST_DEBUG_OVERLAY_COMMAND_COUNT + 2u, &list) ==
         WPL_RESULT_OK);

  assert(wpl_draw_clear(list, (WplColor){0.0f, 0.0f, 0.0f, 1.0f}) ==
         WPL_RESULT_OK);
  assert(wpl_draw_rect(list,
                       (WplRect){1.0f, 2.0f, 3.0f, 4.0f},
                       (WplColor){1.0f, 0.0f, 0.0f, 1.0f}) == WPL_RESULT_OK);

  before = wpl_draw_list_count(list);
  assert(before == 2u);

  assert(wpl_debug_draw_overlay(list, &stats, &input) == WPL_RESULT_OK);
  assert(wpl_draw_list_count(list) ==
         before + WPL_TEST_DEBUG_OVERLAY_COMMAND_COUNT);

  wpl_destroy_draw_list(list);
}


static void
test_truncated_custom_line_does_not_mutate_count(void)
{
  WplDrawList* list = NULL;
  WplDebugStats stats = wpl_test_stats();
  WplInputState input = wpl_test_input();
  char long_value[160];
  WplDebugLine line = {"Long", long_value};
  size_t before = 0;

  memset(long_value, 'x', sizeof(long_value) - 1u);
  long_value[sizeof(long_value) - 1u] = '\0';

  assert(wpl_create_draw_list(WPL_TEST_DEBUG_OVERLAY_COMMAND_COUNT + 1u, &list) ==
         WPL_RESULT_OK);

  before = wpl_draw_list_count(list);
  assert(wpl_debug_draw_overlay_ex(list, &stats, &input, &line, 1u) ==
         WPL_RESULT_TRUNCATED);
  assert(wpl_draw_list_count(list) == before);

  wpl_destroy_draw_list(list);
}

int
main(void)
{
  test_invalid_arguments();
  test_backend_name_null_succeeds();
  test_insufficient_capacity_does_not_mutate_count();
  test_success_appends_expected_count();
  test_overlay_preserves_existing_commands();
  test_truncated_custom_line_does_not_mutate_count();

  return 0;
}
