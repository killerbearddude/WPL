# Contributing

WPL changes should be small, focused, reviewable pull requests.

## Patch workflow

1. Start from the current `main` branch.
2. Create a focused branch for one patch.
3. Keep the diff limited to the patch objective.
4. Run local validation before committing.
5. Commit only intended files.
6. Open a pull request with testing notes and out-of-scope items.

## Local validation

```sh
./scripts/build.sh
./scripts/test.sh
```

## Scope discipline

Do not add GUI widgets, layout systems, node graph behavior, SDL, Wayland,
Windows support, macOS support, GPU abstractions, ECS systems, scene graphs, or
application-specific editor logic to WPL.
