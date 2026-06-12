/* wpl_linux_x11_backend.c - Linux/X11 private backend dispatch. */

#include "wpl_backend_internal.h"
#include "wpl_linux_x11_internal.h"

static const WplBackendVTable wpl_linux_x11_vtable = {
  "linux_x11",
  wpl_linux_x11_create_window,
  wpl_linux_x11_destroy_window,
  wpl_linux_x11_window_should_close,
  wpl_linux_x11_window_request_close,
  wpl_linux_x11_set_cursor_shape,
  wpl_linux_x11_begin_frame,
  wpl_linux_x11_pump_events,
  wpl_linux_x11_end_frame,
  wpl_linux_x11_window_width,
  wpl_linux_x11_window_height,
  wpl_linux_x11_window_delta_time,
  wpl_linux_x11_submit_draw_list
};

const WplBackendVTable*
wpl_linux_x11_backend(void)
{
  return &wpl_linux_x11_vtable;
}

static const WplBackendVTable*
wpl_default_backend(void)
{
  return wpl_linux_x11_backend();
}

WplResult
wpl_create_window(const WplWindowDesc* desc, WplWindow** out_window)
{
  const WplBackendVTable* backend = wpl_default_backend();

  if (backend == NULL || backend->create_window == NULL)
    {
      if (out_window != NULL)
        *out_window = NULL;
      return WPL_RESULT_PLATFORM_ERROR;
    }

  return backend->create_window(desc, out_window);
}

void
wpl_destroy_window(WplWindow* window)
{
  const WplBackendVTable* backend;

  if (window == NULL)
    return;

  backend = wpl_default_backend();
  if (backend == NULL || backend->destroy_window == NULL)
    return;

  backend->destroy_window(window);
}

bool
wpl_window_should_close(const WplWindow* window)
{
  const WplBackendVTable* backend;

  if (window == NULL)
    return true;

  backend = wpl_default_backend();
  if (backend == NULL || backend->window_should_close == NULL)
    return true;

  return backend->window_should_close(window);
}

WplResult
wpl_window_request_close(WplWindow* window)
{
  const WplBackendVTable* backend = wpl_default_backend();

  if (backend == NULL || backend->window_request_close == NULL)
    return WPL_RESULT_PLATFORM_ERROR;

  return backend->window_request_close(window);
}

WplResult
wpl_set_cursor_shape(WplWindow* window, WplCursorShape shape)
{
  const WplBackendVTable* backend = wpl_default_backend();

  if (backend == NULL || backend->set_cursor_shape == NULL)
    return WPL_RESULT_PLATFORM_ERROR;

  return backend->set_cursor_shape(window, shape);
}

WplResult
wpl_begin_frame(WplWindow* window)
{
  const WplBackendVTable* backend = wpl_default_backend();

  if (backend == NULL || backend->begin_frame == NULL)
    return WPL_RESULT_PLATFORM_ERROR;

  return backend->begin_frame(window);
}

WplResult
wpl_pump_events(WplWindow* window)
{
  const WplBackendVTable* backend = wpl_default_backend();

  if (backend == NULL || backend->pump_events == NULL)
    return WPL_RESULT_PLATFORM_ERROR;

  return backend->pump_events(window);
}

WplResult
wpl_end_frame(WplWindow* window)
{
  const WplBackendVTable* backend = wpl_default_backend();

  if (backend == NULL || backend->end_frame == NULL)
    return WPL_RESULT_PLATFORM_ERROR;

  return backend->end_frame(window);
}

int
wpl_window_width(const WplWindow* window)
{
  const WplBackendVTable* backend;

  if (window == NULL)
    return 0;

  backend = wpl_default_backend();
  if (backend == NULL || backend->window_width == NULL)
    return 0;

  return backend->window_width(window);
}

int
wpl_window_height(const WplWindow* window)
{
  const WplBackendVTable* backend;

  if (window == NULL)
    return 0;

  backend = wpl_default_backend();
  if (backend == NULL || backend->window_height == NULL)
    return 0;

  return backend->window_height(window);
}

float
wpl_window_delta_time(const WplWindow* window)
{
  const WplBackendVTable* backend;

  if (window == NULL)
    return 0.0f;

  backend = wpl_default_backend();
  if (backend == NULL || backend->window_delta_time == NULL)
    return 0.0f;

  return backend->window_delta_time(window);
}

WplResult
wpl_submit_draw_list(WplWindow* window, const WplDrawList* list)
{
  const WplBackendVTable* backend = wpl_default_backend();

  if (backend == NULL || backend->submit_draw_list == NULL)
    return WPL_RESULT_PLATFORM_ERROR;

  return backend->submit_draw_list(window, list);
}
