/* test_window_api.c - Public window API validation that needs no X server. */

#include <wpl/wpl.h>

#include <assert.h>

int
main(void)
{
  assert(wpl_set_cursor_shape(NULL, WPL_CURSOR_ARROW)
         == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_set_cursor_shape(NULL, (WplCursorShape)-1)
         == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_set_cursor_shape(NULL,
                              (WplCursorShape)(WPL_CURSOR_NOT_ALLOWED + 1))
         == WPL_RESULT_INVALID_ARGUMENT);
  return 0;
}
