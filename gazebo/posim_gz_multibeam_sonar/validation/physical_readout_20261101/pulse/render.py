# SPDX-License-Identifier: Apache-2.0
"""Redraw the actual native control comparison; no simulation or fitted signals."""
from pathlib import Path
import json
import numpy as np
from matplotlib.figure import Figure
from matplotlib.backends.backend_agg import FigureCanvasAgg

HERE = Path(__file__).resolve().parent
before = np.load(HERE / "before/arrays.npz")
after = np.load(HERE / "after-integrated/arrays.npz")
records = {
    kind: json.loads((HERE / kind / "result.json").read_text())
    for kind in ["before", "after-owner", "after-integrated"]
}
x = after["published_ranges_m"]
DR = x[1] - x[0]
old = abs(before["n599_r15.0_sensor_l2"].astype(np.complex128)) ** 2
new_full = abs(after["s180_m2048_r15.0"].astype(np.complex128)) ** 2
old_share, new_share = old / old.sum(), new_full[: len(x)] / new_full.sum()
fig = Figure(figsize=(15.5, 9), facecolor="#f6f8fb", dpi=160)
FigureCanvasAgg(fig)
fig.text(
    0.055,
    0.948,
    "A far echo must stay far. A longer record must not dim it.",
    fontsize=21,
    fontweight="bold",
    color="#142639",
)
fig.text(
    0.055,
    0.912,
    "Native Metal comparison  |  15 m range  |  29.9 kHz bandwidth  |  unchanged scatterer, beam and random draw",
    fontsize=11.5,
    color="#425569",
)
blue, coral = "#1677b8", "#c5553d"


def style(ax):
    ax.set_facecolor("white")
    for spine in ax.spines.values():
        spine.set_color("#c7d1db")
    ax.tick_params(colors="#425569", labelsize=10)


fig.text(
    0.11,
    0.867,
    "Same 599 output bins; near and far windows enlarged",
    fontsize=12,
    color="#142639",
)
for left, mask, title in [(0.11, x <= 0.30, "Near window"), (0.385, x >= 14.70, "Far window")]:
    ax = fig.add_axes([left, 0.665, 0.225, 0.18])
    style(ax)
    part = x[mask]
    image = ax.imshow(
        10 * np.log10(np.maximum(np.stack([old_share[mask], new_share[mask]]), 1e-12)),
        aspect="auto",
        origin="upper",
        extent=[part[0] - DR / 2, part[-1] + DR / 2, 1.5, -0.5],
        vmin=-80,
        vmax=0,
        cmap="magma",
        interpolation="nearest",
    )
    ax.set_yticks([0, 1], ["Before", "After"] if left < 0.2 else ["", ""])
    ax.set_xlabel("Range (m)")
    ax.text(0.5, 0.92, title, transform=ax.transAxes, fontsize=10, color="white", ha="center")
    if left < 0.2:
        ax.annotate(
            "False near echo",
            xy=(0.002, 0),
            xytext=(0.055, 0.25),
            color="white",
            fontsize=10,
            arrowprops=dict(arrowstyle="->", color="white"),
        )
    else:
        ax.text(14.73, 0.72, "15 m target →", color="white", fontsize=10)
cb = fig.colorbar(image, cax=fig.add_axes([0.625, 0.665, 0.012, 0.18]))
cb.ax.tick_params(labelsize=9)
cb.set_label("dB of total echo energy", fontsize=9)

near = fig.add_axes([0.11, 0.335, 0.225, 0.23])
far = fig.add_axes([0.385, 0.335, 0.225, 0.23])
for a in [near, far]:
    style(a)
    a.plot(x, old_share * 100, color=coral, label="Before", linewidth=2)
    a.plot(x, new_share * 100, color=blue, label="After", linewidth=2)
    a.set_ylim(-1, 77)
    a.set_xlabel("Range (m)")
    a.grid(alpha=0.18)
