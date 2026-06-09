/* wpl_file.c - binary file I/O API stubs. */

#include "wpl/wpl_file.h"

WplResult
wpl_read_entire_file(const char* path, WplFileData* out_data)
{
  (void)path;

  if (out_data == NULL)
    return WPL_RESULT_INVALID_ARGUMENT;

  out_data->data = NULL;
  out_data->size = 0u;

  if (path == NULL)
    return WPL_RESULT_INVALID_ARGUMENT;

  return WPL_RESULT_UNSUPPORTED;
}

WplResult
wpl_write_entire_file(const char* path, const void* data, size_t size)
{
  (void)data;
  (void)size;

  if (path == NULL)
    return WPL_RESULT_INVALID_ARGUMENT;

  return WPL_RESULT_UNSUPPORTED;
}

void
wpl_free_file_data(WplFileData* data)
{
  if (data == NULL)
    return;

  data->data = NULL;
  data->size = 0u;
}
