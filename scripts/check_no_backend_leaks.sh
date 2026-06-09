#!/usr/bin/env sh
set -eu

repo_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)

if grep -RInE '#[[:space:]]*include[[:space:]]*<X11/' \
  "$repo_dir/include" \
  "$repo_dir/src" \
  "$repo_dir/tests" \
  "$repo_dir/examples"; then
  echo "unexpected X11 include outside backend-private files" >&2
  exit 1
fi

if grep -RInE '#[[:space:]]*include[[:space:]]*<X11/|(^|[^A-Za-z0-9_])(Display|XEvent|XImage|GC|Atom|Visual|Colormap|KeySym|KeyCode|Xkb)([^A-Za-z0-9_]|$)' \
  "$repo_dir/include/wpl"; then
  echo "backend type leak detected in public headers" >&2
  exit 1
fi

if ! grep -RInE '#[[:space:]]*include[[:space:]]*<X11/' \
  "$repo_dir/backends/linux_x11" >/dev/null; then
  echo "warning: no X11 includes found in linux_x11 backend; verify backend wiring" >&2
fi