near.set_xlim(-0.01, 0.5)
far.set_xlim(14.8, 15.025)
near.set_ylabel("Total echo energy per bin (%)")
near.set_title("Near sensor: spurious return", loc="left", fontsize=11)
far.set_title("Far range: target retained", loc="left", fontsize=11)
near.text(
    0.045,
    57,
    f"Before: {100 * old_share[x < 0.5].sum():.2f}%\nAfter: <0.000001%",
    fontsize=11,
    linespacing=1.6,
    color="#142639",
)
far.legend(frameon=False, loc="upper left", fontsize=10)

energy = fig.add_axes([0.735, 0.60, 0.225, 0.245])
style(energy)
legacy = next(r for r in records["before"]["energy_scaling"] if r["mode"] == "sensor_l2")["ratios"]
actual = records["after-integrated"]["energy_ratios"][0]["full_transform_energy_ratios"]
ratio = np.array([actual[str(m)] for m in [1024, 2048, 4096]]) / actual["1024"]
energy.plot(
    [1, 2, 4],
    [legacy[str(m)] for m in [599, 1198, 2396]],
    "o-",
    color=coral,
    label="Before: discrete L2",
    linewidth=2,
)
energy.plot([1, 2, 4], ratio, "o-", color=blue, label="After: fixed pulse", linewidth=2)
energy.set_xticks([1, 2, 4], ["1×", "2×", "4×"])
energy.set_ylim(0, 1.14)
energy.set_xlabel("Internal calculation span")
energy.set_ylabel("Energy / shortest-span energy")
energy.set_title("4 m target: energy stays constant", loc="left", fontsize=11)
energy.grid(alpha=0.18)
energy.legend(frameon=False, fontsize=9, loc="lower left")

precision = fig.add_axes([0.735, 0.29, 0.225, 0.20])
style(precision)
errors = [
    100 * max(r["relative_complex_discrete_error"] for r in records[k]["rows"])
    for k in ["after-owner", "after-integrated"]
]
precision.barh([1, 0], errors, color=[coral, blue], height=0.48)
precision.set_xscale("log")
precision.set_xlim(0.001, 1)
precision.set_yticks([1, 0], ["PR #6 integer", "+ #34 float"])
precision.axvline(0.3, color="#566573", ls="--", lw=1)
precision.set_ylim(-0.5, 1.55)
precision.text(0.29, 1.32, "0.3% bound", ha="right", fontsize=9, color="#566573")
for y, value in zip([1, 0], errors):
    precision.text(value * 1.12, y, f"{value:.4f}%", va="center", fontsize=10)
precision.set_xlabel("Worst complex-signal error (%)")
precision.set_title("Weak echoes still need #34", loc="left", fontsize=11)

fig.text(
    0.055,
    0.19,
    "✓ 42 / 42 floating-path controls     ✓ near + far + two targets     ✓ invalid range contract rejected",
    fontsize=12,
    color="#156048",
    fontweight="bold",
)
fig.text(
    0.055,
    0.146,
    "✓ Real Gazebo: 129 × 256 geometry → 256 receivers × 599 ranges; four synchronized packet sets, normal shutdown.",
    fontsize=11,
    color="#425569",
)
fig.text(
    0.055,
    0.099,
    "Top/left: fractions of full-transform energy, including retained guard samples. No range bins are masked. Display floor: −80 dB.\n"
    "Energy curves are relative to each series’ shortest span. Main comparison holds #31–36 fixed; #41 pulse handling changes.\n"
    "Baseband Hamming control, not hardware calibration or SLAM acceptance. PR #6 integer path retains 9 / 42 numerical failures.",
    fontsize=9.5,
    color="#526477",
    va="top",
    linespacing=1.6,
)
for suffix in ["png", "svg"]:
    fig.savefig(HERE.parent / f"pulse-comparison.{suffix}", dpi=160, facecolor=fig.get_facecolor())
