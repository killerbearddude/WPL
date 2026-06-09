/* wpl_file.c - backend-independent POSIX whole-file I/O. */

#define _POSIX_C_SOURCE 200809L

#include "wpl/wpl_file.h"

#include "wpl/wpl_base.h"

#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

static bool
wpl_file_path_is_valid(const char* path)
{
  return path != NULL && path[0] != '\0';
}

static int
wpl_file_open_retry(const char* path, int flags, mode_t mode, bool use_mode)
{
  int fd = -1;

  do {
    if (use_mode) {
      fd = open(path, flags, mode);
    } else {
      fd = open(path, flags);
    }
  } while (fd < 0 && errno == EINTR);

  return fd;
}

static WplResult
wpl_file_close_checked(int fd)
{
  if (close(fd) != 0)
    return WPL_RESULT_IO_ERROR;

  return WPL_RESULT_OK;
}

static WplResult
wpl_file_read_exact(int fd, void* data, size_t size)
{
  uint8_t* bytes = (uint8_t*)data;
  size_t total = 0u;

  while (total < size) {
    ssize_t n = read(fd, bytes + total, size - total);

    if (n < 0) {
      if (errno == EINTR)
        continue;

      return WPL_RESULT_IO_ERROR;
    }

    if (n == 0)
      return WPL_RESULT_IO_ERROR;

    total += (size_t)n;
  }

  return WPL_RESULT_OK;
}

static WplResult
wpl_file_write_exact(int fd, const void* data, size_t size)
{
  const uint8_t* bytes = (const uint8_t*)data;
  size_t total = 0u;

  while (total < size) {
    ssize_t n = write(fd, bytes + total, size - total);

    if (n < 0) {
      if (errno == EINTR)
        continue;

      return WPL_RESULT_IO_ERROR;
    }

    if (n == 0)
      return WPL_RESULT_IO_ERROR;

    total += (size_t)n;
  }

  return WPL_RESULT_OK;
}

WplResult
wpl_read_entire_file(const char* path, WplFileData* out_data)
{
  int fd = -1;
  struct stat st;
  size_t size = 0u;
  void* buffer = NULL;
  WplResult result = WPL_RESULT_OK;

  if (out_data != NULL) {
    out_data->data = NULL;
    out_data->size = 0u;
  }

  if (!wpl_file_path_is_valid(path) || out_data == NULL)
    return WPL_RESULT_INVALID_ARGUMENT;

  fd = wpl_file_open_retry(path, O_RDONLY, 0, false);
  if (fd < 0)
    return WPL_RESULT_IO_ERROR;

  if (fstat(fd, &st) != 0) {
    (void)wpl_file_close_checked(fd);
    return WPL_RESULT_IO_ERROR;
  }

  if (!S_ISREG(st.st_mode)) {
    (void)wpl_file_close_checked(fd);
    return WPL_RESULT_IO_ERROR;
  }

  if (st.st_size < 0) {
    (void)wpl_file_close_checked(fd);
    return WPL_RESULT_IO_ERROR;
  }

  if ((uint64_t)st.st_size > (uint64_t)WPL_MAX_FILE_SIZE_V0_1) {
    (void)wpl_file_close_checked(fd);
    return WPL_RESULT_UNSUPPORTED;
  }

  size = (size_t)st.st_size;
  if (size == 0u) {
    result = wpl_file_close_checked(fd);
    if (result != WPL_RESULT_OK)
      return result;

    return WPL_RESULT_OK;
  }

  buffer = malloc(size);
  if (buffer == NULL) {
    (void)wpl_file_close_checked(fd);
    return WPL_RESULT_OUT_OF_MEMORY;
  }

  result = wpl_file_read_exact(fd, buffer, size);
  if (result != WPL_RESULT_OK) {
    free(buffer);
    (void)wpl_file_close_checked(fd);
    return result;
  }

  result = wpl_file_close_checked(fd);
  if (result != WPL_RESULT_OK) {
    free(buffer);
    return result;
  }

  out_data->data = buffer;
  out_data->size = size;
  return WPL_RESULT_OK;
}

WplResult
wpl_write_entire_file(const char* path, const void* data, size_t size)
{
  int fd = -1;
  WplResult result = WPL_RESULT_OK;

  if (!wpl_file_path_is_valid(path))
    return WPL_RESULT_INVALID_ARGUMENT;

  if (data == NULL && size > 0u)
    return WPL_RESULT_INVALID_ARGUMENT;

  if (size > (size_t)WPL_MAX_FILE_SIZE_V0_1)
    return WPL_RESULT_UNSUPPORTED;

  fd = wpl_file_open_retry(path, O_WRONLY | O_CREAT | O_TRUNC, 0644, true);
  if (fd < 0)
    return WPL_RESULT_IO_ERROR;

  result = wpl_file_write_exact(fd, data, size);
  if (result != WPL_RESULT_OK) {
    (void)wpl_file_close_checked(fd);
    return result;
  }

  result = wpl_file_close_checked(fd);
  if (result != WPL_RESULT_OK)
    return result;

  return WPL_RESULT_OK;
}

void
wpl_free_file_data(WplFileData* data)
{
  if (data == NULL)
    return;

  free(data->data);
  data->data = NULL;
  data->size = 0u;
}
