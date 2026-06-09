#define _POSIX_C_SOURCE 200809L

#include <wpl/wpl.h>

#include <assert.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

static bool
wpl_test_make_temp_file(char* path, size_t path_size, int* out_fd)
{
  const char pattern[] = "/tmp/wpl_file_io_test_XXXXXX";
  int fd = -1;

  if (path == NULL || out_fd == NULL || path_size < sizeof(pattern))
    return false;

  memcpy(path, pattern, sizeof(pattern));
  fd = mkstemp(path);
  if (fd < 0)
    return false;

  *out_fd = fd;
  return true;
}

static bool
wpl_test_make_temp_path(char* path, size_t path_size)
{
  int fd = -1;

  if (!wpl_test_make_temp_file(path, path_size, &fd))
    return false;

  close(fd);
  unlink(path);
  return true;
}

static void
wpl_test_close_fd(int* fd)
{
  if (fd != NULL && *fd >= 0) {
    close(*fd);
    *fd = -1;
  }
}

static void
test_read_invalid_arguments(void)
{
  WplFileData data = {(void*)1, 99u};

  assert(wpl_read_entire_file(NULL, &data) == WPL_RESULT_INVALID_ARGUMENT);
  assert(data.data == NULL);
  assert(data.size == 0u);

  data.data = (void*)1;
  data.size = 99u;
  assert(wpl_read_entire_file("", &data) == WPL_RESULT_INVALID_ARGUMENT);
  assert(data.data == NULL);
  assert(data.size == 0u);

  assert(wpl_read_entire_file("/tmp/wpl_missing_file", NULL) ==
         WPL_RESULT_INVALID_ARGUMENT);
}

static void
test_read_missing_file(void)
{
  WplFileData data = {(void*)1, 99u};

  assert(wpl_read_entire_file("/tmp/wpl_file_io_missing_expected", &data) ==
         WPL_RESULT_IO_ERROR);
  assert(data.data == NULL);
  assert(data.size == 0u);
}

static void
test_read_empty_file(void)
{
  char path[] = "/tmp/wpl_file_io_test_XXXXXX";
  int fd = -1;
  WplFileData data = {0};

  assert(wpl_test_make_temp_file(path, sizeof(path), &fd));
  wpl_test_close_fd(&fd);

  assert(wpl_read_entire_file(path, &data) == WPL_RESULT_OK);
  assert(data.data == NULL);
  assert(data.size == 0u);

  wpl_free_file_data(&data);
  unlink(path);
}

static void
test_read_binary_file_with_nul_bytes(void)
{
  static const unsigned char expected[] = {0x00u, 0x01u, 0x02u, 0x7fu,
                                           0x80u, 0xffu, 0x00u};
  char path[] = "/tmp/wpl_file_io_test_XXXXXX";
  int fd = -1;
  WplFileData data = {0};

  assert(wpl_test_make_temp_file(path, sizeof(path), &fd));
  assert(write(fd, expected, sizeof(expected)) == (ssize_t)sizeof(expected));
  wpl_test_close_fd(&fd);

  assert(wpl_read_entire_file(path, &data) == WPL_RESULT_OK);
  assert(data.size == sizeof(expected));
  assert(data.data != NULL);
  assert(memcmp(data.data, expected, sizeof(expected)) == 0);

  wpl_free_file_data(&data);
  unlink(path);
}

static void
test_read_oversized_file_rejected(void)
{
  char path[] = "/tmp/wpl_file_io_test_XXXXXX";
  int fd = -1;
  WplFileData data = {(void*)1, 99u};

  assert(wpl_test_make_temp_file(path, sizeof(path), &fd));
  assert(ftruncate(fd, (off_t)WPL_MAX_FILE_SIZE_V0_1 + 1) == 0);
  wpl_test_close_fd(&fd);

  assert(wpl_read_entire_file(path, &data) == WPL_RESULT_UNSUPPORTED);
  assert(data.data == NULL);
  assert(data.size == 0u);

  wpl_free_file_data(&data);
  unlink(path);
}

