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

## Enable publication

After builds and installed-image Quickstart checks pass, create or authorize the `ioeslab/posim` Docker Hub repository
and configure these **repository secrets**:

- `DOCKERHUB_USERNAME`
- `DOCKERHUB_TOKEN` with permission to push to `ioeslab/posim`

Set repository variable `POSIM_PUBLISH_IMAGES` to `true` to enable login and
publication for successful `main` or version-tag builds. The workflow validates
the local image before logging in and pushing that same image ID. With the
variable unset, it builds and tests without publishing. PR builds never publish
images. Runtime evidence is uploaded as a workflow artifact (14-day retention).

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
