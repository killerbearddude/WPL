/* test_input_window_isolation.c - Per-window input ownership invariants. */

#include <wpl/wpl.h>

#include "wpl_linux_x11_internal.h"

#include <assert.h>
#include <stdbool.h>

static void
test_input_state_is_owned_per_window(void)
{
  WplWindow first = {0};
  WplWindow second = {0};
  XMotionEvent motion = {0};
  WplInputState first_snapshot;
  WplInputState second_snapshot;

  wpl_linux_x11_input_press_key(&first.input, WPL_KEY_A);
  wpl_linux_x11_input_press_mouse_button(&first.input,
                                         WPL_MOUSE_BUTTON_LEFT);
  first.input.mouse.wheel_delta = 1.0f;

  wpl_linux_x11_input_press_key(&second.input, WPL_KEY_B);
  wpl_linux_x11_input_press_mouse_button(&second.input,
                                         WPL_MOUSE_BUTTON_RIGHT);
  second.input.mouse.wheel_delta = -1.0f;

  motion.x = 10;
  motion.y = 20;
  motion.state = ShiftMask;
  wpl_linux_x11_handle_motion(&first, &motion);

  motion.x = 100;
  motion.y = 200;
  motion.state = ControlMask;
  wpl_linux_x11_handle_motion(&second, &motion);

  assert(first.input.keyboard.key_down[WPL_KEY_A] == true);
  assert(first.input.keyboard.key_down[WPL_KEY_B] == false);
  assert(first.input.mouse.button_down[WPL_MOUSE_BUTTON_LEFT] == true);
  assert(first.input.mouse.button_down[WPL_MOUSE_BUTTON_RIGHT] == false);
  assert(first.input.keyboard.shift_down == true);
  assert(first.input.keyboard.ctrl_down == false);
  assert(first.input.mouse.position.x == 10.0f);
  assert(first.input.mouse.position.y == 20.0f);
  assert(first.input.mouse.wheel_delta == 1.0f);

  assert(second.input.keyboard.key_down[WPL_KEY_A] == false);
  assert(second.input.keyboard.key_down[WPL_KEY_B] == true);
  assert(second.input.mouse.button_down[WPL_MOUSE_BUTTON_LEFT] == false);
  assert(second.input.mouse.button_down[WPL_MOUSE_BUTTON_RIGHT] == true);
  assert(second.input.keyboard.shift_down == false);
  assert(second.input.keyboard.ctrl_down == true);
  assert(second.input.mouse.position.x == 100.0f);
  assert(second.input.mouse.position.y == 200.0f);
  assert(second.input.mouse.wheel_delta == -1.0f);

  wpl_linux_x11_input_reset_transients(&first.input);

  assert(first.input.keyboard.key_down[WPL_KEY_A] == true);
  assert(first.input.keyboard.key_pressed[WPL_KEY_A] == false);
  assert(first.input.mouse.button_down[WPL_MOUSE_BUTTON_LEFT] == true);
  assert(first.input.mouse.button_pressed[WPL_MOUSE_BUTTON_LEFT] == false);
  assert(first.input.mouse.wheel_delta == 0.0f);

  assert(second.input.keyboard.key_down[WPL_KEY_B] == true);
  assert(second.input.keyboard.key_pressed[WPL_KEY_B] == true);
  assert(second.input.mouse.button_down[WPL_MOUSE_BUTTON_RIGHT] == true);
  assert(second.input.mouse.button_pressed[WPL_MOUSE_BUTTON_RIGHT] == true);
  assert(second.input.mouse.wheel_delta == -1.0f);

  wpl_linux_x11_input_clear_down_state(&second.input);

  assert(first.input.keyboard.key_down[WPL_KEY_A] == true);
  assert(first.input.mouse.button_down[WPL_MOUSE_BUTTON_LEFT] == true);
  assert(first.input.keyboard.shift_down == true);

  assert(second.input.keyboard.key_down[WPL_KEY_B] == false);
  assert(second.input.keyboard.key_pressed[WPL_KEY_B] == true);
  assert(second.input.mouse.button_down[WPL_MOUSE_BUTTON_RIGHT] == false);
  assert(second.input.mouse.button_pressed[WPL_MOUSE_BUTTON_RIGHT] == true);
  assert(second.input.keyboard.ctrl_down == false);

  first_snapshot = wpl_get_input(&first);
  second_snapshot = wpl_get_input(&second);

  assert(first_snapshot.keyboard.key_down[WPL_KEY_A] == true);
  assert(first_snapshot.keyboard.key_down[WPL_KEY_B] == false);
  assert(first_snapshot.mouse.button_down[WPL_MOUSE_BUTTON_LEFT] == true);
  assert(first_snapshot.mouse.button_down[WPL_MOUSE_BUTTON_RIGHT] == false);

  assert(second_snapshot.keyboard.key_down[WPL_KEY_A] == false);
  assert(second_snapshot.keyboard.key_down[WPL_KEY_B] == false);
  assert(second_snapshot.keyboard.key_pressed[WPL_KEY_B] == true);
  assert(second_snapshot.mouse.button_down[WPL_MOUSE_BUTTON_LEFT] == false);
  assert(second_snapshot.mouse.button_down[WPL_MOUSE_BUTTON_RIGHT] == false);
  assert(second_snapshot.mouse.button_pressed[WPL_MOUSE_BUTTON_RIGHT] == true);
}

int
main(void)
{
  test_input_state_is_owned_per_window();
  return 0;
}
