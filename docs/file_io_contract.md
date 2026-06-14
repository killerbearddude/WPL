# WPL File I/O Contract

This document records the Phase 8 file I/O contract for WPL. It is a contract
document only. It does not change runtime behavior, public API, tests, or
application semantics.

WPL's file I/O helpers provide small Linux/POSIX whole-file operations for
platform-layer infrastructure, deterministic test data, replay files, and host
persistence helpers. They are not an asset pipeline, virtual filesystem, file
watcher, serialization framework, project format layer, package manager, or graph
storage system.

## Current File I/O Shape

The public file I/O API is value-based C API surface:

```c
typedef struct WplFileData {
  void* data;
  size_t size;
} WplFileData;

WplResult wpl_read_entire_file(const char* path, WplFileData* out_data);

WplResult wpl_write_entire_file(const char* path,
                                const void* data,
                                size_t size);

WplResult wpl_write_entire_file_atomic(const char* path,
                                       const void* data,
                                       size_t size);

void wpl_free_file_data(WplFileData* data);
```

The module operates on paths supplied by the caller. It does not own application
paths, discover project roots, create directory hierarchies, monitor files, or
interpret file contents.

## Read Contract

`wpl_read_entire_file` reads a complete regular file into caller-owned
`WplFileData`.

Current behavior:

- `path` must be non-null and non-empty,
- `out_data` must be non-null,
- `out_data` is reset to `{NULL, 0}` before validation returns,
- missing files and open/stat/read errors return `WPL_RESULT_IO_ERROR`,
- non-regular files return `WPL_RESULT_IO_ERROR`,
- files larger than `WPL_MAX_FILE_SIZE_V0_1` return `WPL_RESULT_UNSUPPORTED`,
- empty files return `WPL_RESULT_OK` with `data == NULL` and `size == 0`,
- non-empty files allocate exactly enough memory for the file contents,
- the returned buffer is binary data and is not null-terminated by contract,
- callers release successful non-empty reads with `wpl_free_file_data`.

The read helper is for whole-file reads only. It does not provide streaming,
memory mapping, line iteration, text decoding, path expansion, or format parsing.

## Write Contract

`wpl_write_entire_file` writes a complete byte range to a path using a direct
truncate-and-write operation.

Current behavior:

- `path` must be non-null and non-empty,
- `data` may be null only when `size == 0`,
- sizes larger than `WPL_MAX_FILE_SIZE_V0_1` return `WPL_RESULT_UNSUPPORTED`,
- the destination is opened with create/truncate behavior,
- output file permissions are requested as `0644`, subject to process umask,
- short writes and write/close errors return `WPL_RESULT_IO_ERROR`,
- successful zero-size writes create or truncate the destination to an empty file.

The direct write helper is not crash-safe. Use `wpl_write_entire_file_atomic`
when replacement should avoid exposing partially written destination contents
under ordinary rename semantics.

## Atomic Write Contract

`wpl_write_entire_file_atomic` writes through a temporary file in the destination
directory, then renames it over the destination.

Current behavior:

- `path` must be non-null and non-empty,
- `data` may be null only when `size == 0`,
- sizes larger than `WPL_MAX_FILE_SIZE_V0_1` return `WPL_RESULT_UNSUPPORTED`,
- the final path must include a non-empty filename component,
- parent directories are not created,
- a hidden temporary sibling path is generated beside the destination,
- the temporary file is created with `mkstemp`,
- output file permissions are set to `0644`, subject to platform behavior,
- byte data is written completely before replacement,
- the temporary file is fsynced before close,
- the temporary path is renamed over the destination on success,
- parent directory fsync is attempted best-effort after rename,
- temporary files are unlinked on failures after creation.

Atomicity is limited to POSIX rename behavior on the same filesystem and the
filesystem's durability semantics. The helper does not implement transactional
multi-file commits, lock files, versioning, journaling, conflict resolution, or
application-level persistence policy.

## Ownership Contract

`WplFileData` returned by `wpl_read_entire_file` is caller-owned.

`wpl_free_file_data`:

- accepts null,
- frees `data->data`,
- resets `data->data` to null,
- resets `data->size` to zero,
- may be called repeatedly on the same reset structure.

Callers own path strings and write buffers. WPL consumes them only for the
duration of each call.

## Backend and Platform Boundary

File I/O lives in core code and is backend-independent. It must not include X11
headers, expose backend-native handles, own window state, poll input, submit draw
commands, or depend on renderer resources.

The implementation is Linux/POSIX-oriented and uses file descriptors internally.
Those descriptors remain private to implementation files and are not exposed in
public headers.

## Non-goals

The file I/O module deliberately does not provide:

- asset pipelines,
- graph serialization,
- project file formats,
- JSON/XML/binary schema policy,
- virtual filesystems,
- file watching,
- recursive directory creation,
- directory enumeration API,
- path normalization API,
- path expansion or environment lookup,
- streaming I/O API,
- memory-mapped files,
- lock files,
- multi-file transactions,
- compression,
- encryption,
- editor autosave policy,
- backend handles.

## Validation Requirements

File I/O changes should add tests or documented validation for:

- invalid path rejection,
- output reset on read failure,
- missing file handling,
- directory read rejection,
- empty file reads,
- binary reads with embedded NUL bytes,
- direct zero-size writes,
- direct zero-size truncation of existing files,
- direct binary writes,
- direct overwrite/truncation behavior,
- oversized read/write rejection,
- atomic zero-size writes,
- atomic zero-size replacement of existing files,
- atomic binary writes,
- atomic replacement behavior,
- atomic missing-parent failure,
- atomic trailing-slash path rejection without temporary-file residue,
- atomic temporary-file cleanup after validation failure,
- `wpl_free_file_data` null/reset behavior,
- backend-leak checks for public headers.

Current focused file I/O validation target:

```sh
ctest --test-dir build --output-on-failure -R 'wpl_test_file_io(_edges)?$'
```

The focused file I/O targets cover invalid path rejection, output reset behavior,
missing and non-regular input handling, empty and binary whole-file reads,
direct and atomic zero-size writes, overwrite/replacement behavior, oversized
rejection, atomic temporary-file cleanup, trailing-slash atomic path rejection,
and `wpl_free_file_data` reset behavior.

Focused file I/O validation does not replace full CTest, sanitizer validation,
backend-leak checks, frame-pacing boundary checks, focused draw, renderer, canvas,
debug overlay validation, replay validation, or Xvfb smoke validation.

Phase 8a documented this contract. Phase 8b added file I/O edge coverage. Phase
8c syncs this validation documentation and closes the Phase 8 file I/O hardening
pass.
