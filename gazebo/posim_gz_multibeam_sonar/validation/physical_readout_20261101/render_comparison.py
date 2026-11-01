# SPDX-License-Identifier: Apache-2.0
"""Re-render the PR comparison from retained native measurements."""
from pathlib import Path
import json
import math
import numpy as np
import matplotlib

import matplotlib.pyplot as plt

matplotlib.use("Agg")

OUT = Path(__file__).resolve().parent
arrays = np.load(OUT / "comparison-data.npz")
OLD = {name[7:]: arrays[name] for name in arrays.files if name.startswith("before_")}
summary = json.loads((OUT / "comparison-result.json").read_text())["after"]
OUTPUTS = arrays["output_angles"]
CENTER = len(OUTPUTS) // 2
HFOV = math.radians(30)
plt.rcParams.update(
    {
        "font.family": "DejaVu Sans",
        "font.size": 13,
        "axes.spines.top": False,
        "axes.spines.right": False,
        "axes.labelcolor": "#334155",
        "text.color": "#0f172a",
        "axes.titleweight": "bold",
    }
)
fig, axs = plt.subplots(
    2, 2, figsize=(14, 9.2), gridspec_kw={"height_ratios": [1, 1]}, facecolor="white"
)
fig.subplots_adjust(top=0.78, bottom=0.17, left=0.08, right=0.97, hspace=0.56, wspace=0.26)
fig.text(0.08, 0.948, "Finer geometry must not change the sonar", fontsize=25, weight="bold")
fig.text(
    0.08,
    0.907,
    "Same 5 m plane · same acoustic inputs · 65 / 129 / 257 geometry samples",
    fontsize=15,
    color="#475569",
)
fig.text(
    0.08,
    0.842,
    "BEFORE  ·  tied to the geometry grid",
    fontsize=17,
    weight="bold",
    color="#a33d32",
)
fig.text(
    0.565,
    0.842,
    "PROPOSED  ·  independent receive model",
    fontsize=17,
    weight="bold",
    color="#087f6b",
)
colors = ["#2563eb", "#d97706", "#7c3aed"]
grid = [65, 129, 257]
angle = np.linspace(-2.4, 2.4, 1601)
for n, color in zip(grid, colors):
    step = HFOV / (n - 1)
    before = np.sinc(0.884 / step * np.sin(np.deg2rad(angle))) ** 2
    axs[0, 0].plot(
        angle, 10 * np.log10(np.maximum(before, 1e-5)), color=color, lw=2, label=f"{n} samples"
    )
    signal = arrays[f"point_{n}"]
    power = np.sum(abs(signal.astype(np.complex128)) ** 2, axis=1)
    axs[0, 1].plot(
        np.rad2deg(OUTPUTS),
        10 * np.log10(np.maximum(power / power[CENTER], 1e-5)),
        color=color,
        lw=2.4,
        ls={65: "-", 129: "--", 257: ":"}[n],
        label=f"{n} samples",
    )
for ax in axs[0]:
    ax.set_xlim(-2.4, 2.4)
    ax.set_ylim(-35, 1)
    ax.set_xlabel("Angle from beam center (degrees)")
    ax.set_ylabel("Relative point power (dB)")
    ax.grid(alpha=0.15)
axs[0, 0].set_title("Beam response gets narrower", fontsize=15, pad=13)
axs[0, 1].set_title("Beam response stays fixed", fontsize=15, pad=13)
axs[0, 0].text(
    0.5,
    0.92,
    "FWHM  0.470° → 0.235° → 0.117°",
    transform=axs[0, 0].transAxes,
    ha="center",
    fontsize=12,
    bbox=dict(fc="white", ec="none", alpha=0.9),
)
axs[0, 1].text(
    0.5,
    0.92,
    "FWHM  1.5° for all three grids*",
    transform=axs[0, 1].transAxes,
    ha="center",
    fontsize=12,
    bbox=dict(fc="white", ec="none", alpha=0.9),
)
axs[0, 0].legend(loc="lower left", fontsize=11, framealpha=0.92)
axs[0, 1].legend(loc="lower left", fontsize=11, framealpha=0.92)
x = np.arange(3)
for ax, kind in zip(axs[1], ["before", "after"]):
    if kind == "before":
        means = np.array([OLD[f"mean_energy_{n}_source"][n // 2] for n in grid])
        sems = np.array([OLD[f"sem_energy_{n}_source"][n // 2] for n in grid])
        expected = np.array([OLD[f"expected_{n}_source"][n // 2] for n in grid])
        color = "#a33d32"
    else:
        means = np.array([s["native_mean_on_axis"] for s in summary])
        sems = np.array([s["sem_on_axis"] for s in summary])
        expected = np.array([s["expected_on_axis"] for s in summary])
        color = "#087f6b"
    reference = expected[0]
    ax.plot(x, expected / reference, color=color, lw=2, label="Conditional mean")
    ax.errorbar(
        x,
        means / reference,
        yerr=sems / reference,
        fmt="o",
        ms=8,
        capsize=5,
        color=color,
        label="256 native pings ± 1 SEM",
    )
    ax.set_xticks(x, [str(n) for n in grid])
    ax.set_xlabel("Geometry samples (same scene)")
    ax.set_ylabel("Relative central-beam energy")
    ax.set_ylim(0, 1.3)
    ax.set_xlim(-0.3, 2.3)
    ax.grid(alpha=0.15)
    ax.legend(fontsize=10, loc="lower left" if kind == "after" else "upper right")
    if kind == "before":
        ax.set_title("Echo fades as the grid is refined", fontsize=15, pad=13)
        ax.annotate(
            "Only 6.3% of coarse-grid expectation",
            xy=(2, expected[-1] / reference),
            xytext=(0.12, 0.43),
            textcoords="axes fraction",
            fontsize=11,
            color=color,
            arrowprops={"arrowstyle": "->", "color": color},
        )
    else:
        ax.set_title("Mean echo energy is conserved", fontsize=15, pad=13)
        change = 100 * max(abs(expected / reference - 1))
        ax.text(
            0.5,
            0.63,
            f"Expected change across grids: {change:.4f}%",
            transform=ax.transAxes,
            ha="center",
            fontsize=12,
            weight="bold",
            color=color,
        )
fig.text(
    0.08,
    0.096,
    "* 1.5° is a synthetic control pattern, not a Sonoptix calibration. Proposed output: 301 fixed test directions.",
    fontsize=11,
    color="#475569",
)
fig.text(
    0.08,
    0.065,
    "Before: source response model + retained native means. Proposed: new native v2 output. Energy normalized to each model’s 65-sample expectation.",
    fontsize=10,
    color="#475569",
)
fig.text(
    0.08,
    0.03,
    "DRAFT · Native compute path verified · Sensor/demo wiring and useful SLAM-map acceptance remain open",
    fontsize=12,
    weight="bold",
    color="#475569",
)
fig.savefig(OUT / "readout-comparison.png", dpi=160, facecolor="white")
fig.savefig(OUT / "readout-comparison.svg", facecolor="white")
plt.close(fig)
p = OUT / "readout-comparison.svg"
p.write_text("\n".join(line.rstrip() for line in p.read_text().splitlines()) + "\n")
