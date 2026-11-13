# POSIM releases

Keep one release-note file per version. The root [changelog](../../CHANGELOG.md)
records the history in reverse chronological order.

- [1.0.1 release candidate](v1.0.1.md): RDP runtime paths, dynamic geometry,
  native installation and Docker publication corrections. Not yet published.
- [1.0.0](v1.0.0.md): first stable release.

## Prepare a release

1. Integrate the intended changes and identify the source commit to qualify.
   Resolve outstanding release blockers before tagging it.
2. Set the same version in `website/package.json`, POSIM's ROS `package.xml`
   files, and the `sonar_wgpu` package in `Cargo.toml` and `Cargo.lock`. Keep
   third-party dependency versions separate. The website reads its header
   version from `website/package.json`.
3. Add `v<version>.md` here and link it from this index. Move the relevant
   changelog entries into a versioned section, retaining **Unreleased** for
   later changes. Use `Status: release candidate.` in the notes until publication.
   Put candidate-only installation instructions between
   `<!-- candidate-only:start -->` and `<!-- candidate-only:end -->` markers.
   The published GitHub release replaces the status with its UTC publication
   date, omits that marked block, and adds the verified image digest.
4. Run the repository checks and the documented workflows against that
   commit. Qualify clean native Ubuntu/WSL installation and both Docker
   architectures, including the Apple Silicon RDP desktop. Check the first
   simulation, sensor data, WAM-V arming and shutdown, and QGroundControl
   connection and video. Record toolchain versions and any limitations with
   the validation evidence outside the product repository.
5. Review the release notes, installation instructions, maintainer contacts
   and [third-party attribution](../../README.md#license-and-attribution).
   Obtain the maintainer's release decision after reviewing the results.

## Publish by merging a release candidate

Create a same-repository branch named `release/<version>-rc` or
`release/<version>-rc-<suffix>`, using lowercase letters, digits and hyphens
in the suffix. Add the `release-candidate` label to its PR targeting `main`.
Update `.github/release-candidate.json` in that PR: set `version` to the
matching package version and start `attempt` at `1` for a new version, or
increment it for a follow-up. The manifest change ensures documentation checks
run even when the follow-up only prepares another attempt.

Review and merge the candidate after the required source and platform checks.
**Publish merged release candidate** evaluates CI for that exact merge commit.
It requires the Docker, lint, documentation and release-automation workflows,
and waits for any other main-branch push workflows on that commit. Every
completed workflow must succeed. A normal PR without the candidate branch,
label and manifest change does not publish a release.

After CI succeeds, the automation checks package versions and the AMD64/ARM64
image revisions. It promotes the existing commit image, verifies the versioned
digest, creates the exact Git tag and a draft GitHub release, then publishes
the release. It does not rebuild the image or depend on a release event to
start another workflow. Existing tags or drafts pointing to a different
commit are rejected. Repeating a successful run does not replace the release.

Once the GitHub release is public, its verified multi-architecture digest also
becomes `ioeslab/posim:latest`, provided it is still GitHub's latest release.
Both the version tag and `latest` contain AMD64 and ARM64 images; Docker selects
the host architecture automatically. Main-branch builds publish only their
`sha-<12-character-commit>` tags. Retrying an older release cannot move `latest`
backwards.

The workflow needs `POSIM_PUBLISH_IMAGES=true`, the existing Docker Hub
credentials, and Actions permission to create pull requests at both organization
and repository level. Its token has contents and pull-request write permissions
and Actions read permission. It never submits a PR approval or merge request.
GitHub may require a maintainer to approve workflow runs on a bot-created PR.

## Recover from a failed candidate

If CI fails, the automation creates one Draft follow-up PR from current `main`
for the failed source commit. It increments the candidate attempt and links the
failed runs. It does not change simulator code or claim to fix the failure.
Add the required fixes, review the candidate, and merge it manually to run the
release checks on its new commit. Repeated completion events reuse the same
PR. A closed follow-up stays closed, and an existing open candidate prevents
another candidate for the same version.

For a transient CI failure, rerun the failed CI job. If it succeeds, the original
commit can proceed to publication; close the unused follow-up PR. Once a newer
candidate is merged, old completion events cannot publish the older attempt.

If Docker tagging or GitHub publication fails after CI passed, rerun
**Publish merged release candidate** from `main`, supplying the original full
merge SHA as `source_sha`. It rechecks CI and reuses the verified image and any
matching draft. It does not create a source-repair PR for a publication-only
failure. Never move an existing version tag to a new commit; use a new version
when source changes are needed after that tag exists.

GitHub publication and Docker's `latest` update are separate operations. If the
last update fails, the release remains public and `latest` retains its previous
value until the publication workflow is rerun successfully.

## Manual publication

For a review build, run **Build and publish POSIM Docker images** with
`source_branch` set to a branch in `IOES-Lab/POSIM`. Both architecture images
are built and tested, but a manual build publishes images only for `main`.
Both builds use GitHub-hosted runners. Fork pull requests remain excluded from
full image builds.

Publish the GitHub release with the matching `v<version>` Git tag on the
qualified commit once its `ioeslab/posim:sha-<12-character-commit>` image is on
Docker Hub. **Tag POSIM Docker release** verifies that both architectures match
the tagged commit and adds `ioeslab/posim:<version>` to the same image digest.
The workflow uses `POSIM_PUBLISH_IMAGES` and the configured Docker Hub
credentials.

To tag an image for an existing release, run **Tag POSIM Docker release**
from `main` and enter its Git tag in `release_tag`.

Confirm that the versioned multi-architecture image contains both AMD64 and
ARM64 variants and that each variant's source-revision label matches the tag.
Record the immutable image digest in the GitHub release. Use the version's
release notes for that release, converting repository-relative links to links
under the source tag, replacing candidate status with the actual publication
date, and omitting candidate-only instructions. Update this index and the
changelog after publication; automation does not add another source commit
after the qualified release commit.
