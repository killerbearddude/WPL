# WPL v0.1 Validation Report

## Summary

Status: **PASS**

Validated commit:

`9e34b53` — `Close v0.1 validation gaps (#18)`

Validation date:

`2026-06-09`

Validator:

`Daniel / WPL local validation`

This report records the evidence used to close the v0.1 validation gate. The fallback repeat-release path is accepted as a deferred v0.1 validation risk rather than claimed as directly validated.

## Automated CI Validation

| Check | Result | Evidence |
|---|---:|---|
| Ubuntu GCC CI | PASS | GitHub Actions `CI`, branch `main`, push run `27242860901` |
| Ubuntu Clang CI | PASS | GitHub Actions `CI`, branch `main`, push run `27242860901` |

The latest observed `main` run listed by `gh run list --branch main --limit 10` was successful for `Close v0.1 validation gaps` on commit `9e34b53`.

## Local Build and Test Validation

| Check | Result |
|---|---:|
| `./scripts/build.sh` | PASS |
| `./scripts/test.sh` | PASS |
| `ctest --test-dir build --output-on-failure` | PASS |
| `cmake -S . -B build-clang -DCMAKE_C_COMPILER=clang -DCMAKE_BUILD_TYPE=Debug` | PASS |
| `cmake --build build-clang --parallel` | PASS |
| `ctest --test-dir build-clang --output-on-failure` | PASS |
| `./scripts/check_public_headers.sh build` | PASS |
| `./scripts/check_public_headers.sh build-clang` | PASS |
| `./scripts/check_no_backend_leaks.sh` | PASS |
| `git diff --check` | PASS |

Default/local test result:

```text
100% tests passed, 0 tests failed out of 10
```

Clang test result:

```text
100% tests passed, 0 tests failed out of 10
```

Public header checks passed for both default and Clang build directories.

## Manual Environment

```text
uname: Linux MyComputer 6.8.0-124-generic #124~22.04.1-Ubuntu SMP PREEMPT_DYNAMIC Tue May 26 21:05:19 UTC x86_64 x86_64 x86_64 GNU/Linux
XDG_SESSION_TYPE: wayland
DESKTOP_SESSION: zorin
XDG_CURRENT_DESKTOP: zorin:GNOME
DISPLAY: :0
Window manager / desktop: Zorin GNOME
X11/XWayland: XWayland session inferred from Wayland desktop with DISPLAY=:0 and successful X11 example execution
```

## Manual Graphical Smoke Validation

Manual command:

```sh
./scripts/manual_graphical_smoke.sh --run
```

| Example | Result | Notes |
|---|---:|---|
| `00_empty_window` | PASS | Window opened and returned to shell during sequential smoke run. |
| `01_input_snapshot` | PASS | Keyboard, mouse, modifier, and motion output observed. |
| `02_draw_primitives` | PASS | Example completed during sequential graphical smoke run. |
| `03_canvas_pan_zoom` | PASS | Example completed during sequential graphical smoke run. |
| `04_debug_overlay` | PASS | Example completed during sequential graphical smoke run. |
| `05_input_replay record` | PASS | Record command completed during sequential smoke run. |
| `05_input_replay playback` | PASS | Playback command completed during sequential smoke run. |

The input snapshot output included mouse movement, mouse button press/release, key press/release, and modifier-state changes, confirming the graphical/input examples were executing against the local X11/XWayland display.

## XKB Auto-Repeat Validation

Procedure:

```text
Run examples/01_input_snapshot.
Focus the window.
Press and hold A for several seconds.
Release A.
```

Expected:

```text
A press appears once.
No A release appears while held.
A release appears once on physical release.
```

Observed:

```text
Focused examples/01_input_snapshot, pressed and held A for several seconds, saw one A press, no A release while held, and one A release when physically released.
```

Result:

```text
PASS
```

## Fallback Repeat-Release Validation

Status:

```text
DEFERRED RISK
```

Decision:

```text
Fallback repeat-release path is accepted as a deferred v0.1 validation risk.
```

Rationale:

```text
Primary XKB detectable auto-repeat behavior was validated on the release test environment. The fallback path is retained for environments where XKB detectable auto-repeat is unavailable, but no reliable local X11/Xvfb/Xnest configuration was available in this validation cycle to force that path. This is tracked as post-v0.1 validation work rather than a v0.1 release blocker.
```

## Known Remaining Risks

| Risk | Severity | Release Decision |
|---|---:|---|
| Fallback repeat-release path not directly validated | Medium | Accepted deferred v0.1 validation risk |
| Uncommon X11 visuals return `WPL_RESULT_UNSUPPORTED` | Low | Accepted v0.1 limitation |
| Graphical validation performed on XWayland rather than native X11 | Low | Accepted because WPL X11 examples executed successfully through the X11 compatibility display |

## Final Release Gate

```text
PASS
```

Decision notes:

```text
WPL v0.1 has passing main-branch CI, passing local default/GCC validation, passing local Clang validation, passing public-header/backend-leak checks, passing manual graphical smoke validation, and passing XKB detectable auto-repeat hold-key validation. The fallback repeat-release path is explicitly accepted as a deferred v0.1 validation risk.
```
