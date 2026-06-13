/* test_draw_list_edges.c - Focused draw-list edge-case tests. */

#include <wpl/wpl.h>

#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <string.h>

static WplColor
wpl_test_color(void)
{
  WplColor color = {0.1f, 0.2f, 0.3f, 1.0f};
  return color;
}

static WplRect
wpl_test_rect(void)
{
  WplRect rect = {1.0f, 2.0f, 3.0f, 4.0f};
  return rect;
}

static WplVec2
wpl_test_vec2(float x, float y)
{
  WplVec2 value = {x, y};
  return value;
}

static WplPanelStyle
wpl_test_panel_style(void)
{
  WplPanelStyle style;
  style.fill_color = wpl_test_color();
  style.border_color = (WplColor){0.8f, 0.7f, 0.6f, 1.0f};
  style.border_thickness = 1.0f;
  style.corner_radius = 3.0f;
  return style;
}

static WplDrawList*
wpl_test_create_list(size_t capacity)
{
  WplDrawList* list = NULL;
  WplResult result = wpl_create_draw_list(capacity, &list);

  assert(result == WPL_RESULT_OK);
  assert(list != NULL);
  return list;
}

static void
test_create_rejects_impossible_capacity_and_clears_output(void)
{
  WplDrawList* list = (WplDrawList*)1;

  assert(wpl_create_draw_list(SIZE_MAX, &list) == WPL_RESULT_OUT_OF_MEMORY);
  assert(list == NULL);
}

static void
test_invalid_simple_commands_precede_capacity_failure(void)
{
  WplDrawList* list = wpl_test_create_list(1u);
  WplRect rect = wpl_test_rect();
  WplColor color = wpl_test_color();
  char rejected_text[WPL_DRAW_TEXT_MAX_BYTES + 2u];

  memset(rejected_text, 'x', sizeof(rejected_text));
  rejected_text[WPL_DRAW_TEXT_MAX_BYTES + 1u] = '\0';

  assert(wpl_draw_clear(list, color) == WPL_RESULT_OK);
  assert(wpl_draw_list_count(list) == 1u);

  rect.w = -1.0f;
  assert(wpl_draw_rect(list, rect, color) == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_draw_list_count(list) == 1u);

  assert(wpl_draw_line(list,
                       wpl_test_vec2(NAN, 0.0f),
                       wpl_test_vec2(1.0f, 1.0f),
                       color,
                       1.0f)
         == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_draw_list_count(list) == 1u);

  color.g = INFINITY;
  assert(wpl_draw_clear(list, color) == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_draw_list_count(list) == 1u);

  assert(wpl_draw_text(list,
                       wpl_test_vec2(0.0f, 0.0f),
                       NULL,
                       wpl_test_color())
         == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_draw_list_count(list) == 1u);

  assert(wpl_draw_text(list,
                       wpl_test_vec2(0.0f, 0.0f),
                       rejected_text,
                       wpl_test_color())
         == WPL_RESULT_TRUNCATED);
  assert(wpl_draw_list_count(list) == 1u);

  assert(wpl_draw_rect(list, wpl_test_rect(), wpl_test_color())
         == WPL_RESULT_CAPACITY_EXCEEDED);
  assert(wpl_draw_list_count(list) == 1u);

  wpl_destroy_draw_list(list);
}

static void
test_text_empty_max_and_over_limit_edges(void)
{
  WplDrawList* list = wpl_test_create_list(2u);
  char accepted_text[WPL_DRAW_TEXT_MAX_BYTES + 1u];
  char rejected_text[WPL_DRAW_TEXT_MAX_BYTES + 2u];

  memset(accepted_text, 'a', sizeof(accepted_text));
  accepted_text[WPL_DRAW_TEXT_MAX_BYTES] = '\0';

  memset(rejected_text, 'b', sizeof(rejected_text));
  rejected_text[WPL_DRAW_TEXT_MAX_BYTES + 1u] = '\0';

  assert(wpl_draw_text(list,
                       wpl_test_vec2(0.0f, 0.0f),
                       "",
                       wpl_test_color())
         == WPL_RESULT_OK);
  assert(wpl_draw_list_count(list) == 1u);

  assert(wpl_draw_text(list,
                       wpl_test_vec2(0.0f, 0.0f),
                       accepted_text,
                       wpl_test_color())
         == WPL_RESULT_OK);
  assert(wpl_draw_list_count(list) == 2u);

  assert(wpl_draw_text(list,
                       wpl_test_vec2(0.0f, 0.0f),
                       rejected_text,
                       wpl_test_color())
         == WPL_RESULT_TRUNCATED);
  assert(wpl_draw_list_count(list) == 2u);

  assert(wpl_draw_text(list,
                       wpl_test_vec2(0.0f, 0.0f),
                       "capacity",
                       wpl_test_color())
         == WPL_RESULT_CAPACITY_EXCEEDED);
  assert(wpl_draw_list_count(list) == 2u);

  wpl_destroy_draw_list(list);
}

static void
test_panel_invalid_input_precedes_capacity_failure(void)
{
  WplDrawList* list = wpl_test_create_list(1u);
  WplPanelStyle style = wpl_test_panel_style();
  WplRect rect = wpl_test_rect();

  assert(wpl_draw_clear(list, wpl_test_color()) == WPL_RESULT_OK);
  assert(wpl_draw_list_count(list) == 1u);

  rect.h = -1.0f;
  assert(wpl_draw_panel(list, rect, style) == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_draw_list_count(list) == 1u);

  rect = wpl_test_rect();
  style.fill_color.r = NAN;
  assert(wpl_draw_panel(list, rect, style) == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_draw_list_count(list) == 1u);

  style = wpl_test_panel_style();
  style.border_thickness = -1.0f;
  assert(wpl_draw_panel(list, rect, style) == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_draw_list_count(list) == 1u);

  style = wpl_test_panel_style();
  style.border_thickness = 0.0f;
  assert(wpl_draw_panel(list, rect, style) == WPL_RESULT_CAPACITY_EXCEEDED);
  assert(wpl_draw_list_count(list) == 1u);

  wpl_destroy_draw_list(list);
}

static void
test_panel_collapsed_inner_exact_capacity(void)
{
  WplDrawList* list = wpl_test_create_list(1u);
  WplPanelStyle style = wpl_test_panel_style();
  WplRect tiny_rect = {0.0f, 0.0f, 1.0f, 1.0f};

  style.border_thickness = 2.0f;

  assert(wpl_draw_panel(list, tiny_rect, style) == WPL_RESULT_OK);
  assert(wpl_draw_list_count(list) == 1u);

  assert(wpl_draw_panel(list, tiny_rect, style) == WPL_RESULT_CAPACITY_EXCEEDED);
  assert(wpl_draw_list_count(list) == 1u);

  wpl_destroy_draw_list(list);
}

int
main(void)
{
  test_create_rejects_impossible_capacity_and_clears_output();
  test_invalid_simple_commands_precede_capacity_failure();
  test_text_empty_max_and_over_limit_edges();
  test_panel_invalid_input_precedes_capacity_failure();
  test_panel_collapsed_inner_exact_capacity();
  return 0;
}
