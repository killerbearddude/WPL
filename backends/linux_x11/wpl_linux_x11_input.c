/* wpl_linux_x11_input.c - Linux/X11 input snapshot storage helpers. */

#include "wpl_linux_x11_internal.h"

#include <stddef.h>

void
wpl_linux_x11_reset_transient_input(WplInputState* input)
{
  int button_index;
  int key_index;

  if (input == NULL)
    return;

  input->mouse.delta.x = 0.0f;
  input->mouse.delta.y = 0.0f;
  input->mouse.wheel_delta = 0.0f;

  for (button_index = 0; button_index < WPL_MOUSE_BUTTON_COUNT; ++button_index)
    {
      input->mouse.button_pressed[button_index] = false;
      input->mouse.button_released[button_index] = false;
    }

  for (key_index = 0; key_index < WPL_KEY_COUNT; ++key_index)
    {
      input->keyboard.key_pressed[key_index] = false;
      input->keyboard.key_released[key_index] = false;
    }
}

void
wpl_linux_x11_clear_held_input(WplInputState* input)
{
  int button_index;
  int key_index;

  if (input == NULL)
    return;

  for (button_index = 0; button_index < WPL_MOUSE_BUTTON_COUNT; ++button_index)
    input->mouse.button_down[button_index] = false;

  for (key_index = 0; key_index < WPL_KEY_COUNT; ++key_index)
    input->keyboard.key_down[key_index] = false;

  input->keyboard.shift_down = false;
  input->keyboard.ctrl_down = false;
  input->keyboard.alt_down = false;
}

WplInputState
wpl_get_input(const WplWindow* window)
{
  WplInputState input = {0};

  if (window == NULL)
    return input;

  return window->input;
}