static void
test_write_invalid_arguments(void)
{
  unsigned char byte = 0u;

  assert(wpl_write_entire_file(NULL, &byte, 1u) == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_write_entire_file("", &byte, 1u) == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_write_entire_file("/tmp/wpl_file_io_invalid", NULL, 1u) ==
         WPL_RESULT_INVALID_ARGUMENT);
}

static void
test_write_null_zero_succeeds_and_creates_file(void)
{
  char path[sizeof("/tmp/wpl_file_io_test_XXXXXX")] = {0};
  WplFileData data = {0};

  assert(wpl_test_make_temp_path(path, sizeof(path)));

  assert(wpl_write_entire_file(path, NULL, 0u) == WPL_RESULT_OK);
  assert(wpl_read_entire_file(path, &data) == WPL_RESULT_OK);
  assert(data.data == NULL);
  assert(data.size == 0u);

  wpl_free_file_data(&data);
  unlink(path);
}

static void
test_write_binary_data_succeeds(void)
{
  static const unsigned char expected[] = {0x00u, 0x01u, 0x80u, 0xffu, 0x00u};
  char path[sizeof("/tmp/wpl_file_io_test_XXXXXX")] = {0};
  WplFileData data = {0};

  assert(wpl_test_make_temp_path(path, sizeof(path)));

  assert(wpl_write_entire_file(path, expected, sizeof(expected)) ==
         WPL_RESULT_OK);
  assert(wpl_read_entire_file(path, &data) == WPL_RESULT_OK);
  assert(data.size == sizeof(expected));
  assert(data.data != NULL);
  assert(memcmp(data.data, expected, sizeof(expected)) == 0);

  wpl_free_file_data(&data);
  unlink(path);
}

static void
test_write_oversized_rejected(void)
{
  unsigned char byte = 0u;
  char path[sizeof("/tmp/wpl_file_io_test_XXXXXX")] = {0};

  assert(wpl_test_make_temp_path(path, sizeof(path)));

  assert(wpl_write_entire_file(path,
                               &byte,
                               (size_t)WPL_MAX_FILE_SIZE_V0_1 + 1u) ==
         WPL_RESULT_UNSUPPORTED);

  unlink(path);
}

static void
test_write_truncates_existing_file(void)
{
  static const unsigned char first[] = {1u, 2u, 3u, 4u, 5u};
  static const unsigned char second[] = {9u, 8u};
  char path[sizeof("/tmp/wpl_file_io_test_XXXXXX")] = {0};
  WplFileData data = {0};

  assert(wpl_test_make_temp_path(path, sizeof(path)));

  assert(wpl_write_entire_file(path, first, sizeof(first)) == WPL_RESULT_OK);
  assert(wpl_write_entire_file(path, second, sizeof(second)) == WPL_RESULT_OK);
  assert(wpl_read_entire_file(path, &data) == WPL_RESULT_OK);
  assert(data.size == sizeof(second));
  assert(memcmp(data.data, second, sizeof(second)) == 0);

  wpl_free_file_data(&data);
  unlink(path);
}

static void
test_free_null_and_reset(void)
{
  WplFileData data;

  wpl_free_file_data(NULL);

  data.data = malloc(4u);
  assert(data.data != NULL);
  data.size = 4u;

  wpl_free_file_data(&data);
  assert(data.data == NULL);
  assert(data.size == 0u);

  wpl_free_file_data(&data);
  assert(data.data == NULL);
  assert(data.size == 0u);
}

int
main(void)
{
  test_read_invalid_arguments();
  test_read_missing_file();
  test_read_empty_file();
  test_read_binary_file_with_nul_bytes();
  test_read_oversized_file_rejected();
  test_write_invalid_arguments();
  test_write_null_zero_succeeds_and_creates_file();
  test_write_binary_data_succeeds();
  test_write_oversized_rejected();
  test_write_truncates_existing_file();
  test_free_null_and_reset();

  return 0;
}
