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

  assert(backend->create_window != NULL);
  assert(backend->destroy_window != NULL);
  assert(backend->window_should_close != NULL);
  assert(backend->window_request_close != NULL);
  assert(backend->set_cursor_shape != NULL);

  assert(backend->begin_frame != NULL);
  assert(backend->pump_events != NULL);
  assert(backend->end_frame != NULL);

  assert(backend->window_width != NULL);
  assert(backend->window_height != NULL);
  assert(backend->window_delta_time != NULL);
  assert(backend->submit_draw_list != NULL);

  return 0;
}
