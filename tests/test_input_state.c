/* test_input_state.c - Public input snapshot invariants. */

#include <wpl/wpl.h>

#include "wpl_linux_x11_internal.h"

#include <assert.h>
#include <stdbool.h>

static void
wpl_test_assert_snapshot_zeroed(const WplInputState* input)
{
  int button_index;
  int key_index;

  assert(input->mouse.position.x == 0.0f);
  assert(input->mouse.position.y == 0.0f);
  assert(input->mouse.delta.x == 0.0f);
  assert(input->mouse.delta.y == 0.0f);
  assert(input->mouse.wheel_delta == 0.0f);

  for (button_index = 0; button_index < WPL_MOUSE_BUTTON_COUNT; ++button_index)
    {
      assert(input->mouse.button_down[button_index] == false);
      assert(input->mouse.button_pressed[button_index] == false);
      assert(input->mouse.button_released[button_index] == false);
    }

  for (key_index = 0; key_index < WPL_KEY_COUNT; ++key_index)
    {
      assert(input->keyboard.key_down[key_index] == false);
      assert(input->keyboard.key_pressed[key_index] == false);
      assert(input->keyboard.key_released[key_index] == false);
    }

  assert(input->keyboard.shift_down == false);
  assert(input->keyboard.ctrl_down == false);
  assert(input->keyboard.alt_down == false);
}

static void
test_null_window_input_is_zeroed(void)
{
  WplInputState input = wpl_get_input(NULL);

  wpl_test_assert_snapshot_zeroed(&input);
}

static void
test_public_input_enum_contracts(void)
{
  assert(WPL_MOUSE_BUTTON_LEFT == 0);
  assert(WPL_MOUSE_BUTTON_RIGHT == 1);
  assert(WPL_MOUSE_BUTTON_MIDDLE == 2);
  assert(WPL_MOUSE_BUTTON_COUNT == 3);

  assert(WPL_KEY_UNKNOWN == 0);
  assert(WPL_KEY_COUNT == 37);
}

static void
test_zero_initialized_snapshot_has_no_transitions(void)
{
  WplInputState input = {0};

  wpl_test_assert_snapshot_zeroed(&input);
}

static void
test_reset_transients_preserves_down_state(void)
{
  WplInputState input = {0};

  input.mouse.delta.x = 4.0f;
  input.mouse.delta.y = -2.0f;
  input.mouse.wheel_delta = 3.0f;
  input.mouse.button_down[WPL_MOUSE_BUTTON_LEFT] = true;
  input.mouse.button_pressed[WPL_MOUSE_BUTTON_LEFT] = true;
  input.mouse.button_released[WPL_MOUSE_BUTTON_RIGHT] = true;
  input.keyboard.key_down[WPL_KEY_A] = true;
  input.keyboard.key_pressed[WPL_KEY_A] = true;
  input.keyboard.key_released[WPL_KEY_SPACE] = true;
  input.keyboard.shift_down = true;

  wpl_linux_x11_input_reset_transients(&input);

  assert(input.mouse.delta.x == 0.0f);
  assert(input.mouse.delta.y == 0.0f);
  assert(input.mouse.wheel_delta == 0.0f);
  assert(input.mouse.button_down[WPL_MOUSE_BUTTON_LEFT] == true);
  assert(input.mouse.button_pressed[WPL_MOUSE_BUTTON_LEFT] == false);
  assert(input.mouse.button_released[WPL_MOUSE_BUTTON_RIGHT] == false);
  assert(input.keyboard.key_down[WPL_KEY_A] == true);
  assert(input.keyboard.key_pressed[WPL_KEY_A] == false);
  assert(input.keyboard.key_released[WPL_KEY_SPACE] == false);
  assert(input.keyboard.shift_down == true);
}

static void
test_mouse_button_press_release_transition(void)
{
  WplInputState input = {0};

  wpl_linux_x11_input_press_mouse_button(&input, WPL_MOUSE_BUTTON_LEFT);
  assert(input.mouse.button_down[WPL_MOUSE_BUTTON_LEFT] == true);
  assert(input.mouse.button_pressed[WPL_MOUSE_BUTTON_LEFT] == true);
  assert(input.mouse.button_released[WPL_MOUSE_BUTTON_LEFT] == false);

  wpl_linux_x11_input_reset_transients(&input);
  assert(input.mouse.button_down[WPL_MOUSE_BUTTON_LEFT] == true);
  assert(input.mouse.button_pressed[WPL_MOUSE_BUTTON_LEFT] == false);

  wpl_linux_x11_input_release_mouse_button(&input, WPL_MOUSE_BUTTON_LEFT);
  assert(input.mouse.button_down[WPL_MOUSE_BUTTON_LEFT] == false);
  assert(input.mouse.button_released[WPL_MOUSE_BUTTON_LEFT] == true);
}

static void
test_key_press_release_transition_and_repeat_press(void)
{
  WplInputState input = {0};

  wpl_linux_x11_input_press_key(&input, WPL_KEY_A);
  assert(input.keyboard.key_down[WPL_KEY_A] == true);
  assert(input.keyboard.key_pressed[WPL_KEY_A] == true);

  wpl_linux_x11_input_reset_transients(&input);
  wpl_linux_x11_input_press_key(&input, WPL_KEY_A);
  assert(input.keyboard.key_down[WPL_KEY_A] == true);
  assert(input.keyboard.key_pressed[WPL_KEY_A] == false);

  wpl_linux_x11_input_release_key(&input, WPL_KEY_A);
  assert(input.keyboard.key_down[WPL_KEY_A] == false);
  assert(input.keyboard.key_released[WPL_KEY_A] == true);
}

static void
test_wheel_delta_accumulates_then_resets(void)
{
  WplInputState input = {0};

  input.mouse.wheel_delta += 1.0f;
  input.mouse.wheel_delta -= 2.0f;
  assert(input.mouse.wheel_delta == -1.0f);

  wpl_linux_x11_input_reset_transients(&input);
  assert(input.mouse.wheel_delta == 0.0f);
}

static void
test_clear_down_state_preserves_transients_but_clears_down_and_modifiers(void)
{
  WplInputState input = {0};

  input.mouse.button_down[WPL_MOUSE_BUTTON_LEFT] = true;
  input.mouse.button_pressed[WPL_MOUSE_BUTTON_LEFT] = true;
  input.keyboard.key_down[WPL_KEY_SPACE] = true;
  input.keyboard.key_pressed[WPL_KEY_SPACE] = true;
  input.keyboard.shift_down = true;
  input.keyboard.ctrl_down = true;
  input.keyboard.alt_down = true;

  wpl_linux_x11_input_clear_down_state(&input);

  assert(input.mouse.button_down[WPL_MOUSE_BUTTON_LEFT] == false);
  assert(input.mouse.button_pressed[WPL_MOUSE_BUTTON_LEFT] == true);
  assert(input.keyboard.key_down[WPL_KEY_SPACE] == false);
  assert(input.keyboard.key_pressed[WPL_KEY_SPACE] == true);
  assert(input.keyboard.shift_down == false);
  assert(input.keyboard.ctrl_down == false);
  assert(input.keyboard.alt_down == false);
}

int
main(void)
{
  test_null_window_input_is_zeroed();
  test_public_input_enum_contracts();
  test_zero_initialized_snapshot_has_no_transitions();
  test_reset_transients_preserves_down_state();
  test_mouse_button_press_release_transition();
  test_key_press_release_transition_and_repeat_press();
  test_wheel_delta_accumulates_then_resets();
  test_clear_down_state_preserves_transients_but_clears_down_and_modifiers();
  return 0;
}
