/* test_input_state.c - Public input snapshot invariants. */

#include <wpl/wpl.h>

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

int
main(void)
{
  test_null_window_input_is_zeroed();
  test_public_input_enum_contracts();
  test_zero_initialized_snapshot_has_no_transitions();
  return 0;
}
