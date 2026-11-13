# POSIM releases

Keep one release-note file per version. The root [changelog](../../CHANGELOG.md)
records the history in reverse chronological order.

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
   later changes. Add the publication date when the release ships.
4. Run the repository checks and the documented workflows against that
   commit. Qualify clean native Ubuntu/WSL installation and both Docker
   architectures, including the Apple Silicon RDP desktop. Check the first
   simulation, sensor data, WAM-V arming and shutdown, and QGroundControl
   connection and video. Record toolchain versions and any limitations with
   the validation evidence outside the product repository.
5. Review the release notes, installation instructions, maintainer contacts
   and [third-party attribution](../../README.md#license-and-attribution).
   Obtain the maintainer's release decision after reviewing the results.

## Publish the qualified version

Publish the GitHub release with the matching `v<version>` Git tag on the
qualified commit. The Docker workflow checks the tag against the documentation
version, builds and tests both architectures, and publishes
`ioeslab/posim:<version>`. Image publication requires the repository's
`POSIM_ENABLE_DOCKER_CI` and `POSIM_PUBLISH_IMAGES` variables to be enabled and its
Docker Hub credentials to be configured.

To build an image for an existing release, run **Build and publish POSIM Docker
images** from `main` and enter its Git tag in `release_tag`.

Confirm that the versioned multi-architecture image contains both AMD64 and
ARM64 variants and that each variant's source-revision label matches the tag.
Record the immutable image digest in the GitHub release. Use the version's
release notes for that release, converting repository-relative links to links
under the source tag. Update this index after publication.
