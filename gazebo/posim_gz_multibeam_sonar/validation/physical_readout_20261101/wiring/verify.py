# SPDX-License-Identifier: Apache-2.0
"""Verify original native output arrays, metadata and SDF without running Gazebo."""
from pathlib import Path
import hashlib
import json
import xml.etree.ElementTree as ET

import numpy as np

ROOT = Path(__file__).resolve().parent


def verify():
    summary = json.loads((ROOT / "summary.json").read_text())
    assert hashlib.sha256((ROOT / "data.npz").read_bytes()).hexdigest() == summary["data_sha256"]
    arrays = np.load(ROOT / "data.npz", allow_pickle=False)
    for case in summary["cases"]:
        label = case["label"]
        columns = case["geometry_columns"]
        raw = arrays[label + "_raw"]
        directions = arrays[label + "_raw_directions"]
        ranges = arrays[label + "_raw_ranges"]
        width = np.pi * 2 / 3 / 255 if case["mode"] == "legacy" else np.deg2rad(1.5)
        assert raw.shape == (599, 256) and not np.isnan(raw).any()
        assert np.allclose(
            np.arctan2(directions[:, 1], directions[:, 0]),
            np.linspace(-np.pi / 3, np.pi / 3, 256),
            rtol=0,
            atol=1e-6,
        )
        assert np.allclose(directions[:, 2], 0)
        assert np.allclose(arrays[label + "_raw_rx_widths"], width, rtol=0, atol=1e-7)
        assert np.allclose(arrays[label + "_raw_tx_widths"], np.pi / 9, rtol=0, atol=1e-7)
        assert np.allclose(ranges, np.arange(599) * 1500 / 59800, rtol=0, atol=2e-6)
        assert arrays[label + "_normal"].shape == (256, columns, 3)
        depth = arrays[label + "_depth"]
        assert depth.shape == (256, columns, 3)
        finite = np.isfinite(depth).all(axis=-1)
        assert finite.sum() > columns * 250
        assert np.max(np.abs(depth[..., 0][finite] - 4)) < 0.05
        preview = arrays[label + "_preview"]
        assert preview.shape == (600, 1199, 3)
        if case["mode"] == "zero-gain":
            assert np.isneginf(raw).all() and preview.max() == 0
        else:
            assert np.isfinite(raw).any() and preview.max() > 0
        assert len(case["matched_stamps_ns"]) >= 4
        assert len({row["stamp_ns"] for row in case["selected_packet"]}) == 1
        assert all(row["exit"] == 0 and not row["forced"] for row in case["cleanup"])
        for row in case["selected_packet"]:
            if row["kind"] != "depth":  # XYZ geometry omits packed intensity/ring fields.
                assert (
                    hashlib.sha256(arrays[row["key"]].tobytes()).hexdigest()
                    == row["raw_payload_sha256"]
                )
        sdf = ET.parse(ROOT / (label + ".sdf"))
        config = sdf.find("model/link/sensor/{http://gazebosim.org}multibeam_sonar")
        assert int(config.findtext("ray/scan/horizontal/beams")) == 256
        assert int(config.findtext("ray/scan/horizontal/samples", "256")) == columns
        assert float(config.findtext("ray/range/max")) == 15
        if case["mode"] != "legacy":
            assert abs(float(config.findtext("spec/receiveBeamWidth")) - width) < 1e-12
            assert float(config.findtext("spec/receiveGain")) == (
                0 if case["mode"] == "zero-gain" else 1
            )
    print(
        "PASS: five actual sensor controls; output/geometry separation, metadata, legacy and zero gain"
    )
    return summary, arrays


if __name__ == "__main__":
    verify()
