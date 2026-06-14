#define _POSIX_C_SOURCE 200809L

#include <wpl/wpl.h>

#include <assert.h>
#include <dirent.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static bool
wpl_test_make_temp_file(char* path, size_t path_size, int* out_fd)
{
  const char pattern[] = "/tmp/wpl_file_io_edge_test_XXXXXX";
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
  remove(path);
  return true;
}

static bool
wpl_test_make_temp_dir(char* path, size_t path_size)
{
  const char pattern[] = "/tmp/wpl_file_io_edge_dir_XXXXXX";

  if (path == NULL || path_size < sizeof(pattern))
    return false;

  memcpy(path, pattern, sizeof(pattern));
  return mkdtemp(path) != NULL;
}

static bool
wpl_test_join_path(char* out_path,
                   size_t out_path_size,
                   const char* dir,
                   const char* file)
{
  int count = 0;

  if (out_path == NULL || dir == NULL || file == NULL)
    return false;

  count = snprintf(out_path, out_path_size, "%s/%s", dir, file);
  return count > 0 && (size_t)count < out_path_size;
}

static bool
wpl_test_directory_is_empty(const char* path)
{
  DIR* dir = opendir(path);
  struct dirent* entry = NULL;

  if (dir == NULL)
    return false;

  while ((entry = readdir(dir)) != NULL) {
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
      continue;

    closedir(dir);
    return false;
  }

  closedir(dir);
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
test_read_directory_rejected_and_resets_output(void)
{
  char dir[sizeof("/tmp/wpl_file_io_edge_dir_XXXXXX")] = {0};
  WplFileData data = {(void*)1, 99u};

  assert(wpl_test_make_temp_dir(dir, sizeof(dir)));

  assert(wpl_read_entire_file(dir, &data) == WPL_RESULT_IO_ERROR);
  assert(data.data == NULL);
  assert(data.size == 0u);

  rmdir(dir);
}

static void
test_write_null_zero_truncates_existing_file(void)
{
  static const unsigned char first[] = {1u, 2u, 3u, 4u, 5u};
  char path[sizeof("/tmp/wpl_file_io_edge_test_XXXXXX")] = {0};
  WplFileData data = {0};

  assert(wpl_test_make_temp_path(path, sizeof(path)));

  assert(wpl_write_entire_file(path, first, sizeof(first)) == WPL_RESULT_OK);
  assert(wpl_write_entire_file(path, NULL, 0u) == WPL_RESULT_OK);
  assert(wpl_read_entire_file(path, &data) == WPL_RESULT_OK);
  assert(data.data == NULL);
  assert(data.size == 0u);

  wpl_free_file_data(&data);
  remove(path);
}

static void
test_atomic_write_null_zero_replaces_existing_file(void)
{
  static const unsigned char first[] = {1u, 2u, 3u, 4u, 5u};
  char dir[sizeof("/tmp/wpl_file_io_edge_dir_XXXXXX")] = {0};
  char path[512] = {0};
  WplFileData data = {0};

  assert(wpl_test_make_temp_dir(dir, sizeof(dir)));
  assert(wpl_test_join_path(path, sizeof(path), dir, "replace-empty.bin"));

  assert(wpl_write_entire_file_atomic(path, first, sizeof(first)) ==
         WPL_RESULT_OK);
  assert(wpl_write_entire_file_atomic(path, NULL, 0u) == WPL_RESULT_OK);
  assert(wpl_read_entire_file(path, &data) == WPL_RESULT_OK);
  assert(data.data == NULL);
  assert(data.size == 0u);

  wpl_free_file_data(&data);
  remove(path);
  rmdir(dir);
}

static void
test_atomic_write_trailing_slash_rejected_without_temp_file(void)
{
  unsigned char byte = 0u;
  char dir[sizeof("/tmp/wpl_file_io_edge_dir_XXXXXX")] = {0};
  char path[512] = {0};

  assert(wpl_test_make_temp_dir(dir, sizeof(dir)));
  assert(wpl_test_join_path(path, sizeof(path), dir, ""));

  assert(wpl_write_entire_file_atomic(path, &byte, 1u) ==
         WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_test_directory_is_empty(dir));

  rmdir(dir);
}

static void
test_free_null_data_with_nonzero_size_resets_size(void)
{
  WplFileData data;

  data.data = NULL;
  data.size = 99u;

  wpl_free_file_data(&data);
  assert(data.data == NULL);
  assert(data.size == 0u);
}

int
main(void)
{
  test_read_directory_rejected_and_resets_output();
  test_write_null_zero_truncates_existing_file();
  test_atomic_write_null_zero_replaces_existing_file();
  test_atomic_write_trailing_slash_rejected_without_temp_file();
  test_free_null_data_with_nonzero_size_resets_size();

  return 0;
}
