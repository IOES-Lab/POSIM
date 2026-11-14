# Fast DDS participant shutdown

The Docker images use Fast DDS 3.6.2 with a narrow backport of the shutdown
ordering correction from [upstream commit
3b4966787c](https://github.com/eProsima/Fast-DDS/commit/3b4966787cc897a6287c3a6242b22a1dac874007).
The TypeLookup manager stops and joins its request worker before deleting the
reply writer and history that the worker uses. This addresses the participant
deletion crash described in [Fast DDS issue
6599](https://github.com/eProsima/Fast-DDS/issues/6599).

`build.sh` verifies the 3.6.2 source archive and builds the patched runtime
library in `/opt/posim/fastdds`. It preserves the ROS-owned library, headers and
tools. The POSIM environment hook selects the corrected library after ROS
setup, including when Docker already exports that directory. A different
installed Fast DDS version stops the build until the backport is reviewed.

The patch changes shutdown ordering and grants the manager access to the
existing worker stop method. It retains the listener until its endpoints have
been removed and leaves discovery and shared-memory transport enabled. It does
not include the other changes in the upstream commit.

`test.sh` checks the loaded library and runs concurrent coordinate tests in a
shared ROS domain, exercising participant creation and deletion during type
discovery. Each process must pass every test and exit normally; failures are
retained in the image-test artifact.

Fast DDS and the upstream changes are licensed under Apache-2.0. The image
retains the upstream license, this recipe, its patch and library checksums.
