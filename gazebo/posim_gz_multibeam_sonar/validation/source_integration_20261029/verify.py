"""Verify measured lifecycle data and hashes using the standard library."""

import hashlib
import json
from pathlib import Path

root = Path(__file__).resolve().parent


def read(name):
    return json.loads((root / name).read_text())


for name, digest in read("sha256.json").items():
    assert hashlib.sha256((root / name).read_bytes()).hexdigest() == digest, name
source = read("source.json")
assert set(source["ancestors"]) == {"main", "6", "31", "32", "33", "34"}
assert source["runtime_changes_since_binary_build"] == []
assert source["scope"]["lifecycle_qualified"] is True
for key in (
    "geometric_accuracy_qualified",
    "slam_map_qualified",
    "installed_archive_qualified",
    "installed_sdk_modified",
):
    assert source["scope"][key] is False
for backend, values in read("native.json").items():
    creation = values["creation"]
    assert creation["pass"] and creation["exit"] == 0
    assert creation["initial_frames"] >= 5 and creation["recreated_frames"] >= 5
    assert creation["initial_initializations"] == 1 and creation["total_initializations"] == 2
    assert creation["shape"] == {"beams": 513, "ranges": 399}
    rows = values["shutdown"]
    assert len(rows) == 12
    assert {(r["repetition"], r["signal"], r["delay_s"]) for r in rows} == {
        (repeat, signal, delay)
        for repeat in (0, 1)
        for signal in (2, 15)
        for delay in (0.0, 0.005, 0.05)
    }
    assert all(r["passed"] and r["exit"] == 0 and not r.get("forced_cleanup") for r in rows)
    assert all(r["shutdown_s"] < 20 for r in rows)
assert [r["exit"] for r in read("constructor-before.json")] == [0, 1]
assert [r["exit"] for r in read("constructor-after.json")] == [0, 0, 1, 1]
render = read("render-controls.json")
assert len(render) == 4
for row in render:
    assert row["camera_count"] == row["sonar_count"] == 0
    if row["variant"] == "sdk":
        assert row["exit"] == -11 and not row["render_demand_served"]
    else:
        assert row["exit"] == 0 and row["render_demand_served"]
        assert row["sensor_sha256"] == source["sensor_assembly"]["assembled_library_sha256"]
groups = read("cleanup.json")["groups"]
assert len(groups) == 26 and all(r["empty"] for r in groups)
assert len(read("traces.json")) == 26
print("PASS: CPU/Metal creation and 24 early exits; geometry, SLAM and archive remain separate")
