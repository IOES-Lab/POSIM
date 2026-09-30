# Transport subscription-churn regression

The publisher sends real payloads on 24 topics. The subscriber retains one
anchor subscription and repeatedly creates and removes the last subscriber on
the other topics. A passing trial requires nonzero anchor and churn deliveries,
normal termination of both processes, and no timeout. `run_pairs.py` preserves
every trial, the loaded library path/hash, and an available core dump. It never
retries a failed trial to obtain a pass.

The test exercises the concurrent SUB-socket access implicated in a Gazebo
shutdown failure: `RunReceptionTask -> zmq_poll -> xsub_t::match -> trie_t::check`.
The candidate patch serializes the zero-timeout socket poll with subscription
changes. Blocking waits use only `ZMQ_FD` notification descriptors outside the
node mutex. Socket events are rechecked on every loop, as required by ZeroMQ's
[edge-triggered notification contract](https://libzmq.readthedocs.io/en/latest/zmq_getsockopt.html).

`tools/diagnose-transport-poll.sh` builds matched unpatched/patched libraries in
a disposable container for five paired 20-second trials. This diagnostic uses
the source defaults and is not final-image acceptance. Release-candidate builds
use `extras/build-image-transport.sh`: a pinned ROS vendor overlay, Zenoh disabled
to match the installed vendor, and a byte-for-byte public configuration check.
No files under `/opt/ros` are replaced.

Image inventory validation verifies the overlay revision, patch and library
hashes, and the linkage of both the probe and Gazebo simulator. It repeats the
probe without overriding the installed library search path. The separate full
matrix still has to pass all 77 runtime trials on each native architecture.
Neither a green diagnostic job nor zero failures in a finite sample establishes
zero failure probability or authorizes image publication.
