# SPDX-License-Identifier: Apache-2.0
"""Plot original native sonar packets with a common metric and intensity scale."""
from pathlib import Path
from matplotlib.figure import Figure
from matplotlib.backends.backend_agg import FigureCanvasAgg
import numpy as np
from verify import verify

ROOT = Path(__file__).resolve().parent
summary, arrays = verify()
labels = ["render-129", "render-257", "render-513-reliable"]
maximum = max(float(np.nanmax(arrays[label + "_first_raw"])) for label in labels)
fig = Figure(figsize=(16, 6))
FigureCanvasAgg(fig)
axes = fig.subplots(1, 3)
fig.patch.set_facecolor("white")
fig.subplots_adjust(left=0.055, right=0.91, top=0.75, bottom=0.25, wspace=0.18)
for ax, label, samples in zip(axes, labels, [129, 257, 513]):
    raw = arrays[label + "_first_raw"]
    ranges = arrays[label + "_first_raw_ranges"]
    directions = arrays[label + "_first_raw_directions"]
    angles = np.arctan2(directions[:, 1], directions[:, 0])
    theta = np.r_[-np.pi / 3, (angles[:-1] + angles[1:]) / 2, np.pi / 3]
    radius = np.r_[0, (ranges[:-1] + ranges[1:]) / 2, 15]
    x, y = radius[:, None] * np.sin(theta), radius[:, None] * np.cos(theta)
    ax.set_facecolor("#08090c")
    plot = ax.pcolormesh(
        x, y, raw, cmap="inferno", vmin=maximum - 60, vmax=maximum, shading="flat", rasterized=True
    )
    ax.axhline(4, color="#77d6ff", lw=0.9, ls="--", alpha=0.8)
    ax.set(xlim=(-13, 13), ylim=(0, 15), xlabel="Lateral distance (m)")
    ax.set_aspect("equal", adjustable="box")
    ax.set_title(
        f"{samples} geometry samples\n256 output beams", fontsize=16, weight="bold", pad=13
    )
    ax.text(
        0.5,
        -0.25,
        "599 ranges × 256 beams\nReported power FWHM: 1.5°",
        transform=ax.transAxes,
        ha="center",
        va="top",
        fontsize=12,
        color="#344054",
    )
axes[0].set_ylabel("Forward distance (m)")
cax = fig.add_axes([0.935, 0.25, 0.012, 0.49])
fig.colorbar(plot, cax=cax, label="Raw level (dB), common scale")
fig.suptitle(
    "129 → 513 geometry samples · The same 256 receive beams",
    x=0.055,
    y=0.96,
    ha="left",
    fontsize=23,
    weight="bold",
    color="#142034",
)
fig.text(
    0.055,
    0.875,
    "Actual Gazebo + Metal + ROS output  ·  Same 4 m wall  ·  120° × 20° fan  ·  15 m range",
    fontsize=15,
    color="#344054",
)
fig.text(
    0.055,
    0.065,
    "First original ping from each run (simulation time 0.03 s). Full raw arrays; no fitted brightness correction.",
    fontsize=11,
    color="#475467",
)
fig.text(
    0.055,
    0.03,
    "1.5° is a specified synthetic receive model, not a Sonoptix calibration. Temporal persistence and useful SLAM remain unqualified.",
    fontsize=11,
    color="#475467",
)
fig.savefig(ROOT.parent / "sensor-wiring.png", dpi=150)
fig.savefig(ROOT.parent / "sensor-wiring.svg")
