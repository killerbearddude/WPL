# WPL Release Process

## v0.1 Release Procedure

1. Start from clean `main`.

```sh
git switch main
git pull --ff-only origin main
git status --short
git log --oneline -8
```

2. Confirm CI status.

```sh
gh run list --branch main --limit 10
gh run view --log
```

3. Confirm validation report exists.

```sh
test -f docs/validation_report_v0.1.md
```

4. Confirm release notes and changelog exist.

```sh
test -f CHANGELOG.md
test -f docs/release_notes_v0.1.md
```

5. Create an annotated tag after the release-prep PR is merged and CI passes on `main`.

```sh
git tag -a v0.1.0 -m "WPL v0.1.0"
git push origin v0.1.0
```

6. Create the GitHub release.

```sh
gh release create v0.1.0 \
  --title "WPL v0.1.0" \
  --notes-file docs/release_notes_v0.1.md
```

7. Verify the pushed tag and GitHub release.

```sh
git ls-remote --tags origin
gh release view v0.1.0
```

## Rules

- Do not tag from a feature branch.
- Do not tag before CI passes on `main`.
- Do not tag before validation evidence is recorded.
- Do not tag with unmerged local changes.
- Use annotated tags.
- Release notes must state known limitations.
- Do not publish source-feature changes as part of a release-prep patch.
