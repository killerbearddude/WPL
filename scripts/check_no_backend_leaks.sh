#!/usr/bin/env sh
set -eu

repo_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)

if grep -RInE '#[[:space:]]*include[[:space:]]*<X11/|(^|[^A-Za-z0-9_])(Display|XEvent|XImage|GC|Atom|Visual|Colormap|KeySym|KeyCode|Xkb)([^A-Za-z0-9_]|$)' "$repo_dir/include/wpl"; then
  echo "backend type leak detected in public headers" >&2
  exit 1
fi
