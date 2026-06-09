/* main.c - Print frame-stable input snapshot transitions. */

#define _POSIX_C_SOURCE 199309L

#include <wpl/wpl.h>

#include <stdio.h>
#include <time.h>

static const char*
wpl_example_key_name(WplKey key)
{
  switch (key)
    {
    case WPL_KEY_ESCAPE:
      return "Escape";
    case WPL_KEY_DELETE:
      return "Delete";
    case WPL_KEY_BACKSPACE:
      return "Backspace";
    case WPL_KEY_ENTER:
      return "Enter";
    case WPL_KEY_TAB:
      return "Tab";
    case WPL_KEY_SPACE:
      return "Space";
    case WPL_KEY_LEFT:
      return "Left";
    case WPL_KEY_RIGHT:
      return "Right";
    case WPL_KEY_UP:
      return "Up";
    case WPL_KEY_DOWN:
      return "Down";
    case WPL_KEY_A:
      return "A";
    case WPL_KEY_B:
      return "B";
    case WPL_KEY_C:
      return "C";
    case WPL_KEY_D:
      return "D";
    case WPL_KEY_E:
      return "E";
    case WPL_KEY_F:
      return "F";
    case WPL_KEY_G:
      return "G";
    case WPL_KEY_H:
      return "H";
    case WPL_KEY_I:
      return "I";
    case WPL_KEY_J:
      return "J";
    case WPL_KEY_K:
      return "K";
    case WPL_KEY_L:
      return "L";
    case WPL_KEY_M:
      return "M";
    case WPL_KEY_N:
      return "N";
    case WPL_KEY_O:
      return "O";
    case WPL_KEY_P:
      return "P";
    case WPL_KEY_Q:
      return "Q";
    case WPL_KEY_R:
      return "R";
    case WPL_KEY_S:
      return "S";
    case WPL_KEY_T:
      return "T";
    case WPL_KEY_U:
      return "U";
    case WPL_KEY_V:
      return "V";
    case WPL_KEY_W:
      return "W";
    case WPL_KEY_X:
      return "X";
    case WPL_KEY_Y:
      return "Y";
    case WPL_KEY_Z:
      return "Z";
    case WPL_KEY_UNKNOWN:
    case WPL_KEY_COUNT:
    default:
      return "Unknown";
    }
}

static const char*
wpl_example_mouse_button_name(WplMouseButton button)
{
  switch (button)
    {
    case WPL_MOUSE_BUTTON_LEFT:
      return "left";
    case WPL_MOUSE_BUTTON_RIGHT:
      return "right";
    case WPL_MOUSE_BUTTON_MIDDLE:
      return "middle";
    case WPL_MOUSE_BUTTON_COUNT:
    default:
      return "unknown";
    }
}

static void
wpl_example_print_key_transitions(const WplInputState* input)
{
  int key_index;

  for (key_index = WPL_KEY_ESCAPE; key_index < WPL_KEY_COUNT; ++key_index)
    {
      WplKey key = (WplKey)key_index;

      if (input->keyboard.key_pressed[key])
        printf("key pressed: %s\n", wpl_example_key_name(key));

      if (input->keyboard.key_released[key])
        printf("key released: %s\n", wpl_example_key_name(key));
    }
}

static void
wpl_example_print_mouse_transitions(const WplInputState* input)
{
  int button_index;

  for (button_index = 0; button_index < WPL_MOUSE_BUTTON_COUNT;
       ++button_index)
    {
      WplMouseButton button = (WplMouseButton)button_index;

      if (input->mouse.button_pressed[button])
        printf("mouse pressed: %s\n", wpl_example_mouse_button_name(button));

      if (input->mouse.button_released[button])
        printf("mouse released: %s\n", wpl_example_mouse_button_name(button));
    }

  if (input->mouse.wheel_delta != 0.0f)
    printf("wheel: %+0.1f\n", input->mouse.wheel_delta);

  if (input->mouse.delta.x != 0.0f || input->mouse.delta.y != 0.0f)
    printf("mouse: pos=(%.1f, %.1f) delta=(%.1f, %.1f)\n",
           input->mouse.position.x,
           input->mouse.position.y,
           input->mouse.delta.x,
           input->mouse.delta.y);
}

static void
wpl_example_print_modifier_changes(const WplInputState* input,
                                   WplInputState* previous)
{
  if (input->keyboard.shift_down != previous->keyboard.shift_down
      || input->keyboard.ctrl_down != previous->keyboard.ctrl_down
      || input->keyboard.alt_down != previous->keyboard.alt_down)
    {
      printf("modifiers: shift=%d ctrl=%d alt=%d\n",
             input->keyboard.shift_down ? 1 : 0,
             input->keyboard.ctrl_down ? 1 : 0,
             input->keyboard.alt_down ? 1 : 0);
    }

  previous->keyboard.shift_down = input->keyboard.shift_down;
  previous->keyboard.ctrl_down = input->keyboard.ctrl_down;
  previous->keyboard.alt_down = input->keyboard.alt_down;
}

int
main(void)
{
  WplWindowDesc desc = {
    .title = "WPL Input Snapshot",
    .width = 960,
    .height = 540,
    .resizable = true
  };
  WplWindow* window = NULL;
  WplInputState previous = {0};
  WplResult result;

  result = wpl_create_window(&desc, &window);
  if (result != WPL_RESULT_OK)
    return 1;

  while (!wpl_window_should_close(window))
    {
      WplInputState input;

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

      input = wpl_get_input(window);
      wpl_example_print_key_transitions(&input);
      wpl_example_print_mouse_transitions(&input);
      wpl_example_print_modifier_changes(&input, &previous);

      if (input.keyboard.key_pressed[WPL_KEY_ESCAPE])
        {
          printf("escape pressed, closing\n");
          result = wpl_window_request_close(window);
          if (result != WPL_RESULT_OK)
            {
              wpl_destroy_window(window);
              return 1;
            }
        }

      result = wpl_end_frame(window);
      if (result != WPL_RESULT_OK)
        {
          wpl_destroy_window(window);
          return 1;
        }

      fflush(stdout);
      {
        const struct timespec frame_pause = {0, 1000000L};
        nanosleep(&frame_pause, NULL);
      }
    }

  wpl_destroy_window(window);
  return 0;
}
