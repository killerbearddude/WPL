/* test_backend_dispatch.c - Public backend-dispatch fallback smoke coverage. */

#include "wpl/wpl_draw.h"
#include "wpl/wpl_result.h"
#include "wpl/wpl_time.h"
#include "wpl/wpl_window.h"

#include <assert.h>
#include <stddef.h>

int
main(void)
{
  WplWindow* window = (WplWindow*)0;
  WplWindowDesc desc = {0};
  WplDrawList* draw_list = NULL;

  assert(wpl_create_window(NULL, &window) == WPL_RESULT_INVALID_ARGUMENT);
  assert(window == NULL);
  assert(wpl_create_window(&desc, NULL) == WPL_RESULT_INVALID_ARGUMENT);

  wpl_destroy_window(NULL);

  assert(wpl_window_should_close(NULL));
  assert(wpl_window_request_close(NULL) == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_set_cursor_shape(NULL, WPL_CURSOR_ARROW)
         == WPL_RESULT_INVALID_ARGUMENT);

  assert(wpl_begin_frame(NULL) == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_pump_events(NULL) == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_end_frame(NULL) == WPL_RESULT_INVALID_ARGUMENT);

  assert(wpl_window_width(NULL) == 0);
  assert(wpl_window_height(NULL) == 0);
  assert(wpl_window_delta_time(NULL) == 0.0f);

  assert(wpl_submit_draw_list(NULL, NULL) == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_create_draw_list(1u, &draw_list) == WPL_RESULT_OK);
  assert(draw_list != NULL);
  assert(wpl_submit_draw_list(NULL, draw_list) == WPL_RESULT_INVALID_ARGUMENT);
  wpl_destroy_draw_list(draw_list);

  return 0;
}
