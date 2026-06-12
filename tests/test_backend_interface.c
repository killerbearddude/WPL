/* test_backend_interface.c - Private backend vtable smoke coverage. */

#include "wpl_backend_internal.h"
#include "wpl_linux_x11_internal.h"

#include <assert.h>
#include <string.h>

int
main(void)
{
  const WplBackendVTable* backend = wpl_linux_x11_backend();

  assert(backend != NULL);
  assert(backend->name != NULL);
  assert(strcmp(backend->name, "linux_x11") == 0);

  assert(backend->create_window == wpl_linux_x11_create_window);
  assert(backend->destroy_window == wpl_linux_x11_destroy_window);
  assert(backend->window_should_close == wpl_linux_x11_window_should_close);
  assert(backend->window_request_close == wpl_linux_x11_window_request_close);
  assert(backend->set_cursor_shape == wpl_linux_x11_set_cursor_shape);

  assert(backend->begin_frame == wpl_linux_x11_begin_frame);
  assert(backend->pump_events == wpl_linux_x11_pump_events);
  assert(backend->end_frame == wpl_linux_x11_end_frame);

  assert(backend->window_width == wpl_linux_x11_window_width);
  assert(backend->window_height == wpl_linux_x11_window_height);
  assert(backend->window_delta_time == wpl_linux_x11_window_delta_time);
  assert(backend->submit_draw_list == wpl_linux_x11_submit_draw_list);

  return 0;
}
