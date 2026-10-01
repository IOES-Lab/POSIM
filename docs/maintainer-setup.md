# CI and image configuration

POSIM uses `main` as its default branch. DAVE repository secrets, runners, branch
rules, and completed CI runs are not copied by importing Git history.

## Enable builds

1. Grant the development account write access to `IOES-Lab/POSIM`.
2. Make the appropriate self-hosted runners available to this repository. The
   workflows select `[self-hosted, x64]` and `[self-hosted, ARM64]`. Runners
   registered only to the DAVE repository cannot receive POSIM jobs.
3. Check Docker access under the runner service account.
4. Set repository variable `POSIM_ENABLE_DOCKER_CI` to `true`.
5. Run both Docker workflows against the same POSIM commit.

Until that variable is enabled, Docker jobs remain skipped. Lint runs on `main`
and pull requests. Docker jobs reject fork pull requests on self-hosted runners.
The image build uses the checked-out source as its Docker context.

## Current publication boundary (2026-10-01)

`POSIM_ENABLE_DOCKER_CI` is enabled. `POSIM_PUBLISH_IMAGES` remains `false`.
Two [validation-only candidate tags](docker.md) were published through PR #5's
explicit manual workflow; this did not enable normal main/release publication
or merge PR #5. The runtime fixes and candidate-publishing tools are still on
that PR branch, not on `main`. Do not enable general publication until the
runtime validation changes have been reviewed/merged and `main` has passed
its own build, runtime and registry re-pull checks.

## Enable future main/release publication

After builds work, create or authorize the `ioeslab/posim` Docker Hub repository
and configure these **repository secrets**:

- `DOCKERHUB_USERNAME`
- `DOCKERHUB_TOKEN` with permission to push to `ioeslab/posim`

Set repository variable `POSIM_PUBLISH_IMAGES` to `true` to enable login and
publication for non-PR builds. With that variable unset, the workflows build
without logging in or publishing. The ordinary PR build workflows never publish images. The explicitly guarded
PR #5 candidate publisher is a separate validation-only path.

The inherited DAVE PR-image publisher is omitted from the initial POSIM setup:
the imported Docker build workflows do not produce the image archives that it
requires. PR artifact publication needs a complete producer/consumer change and
separate validation before it can be enabled here.

## Before a release

- Record the exact source and resolved companion repository revisions.
- Confirm both architecture builds for that revision.
- Pull the published images on their target architectures and execute Quickstart
  tests from their installed workspaces.
- Record the image digests and runtime results.
- State CUDA-dependent limitations and whether WGPU is actually included.
- Review package versioning, release notes, and the release tag together.

The initial repository import is not a POSIM 1.0 release or a new runtime test
result. The first release announcement is prepared after these checks.
