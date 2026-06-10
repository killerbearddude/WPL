/* wpl_file.c - backend-independent POSIX whole-file I/O. */

#define _POSIX_C_SOURCE 200809L

#include "wpl/wpl_file.h"

#include "wpl/wpl_base.h"

#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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


static WplResult
wpl_file_fsync_checked(int fd)
{
  int result = 0;

  do {
    result = fsync(fd);
  } while (result != 0 && errno == EINTR);

  if (result != 0)
    return WPL_RESULT_IO_ERROR;

  return WPL_RESULT_OK;
}

static WplResult
wpl_file_make_atomic_temp_path(const char* path, char** out_temp_path)
{
  const char* slash = NULL;
  const char* base = NULL;
  size_t dir_len = 0u;
  size_t base_len = 0u;
  size_t suffix_len = sizeof(".tmp.XXXXXX") - 1u;
  size_t total_len = 0u;
  char* temp_path = NULL;

  if (out_temp_path == NULL)
    return WPL_RESULT_INVALID_ARGUMENT;

  *out_temp_path = NULL;

  if (!wpl_file_path_is_valid(path))
    return WPL_RESULT_INVALID_ARGUMENT;

  slash = strrchr(path, '/');
  if (slash != NULL) {
    dir_len = (size_t)(slash - path) + 1u;
    base = slash + 1;
  } else {
    dir_len = 0u;
    base = path;
  }

  if (base == NULL || base[0] == '\0')
    return WPL_RESULT_INVALID_ARGUMENT;

  base_len = strlen(base);

  if (dir_len > SIZE_MAX - 1u ||
      dir_len + 1u > SIZE_MAX - base_len ||
      dir_len + 1u + base_len > SIZE_MAX - suffix_len ||
      dir_len + 1u + base_len + suffix_len > SIZE_MAX - 1u)
    return WPL_RESULT_UNSUPPORTED;

  total_len = dir_len + 1u + base_len + suffix_len + 1u;
  temp_path = (char*)malloc(total_len);
  if (temp_path == NULL)
    return WPL_RESULT_OUT_OF_MEMORY;

  if (dir_len > 0u)
    memcpy(temp_path, path, dir_len);

  temp_path[dir_len] = '.';
  memcpy(temp_path + dir_len + 1u, base, base_len);
  memcpy(temp_path + dir_len + 1u + base_len, ".tmp.XXXXXX", suffix_len + 1u);

  *out_temp_path = temp_path;
  return WPL_RESULT_OK;
}

static WplResult
wpl_file_make_parent_directory_path(const char* path, char** out_dir_path)
{
  const char* slash = NULL;
  size_t dir_len = 0u;
  char* dir_path = NULL;

  if (out_dir_path == NULL)
    return WPL_RESULT_INVALID_ARGUMENT;

  *out_dir_path = NULL;

  if (!wpl_file_path_is_valid(path))
    return WPL_RESULT_INVALID_ARGUMENT;

  slash = strrchr(path, '/');
  if (slash == NULL) {
    dir_path = (char*)malloc(2u);
    if (dir_path == NULL)
      return WPL_RESULT_OUT_OF_MEMORY;

    dir_path[0] = '.';
    dir_path[1] = '\0';
    *out_dir_path = dir_path;
    return WPL_RESULT_OK;
  }

  if (slash == path) {
    dir_path = (char*)malloc(2u);
    if (dir_path == NULL)
      return WPL_RESULT_OUT_OF_MEMORY;

    dir_path[0] = '/';
    dir_path[1] = '\0';
    *out_dir_path = dir_path;
    return WPL_RESULT_OK;
  }

  dir_len = (size_t)(slash - path);
  dir_path = (char*)malloc(dir_len + 1u);
  if (dir_path == NULL)
    return WPL_RESULT_OUT_OF_MEMORY;

  memcpy(dir_path, path, dir_len);
  dir_path[dir_len] = '\0';
  *out_dir_path = dir_path;
  return WPL_RESULT_OK;
}

static WplResult
wpl_file_open_parent_directory(const char* path, int* out_fd)
{
  char* dir_path = NULL;
  WplResult result = WPL_RESULT_OK;
  int flags = O_RDONLY;
  int fd = -1;

  if (out_fd == NULL)
    return WPL_RESULT_INVALID_ARGUMENT;

  *out_fd = -1;

  result = wpl_file_make_parent_directory_path(path, &dir_path);
  if (result != WPL_RESULT_OK)
    return result;

#ifdef O_DIRECTORY
  flags |= O_DIRECTORY;
#endif

  fd = wpl_file_open_retry(dir_path, flags, 0, false);
  free(dir_path);

  if (fd < 0)
    return WPL_RESULT_IO_ERROR;

  *out_fd = fd;
  return WPL_RESULT_OK;
}

static void
wpl_file_fsync_parent_directory_best_effort(const char* path)
{
  int fd = -1;

  if (wpl_file_open_parent_directory(path, &fd) != WPL_RESULT_OK)
    return;

  (void)wpl_file_fsync_checked(fd);
  (void)wpl_file_close_checked(fd);
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


WplResult
wpl_write_entire_file_atomic(const char* path, const void* data, size_t size)
{
  char* temp_path = NULL;
  int fd = -1;
  bool temp_created = false;
  WplResult result = WPL_RESULT_OK;

  if (!wpl_file_path_is_valid(path))
    return WPL_RESULT_INVALID_ARGUMENT;

  if (data == NULL && size > 0u)
    return WPL_RESULT_INVALID_ARGUMENT;

  if (size > (size_t)WPL_MAX_FILE_SIZE_V0_1)
    return WPL_RESULT_UNSUPPORTED;

  result = wpl_file_make_atomic_temp_path(path, &temp_path);
  if (result != WPL_RESULT_OK)
    return result;

  fd = mkstemp(temp_path);
  if (fd < 0) {
    free(temp_path);
    return WPL_RESULT_IO_ERROR;
  }
  temp_created = true;

  if (fchmod(fd, 0644) != 0) {
    result = WPL_RESULT_IO_ERROR;
    goto cleanup;
  }

  result = wpl_file_write_exact(fd, data, size);
  if (result != WPL_RESULT_OK)
    goto cleanup;

  result = wpl_file_fsync_checked(fd);
  if (result != WPL_RESULT_OK)
    goto cleanup;

  result = wpl_file_close_checked(fd);
  fd = -1;
  if (result != WPL_RESULT_OK)
    goto cleanup;

  if (rename(temp_path, path) != 0) {
    result = WPL_RESULT_IO_ERROR;
    goto cleanup;
  }
  temp_created = false;

  wpl_file_fsync_parent_directory_best_effort(path);

  free(temp_path);
  return WPL_RESULT_OK;

cleanup:
  if (fd >= 0)
    (void)wpl_file_close_checked(fd);

  if (temp_created)
    (void)unlink(temp_path);

  free(temp_path);
  return result;
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
