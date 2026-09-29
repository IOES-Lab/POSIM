# Origin and compatibility

POSIM stands for **Platform for Ocean Simulation**. The project continues the
maritime simulation library developed as DAVE. This first import retains the
ancestry of `IOES-Lab/dave`, including its upstream history from
`Field-Robotics-Lab/dave`.

## Import boundary

- Source: <https://github.com/IOES-Lab/dave>
- Source branch: `ros2`
- Source revision: `8a3f6abc2ba14ced787aff8befa0201f6c80ca8c`
- POSIM destination: <https://github.com/IOES-Lab/POSIM>
- POSIM default branch: `main`

The existing DAVE repository is retained. Its issues, pull requests, releases,
and Actions results do not become POSIM records automatically. Links to DAVE
changes in this documentation describe provenance, not POSIM release results.
Historical DAVE tags are not republished as POSIM releases.

## Names retained for existing users

| Interface | Initial POSIM behavior |
| --- | --- |
| ROS packages | The 13 existing package identifiers remain unchanged. |
| Launch files | `dave_world`, `dave_robot`, `dave_sensor`, and `dave_object` entry points are retained. |
| Resources | Package-share paths, model identifiers, world filenames, and Fuel URIs are retained. |
| ROS interfaces | Message/service definitions, configured topics, and frame names are retained. |
| Installation helper | `extras/ros-lyrical-gz-jetty-install.sh` and `DAVE_EXTRAS_DIR` remain supported. |
| Workspace directory | Source instructions use `posim_ws/src/dave`; the checkout key stays `dave`. |
| Docker workspace | `/opt/dave_ws` on AMD64 and `/home/docker/dave_ws` on ARM64 remain unchanged. |
| Repository manifests | `posim.lyrical.repos` is preferred; `dave.lyrical.repos` has the same repository entries. |

`dave_*` names in launch commands, package dependencies, and installed resource
paths are intentional. Renaming these identifiers requires a separate migration
with compatibility tests; the project name change alone does not require it.

## Current scope

The initial POSIM import changes project documentation, repository URLs, Docker
branding, and CI configuration. It retains the simulator source, descriptors,
launch implementations, and package manifests from the source revision.

The lint workflow also incorporates the lint configuration proposed in
[DAVE PR #74](https://github.com/IOES-Lab/dave/pull/74) at commit
`cfae083f4468c369fe002418d913cb5a5612a9bc`, adapted to `main`. This uses direct
format checks with read-only repository permissions and matching formatter
versions. Only its lint workflow is carried over.

WGPU integration is still being reviewed in
[DAVE PR #44](https://github.com/IOES-Lab/dave/pull/44). It is not part of this
initial `main` branch. Its inclusion and validation must be recorded in a later
change before POSIM advertises WGPU support.

## Versions

This is development work toward the first POSIM release. No `v1.0.0` tag is
created by this import, and inherited package version fields are unchanged.
Before release, record the exact POSIM commit, dependency revisions, build
results, image digests, and runtime checks. Earlier DAVE or local-candidate
test results must retain their original source revision labels.
