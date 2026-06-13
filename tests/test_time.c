/* test_time.c - Timing and frame-delta invariants. */

#include <wpl/wpl.h>

#include "wpl_linux_x11_internal.h"

#include <assert.h>

static void
test_time_seconds_is_monotonic_under_normal_execution(void)
{
  double previous = wpl_time_seconds();
  int i;

  assert(previous > 0.0);

  for (i = 0; i < 1024; ++i)
    {
      double current = wpl_time_seconds();
      assert(current >= previous);
      previous = current;
    }
}

static void
test_null_window_timing_contract(void)
{
  assert(wpl_window_delta_time(NULL) == 0.0f);
  assert(wpl_begin_frame(NULL) == WPL_RESULT_INVALID_ARGUMENT);
}

static void
test_window_delta_time_is_per_window_state(void)
{
  WplWindow first = {0};
  WplWindow second = {0};

  first.delta_time = 0.125f;
  second.delta_time = 0.5f;

  assert(wpl_window_delta_time(&first) == 0.125f);
  assert(wpl_window_delta_time(&second) == 0.5f);

  first.delta_time = 0.25f;

  assert(wpl_window_delta_time(&first) == 0.25f);
  assert(wpl_window_delta_time(&second) == 0.5f);
}

static void
test_begin_frame_updates_delta_from_monotonic_clock(void)
{
  WplWindow window = {0};
  double now = wpl_time_seconds();

  assert(now > 0.0);

  window.last_time = now - 0.25;
  window.delta_time = 99.0f;

  assert(wpl_begin_frame(&window) == WPL_RESULT_OK);
  assert(window.last_time >= now);
  assert(window.delta_time >= 0.0f);
  assert(window.delta_time < 10.0f);
  assert(wpl_window_delta_time(&window) == window.delta_time);
}

static void
test_begin_frame_clamps_regressing_clock_to_zero_delta(void)
{
  WplWindow window = {0};
  double now = wpl_time_seconds();
  double future;

  assert(now > 0.0);

  future = now + 1000.0;
  window.last_time = future;
  window.delta_time = 1.0f;

  assert(wpl_begin_frame(&window) == WPL_RESULT_OK);
  assert(window.last_time > 0.0);
  assert(window.last_time < future);
  assert(window.delta_time == 0.0f);
}

int
main(void)
{
  test_time_seconds_is_monotonic_under_normal_execution();
  test_null_window_timing_contract();
  test_window_delta_time_is_per_window_state();
  test_begin_frame_updates_delta_from_monotonic_clock();
  test_begin_frame_clamps_regressing_clock_to_zero_delta();
  return 0;
}
