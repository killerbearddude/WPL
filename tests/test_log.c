/* test_log.c - Public logging callback behavior. */

#include <wpl/wpl.h>

#include <assert.h>
#include <stdbool.h>
#include <string.h>

typedef struct WplTestLogCapture {
  bool called;
  WplLogLevel level;
  const char* message;
  void* user_data;
} WplTestLogCapture;

static void
wpl_test_log_callback(WplLogLevel level, const char* message, void* user_data)
{
  WplTestLogCapture* capture = (WplTestLogCapture*)user_data;

  assert(capture != 0);
  capture->called = true;
  capture->level = level;
  capture->message = message;
  capture->user_data = user_data;
}

static void
test_set_callback_then_log_invokes_callback(void)
{
  WplTestLogCapture capture = {0};

  wpl_set_log_callback(wpl_test_log_callback, &capture);
  wpl_log(WPL_LOG_WARNING, "hello");

  assert(capture.called == true);
  assert(capture.level == WPL_LOG_WARNING);
  assert(strcmp(capture.message, "hello") == 0);
  assert(capture.user_data == &capture);

  wpl_set_log_callback(0, 0);
}

static void
test_null_callback_disables_logging(void)
{
  WplTestLogCapture capture = {0};

  wpl_set_log_callback(wpl_test_log_callback, &capture);
  wpl_set_log_callback(0, &capture);
  wpl_log(WPL_LOG_ERROR, "ignored");

  assert(capture.called == false);
}

static void
test_null_message_is_safe(void)
{
  WplTestLogCapture capture = {0};

  wpl_set_log_callback(wpl_test_log_callback, &capture);
  wpl_log(WPL_LOG_INFO, 0);

  assert(capture.called == true);
  assert(capture.level == WPL_LOG_INFO);
  assert(capture.message != 0);
  assert(strcmp(capture.message, "") == 0);

  wpl_set_log_callback(0, 0);
}

int
main(void)
{
  test_set_callback_then_log_invokes_callback();
  test_null_callback_disables_logging();
  test_null_message_is_safe();
  return 0;
}
