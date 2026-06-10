# File I/O

WPL v0.1 provides backend-independent whole-file binary read/write helpers.

## Scope

- Reads are binary.
- Writes are binary.
- The caller owns read buffers and releases them with `wpl_free_file_data`.
- Files larger than `WPL_MAX_FILE_SIZE_V0_1` return `WPL_RESULT_UNSUPPORTED`.
- Writes create or truncate files using mode `0644`, subject to the process umask.
- Symlinks are followed normally.
- `open`, `read`, and `write` retry on `EINTR`.
- Short reads and short writes are handled with explicit loops.
- Direct writes are not atomic. Use `wpl_write_entire_file_atomic` when a safer
  whole-file replacement primitive is needed.

## Non-goals

File I/O is not an asset pipeline, serialization layer, config system, virtual
file system, directory traversal API, file watcher, async I/O system, or replay
implementation.

## Atomic Whole-File Writes

`wpl_write_entire_file_atomic` writes binary data to a temporary file in the
target directory, flushes the temporary file where practical, closes it, and
renames it over the target path. The helper is intended for safer host
persistence workflows while keeping file formats and serialization policy above
WPL.

Properties:

- Binary data.
- Same 64 MiB cap as other WPL file writes.
- Temporary file is created in the target directory so POSIX `rename` can replace
  the target on the same filesystem.
- Temporary file cleanup is attempted on failure where practical.
- Final file mode is set to `0644`, subject to process permissions and platform
  behavior.
- Parent-directory sync is best-effort after rename.
- No graph serialization behavior.
- No file format behavior.

Limitations:

- This is not a transactional database.
- Existing file permissions, owner, and group are not preserved beyond the
  v0.1.x `0644` replacement-file policy.
- Replacing a symlink path replaces the symlink directory entry rather than
  writing through to the symlink target.
- Directory `fsync` support varies across filesystems, so parent-directory sync
  is best-effort.
