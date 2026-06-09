/* main.c - minimal WPL X11 empty-window smoke example. */

#include <wpl/wpl.h>

int
main(void)
{
  WplWindowDesc desc = {
    .title = "WPL Empty Window",
    .width = 1280,
    .height = 720,
    .resizable = true
  };
  WplWindow* window = NULL;
  WplResult result;

  result = wpl_create_window(&desc, &window);
  if (result != WPL_RESULT_OK)
    return 1;

  while (!wpl_window_should_close(window))
    {
      result = wpl_begin_frame(window);
      if (result != WPL_RESULT_OK)
        {
          wpl_destroy_window(window);
          return 1;
        }

      result = wpl_pump_events(window);
      if (result != WPL_RESULT_OK)
        {
          wpl_destroy_window(window);
          return 1;
        }

      result = wpl_end_frame(window);
      if (result != WPL_RESULT_OK)
        {
          wpl_destroy_window(window);
          return 1;
        }
    }

  wpl_destroy_window(window);
  return 0;
}
