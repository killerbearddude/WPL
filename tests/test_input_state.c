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
test_mouse_press_release_same_frame_records_both_transitions(void)
{
  WplInputState input = {0};

  wpl_linux_x11_input_press_mouse_button(&input, WPL_MOUSE_BUTTON_RIGHT);
  wpl_linux_x11_input_release_mouse_button(&input, WPL_MOUSE_BUTTON_RIGHT);

  assert(input.mouse.button_down[WPL_MOUSE_BUTTON_RIGHT] == false);
  assert(input.mouse.button_pressed[WPL_MOUSE_BUTTON_RIGHT] == true);
  assert(input.mouse.button_released[WPL_MOUSE_BUTTON_RIGHT] == true);
}

static void
test_redundant_mouse_release_does_not_emit_transition(void)
{
  WplInputState input = {0};

  wpl_linux_x11_input_release_mouse_button(&input, WPL_MOUSE_BUTTON_MIDDLE);

  assert(input.mouse.button_down[WPL_MOUSE_BUTTON_MIDDLE] == false);
  assert(input.mouse.button_pressed[WPL_MOUSE_BUTTON_MIDDLE] == false);
  assert(input.mouse.button_released[WPL_MOUSE_BUTTON_MIDDLE] == false);
}

static void
test_invalid_mouse_buttons_are_ignored(void)
{
  WplInputState input = {0};

  wpl_linux_x11_input_press_mouse_button(&input, (WplMouseButton)-1);
  wpl_linux_x11_input_press_mouse_button(&input, WPL_MOUSE_BUTTON_COUNT);
  wpl_linux_x11_input_release_mouse_button(&input, (WplMouseButton)-1);
  wpl_linux_x11_input_release_mouse_button(&input, WPL_MOUSE_BUTTON_COUNT);

  wpl_test_assert_snapshot_zeroed(&input);
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
test_key_press_release_same_frame_records_both_transitions(void)
{
  WplInputState input = {0};

  wpl_linux_x11_input_press_key(&input, WPL_KEY_SPACE);
  wpl_linux_x11_input_release_key(&input, WPL_KEY_SPACE);

  assert(input.keyboard.key_down[WPL_KEY_SPACE] == false);
  assert(input.keyboard.key_pressed[WPL_KEY_SPACE] == true);
  assert(input.keyboard.key_released[WPL_KEY_SPACE] == true);
}

static void
test_redundant_key_release_does_not_emit_transition(void)
{
  WplInputState input = {0};

  wpl_linux_x11_input_release_key(&input, WPL_KEY_TAB);

  assert(input.keyboard.key_down[WPL_KEY_TAB] == false);
  assert(input.keyboard.key_pressed[WPL_KEY_TAB] == false);
  assert(input.keyboard.key_released[WPL_KEY_TAB] == false);
}

static void
test_unknown_and_invalid_keys_are_ignored(void)
{
  WplInputState input = {0};

  wpl_linux_x11_input_press_key(&input, WPL_KEY_UNKNOWN);
  wpl_linux_x11_input_press_key(&input, WPL_KEY_COUNT);
  wpl_linux_x11_input_release_key(&input, WPL_KEY_UNKNOWN);
  wpl_linux_x11_input_release_key(&input, WPL_KEY_COUNT);

  wpl_test_assert_snapshot_zeroed(&input);
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

static void
test_get_input_returns_by_value_snapshot(void)
{
  WplWindow window = {0};
  WplInputState input;

  window.input.keyboard.key_down[WPL_KEY_A] = true;
  window.input.keyboard.key_pressed[WPL_KEY_A] = true;
  window.input.mouse.button_down[WPL_MOUSE_BUTTON_LEFT] = true;
  window.input.mouse.position.x = 12.0f;
  window.input.mouse.position.y = 34.0f;

  input = wpl_get_input(&window);
  assert(input.keyboard.key_down[WPL_KEY_A] == true);
  assert(input.keyboard.key_pressed[WPL_KEY_A] == true);
  assert(input.mouse.button_down[WPL_MOUSE_BUTTON_LEFT] == true);
  assert(input.mouse.position.x == 12.0f);
  assert(input.mouse.position.y == 34.0f);

  input.keyboard.key_down[WPL_KEY_A] = false;
  input.keyboard.key_pressed[WPL_KEY_A] = false;
  input.mouse.button_down[WPL_MOUSE_BUTTON_LEFT] = false;
  input.mouse.position.x = 0.0f;
  input.mouse.position.y = 0.0f;

  assert(window.input.keyboard.key_down[WPL_KEY_A] == true);
  assert(window.input.keyboard.key_pressed[WPL_KEY_A] == true);
  assert(window.input.mouse.button_down[WPL_MOUSE_BUTTON_LEFT] == true);
  assert(window.input.mouse.position.x == 12.0f);
  assert(window.input.mouse.position.y == 34.0f);
}

static void
test_first_motion_initializes_position_without_delta(void)
{
  WplWindow window = {0};
  XMotionEvent event = {0};

  event.x = 120;
  event.y = 80;

  wpl_linux_x11_handle_motion(&window, &event);

  assert(window.mouse_position_initialized == true);
  assert(window.input.mouse.position.x == 120.0f);
  assert(window.input.mouse.position.y == 80.0f);
  assert(window.input.mouse.delta.x == 0.0f);
  assert(window.input.mouse.delta.y == 0.0f);
}

static void
test_subsequent_motion_accumulates_delta(void)
{
  WplWindow window = {0};
  XMotionEvent event = {0};

  event.x = 10;
  event.y = 20;
  wpl_linux_x11_handle_motion(&window, &event);

  event.x = 14;
  event.y = 13;
  wpl_linux_x11_handle_motion(&window, &event);

  assert(window.input.mouse.position.x == 14.0f);
  assert(window.input.mouse.position.y == 13.0f);
  assert(window.input.mouse.delta.x == 4.0f);
  assert(window.input.mouse.delta.y == -7.0f);
}

static void
test_multiple_motion_events_accumulate_delta_in_one_frame(void)
{
  WplWindow window = {0};
  XMotionEvent event = {0};

  event.x = 1;
  event.y = 2;
  wpl_linux_x11_handle_motion(&window, &event);

  event.x = 5;
  event.y = 8;
  wpl_linux_x11_handle_motion(&window, &event);

  event.x = -2;
  event.y = 13;
  wpl_linux_x11_handle_motion(&window, &event);

  assert(window.input.mouse.position.x == -2.0f);
  assert(window.input.mouse.position.y == 13.0f);
  assert(window.input.mouse.delta.x == -3.0f);
  assert(window.input.mouse.delta.y == 11.0f);
}

static void
test_button_events_update_position_without_delta(void)
{
  WplWindow window = {0};
  XButtonEvent event = {0};

  event.x = 30;
  event.y = 40;
  event.button = Button1;
  wpl_linux_x11_handle_button_press(&window, &event);

  assert(window.mouse_position_initialized == true);
  assert(window.input.mouse.position.x == 30.0f);
  assert(window.input.mouse.position.y == 40.0f);
  assert(window.input.mouse.delta.x == 0.0f);
  assert(window.input.mouse.delta.y == 0.0f);
  assert(window.input.mouse.button_down[WPL_MOUSE_BUTTON_LEFT] == true);
  assert(window.input.mouse.button_pressed[WPL_MOUSE_BUTTON_LEFT] == true);

  wpl_linux_x11_input_reset_transients(&window.input);

  event.x = 35;
  event.y = 50;
  event.button = Button1;
  wpl_linux_x11_handle_button_release(&window, &event);

  assert(window.input.mouse.position.x == 35.0f);
  assert(window.input.mouse.position.y == 50.0f);
  assert(window.input.mouse.delta.x == 0.0f);
  assert(window.input.mouse.delta.y == 0.0f);
  assert(window.input.mouse.button_down[WPL_MOUSE_BUTTON_LEFT] == false);
  assert(window.input.mouse.button_released[WPL_MOUSE_BUTTON_LEFT] == true);
}

static void
test_wheel_button_updates_position_and_wheel_without_delta(void)
{
  WplWindow window = {0};
  XButtonEvent event = {0};

  event.x = 9;
  event.y = 11;
  event.button = Button4;

  wpl_linux_x11_handle_button_press(&window, &event);

  assert(window.mouse_position_initialized == true);
  assert(window.input.mouse.position.x == 9.0f);
  assert(window.input.mouse.position.y == 11.0f);
  assert(window.input.mouse.delta.x == 0.0f);
  assert(window.input.mouse.delta.y == 0.0f);
  assert(window.input.mouse.wheel_delta == 1.0f);
}

static void
test_wheel_up_down_accumulates_and_horizontal_wheel_is_ignored(void)
{
  WplWindow window = {0};
  XButtonEvent event = {0};

  event.x = 1;
  event.y = 2;
  event.button = Button4;
  wpl_linux_x11_handle_button_press(&window, &event);

  event.x = 3;
  event.y = 4;
  event.button = Button5;
  wpl_linux_x11_handle_button_press(&window, &event);

  event.x = 5;
  event.y = 6;
  event.button = 6u;
  wpl_linux_x11_handle_button_press(&window, &event);

  event.button = 7u;
  wpl_linux_x11_handle_button_press(&window, &event);

  assert(window.input.mouse.position.x == 5.0f);
  assert(window.input.mouse.position.y == 6.0f);
  assert(window.input.mouse.delta.x == 0.0f);
  assert(window.input.mouse.delta.y == 0.0f);
  assert(window.input.mouse.wheel_delta == 0.0f);
}

static void
test_wheel_release_events_do_not_change_wheel_or_buttons(void)
{
  WplWindow window = {0};
  XButtonEvent event = {0};

  window.input.mouse.wheel_delta = 2.0f;

  event.x = 10;
  event.y = 20;
  event.button = Button4;
  wpl_linux_x11_handle_button_release(&window, &event);

  event.button = Button5;
  wpl_linux_x11_handle_button_release(&window, &event);

  assert(window.input.mouse.position.x == 10.0f);
  assert(window.input.mouse.position.y == 20.0f);
  assert(window.input.mouse.wheel_delta == 2.0f);
  assert(window.input.mouse.button_down[WPL_MOUSE_BUTTON_LEFT] == false);
  assert(window.input.mouse.button_down[WPL_MOUSE_BUTTON_RIGHT] == false);
  assert(window.input.mouse.button_down[WPL_MOUSE_BUTTON_MIDDLE] == false);
}

static void
test_enter_initializes_position_without_delta(void)
{
  WplWindow window = {0};
  XCrossingEvent event = {0};

  event.x = 70;
  event.y = 90;
  event.state = ShiftMask | ControlMask;

  wpl_linux_x11_handle_enter(&window, &event);

  assert(window.mouse_position_initialized == true);
  assert(window.input.mouse.position.x == 70.0f);
  assert(window.input.mouse.position.y == 90.0f);
  assert(window.input.mouse.delta.x == 0.0f);
  assert(window.input.mouse.delta.y == 0.0f);
  assert(window.input.keyboard.shift_down == true);
  assert(window.input.keyboard.ctrl_down == true);
  assert(window.input.keyboard.alt_down == false);
}

static void
test_leave_reenter_suppresses_artificial_delta(void)
{
  WplWindow window = {0};
  XMotionEvent motion = {0};
  XCrossingEvent crossing = {0};

  motion.x = 10;
  motion.y = 20;
  wpl_linux_x11_handle_motion(&window, &motion);

  motion.x = 15;
  motion.y = 25;
  wpl_linux_x11_handle_motion(&window, &motion);
  assert(window.input.mouse.delta.x == 5.0f);
  assert(window.input.mouse.delta.y == 5.0f);

  wpl_linux_x11_input_reset_transients(&window.input);

  crossing.x = 16;
  crossing.y = 26;
  crossing.state = Mod1Mask;
  wpl_linux_x11_handle_leave(&window, &crossing);
  assert(window.mouse_position_initialized == false);
  assert(window.input.keyboard.shift_down == false);
  assert(window.input.keyboard.ctrl_down == false);
  assert(window.input.keyboard.alt_down == true);

  crossing.x = 100;
  crossing.y = 120;
  crossing.state = 0u;
  wpl_linux_x11_handle_enter(&window, &crossing);
  assert(window.mouse_position_initialized == true);
  assert(window.input.mouse.position.x == 100.0f);
  assert(window.input.mouse.position.y == 120.0f);
  assert(window.input.mouse.delta.x == 0.0f);
  assert(window.input.mouse.delta.y == 0.0f);
  assert(window.input.keyboard.alt_down == false);

  motion.x = 103;
  motion.y = 118;
  wpl_linux_x11_handle_motion(&window, &motion);
  assert(window.input.mouse.position.x == 103.0f);
  assert(window.input.mouse.position.y == 118.0f);
  assert(window.input.mouse.delta.x == 3.0f);
  assert(window.input.mouse.delta.y == -2.0f);
}

int
main(void)
{
  test_null_window_input_is_zeroed();
  test_public_input_enum_contracts();
  test_zero_initialized_snapshot_has_no_transitions();
  test_reset_transients_preserves_down_state();
  test_mouse_button_press_release_transition();
  test_mouse_press_release_same_frame_records_both_transitions();
  test_redundant_mouse_release_does_not_emit_transition();
  test_invalid_mouse_buttons_are_ignored();
  test_key_press_release_transition_and_repeat_press();
  test_key_press_release_same_frame_records_both_transitions();
  test_redundant_key_release_does_not_emit_transition();
  test_unknown_and_invalid_keys_are_ignored();
  test_wheel_delta_accumulates_then_resets();
  test_clear_down_state_preserves_transients_but_clears_down_and_modifiers();
  test_get_input_returns_by_value_snapshot();
  test_first_motion_initializes_position_without_delta();
  test_subsequent_motion_accumulates_delta();
  test_multiple_motion_events_accumulate_delta_in_one_frame();
  test_button_events_update_position_without_delta();
  test_wheel_button_updates_position_and_wheel_without_delta();
  test_wheel_up_down_accumulates_and_horizontal_wheel_is_ignored();
  test_wheel_release_events_do_not_change_wheel_or_buttons();
  test_enter_initializes_position_without_delta();
  test_leave_reenter_suppresses_artificial_delta();
  return 0;
}
