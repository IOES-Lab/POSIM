#!/usr/bin/env python3
"""Plot a recorded sonar frame and selected beam profiles."""

import argparse
from pathlib import Path

import numpy as np


def load_recording(data_path, angles_path):
    rows = []
    with data_path.open() as stream:
        for number, line in enumerate(stream, start=1):
            if not line.strip() or line.lstrip().startswith("#"):
                continue
            try:
                rows.append([complex(value.replace("i", "j")) for value in line.split(",")])
            except ValueError as error:
                raise ValueError(f"invalid complex value on CSV line {number}") from error
    try:
        data = np.asarray(rows, dtype=complex)
    except ValueError as error:
        raise ValueError("CSV rows must have the same number of columns") from error
    if data.ndim != 2 or data.shape[0] < 2 or data.shape[1] < 2:
        raise ValueError("CSV needs at least two range rows and one echo column")
    if not np.isfinite(data).all() or np.any(data[:, 0].imag != 0):
        raise ValueError("CSV values must be finite and ranges must be real")
    ranges = data[:, 0].real
    if np.any(ranges < 0) or np.any(np.diff(ranges) <= 0):
        raise ValueError("ranges must be nonnegative and strictly increasing")
    angles = np.atleast_1d(np.loadtxt(angles_path, comments="#"))
    if angles.ndim != 1 or angles.size != data.shape[1] - 1:
        raise ValueError("beam-angle count must match the number of echo columns")
    if not np.isfinite(angles).all():
        raise ValueError("beam angles must be finite")
    return ranges, angles, data[:, 1:]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("data", nargs="?", type=Path, default=Path("SonarRawData_000001.csv"))
    parser.add_argument("--angles", type=Path, help="defaults to the beam-angle CSV beside data")
    parser.add_argument("--range-max", type=float, help="maximum plotted range in meters")
    parser.add_argument(
        "--beam-step", type=int, default=1, help="plot every Nth beam (default: 1)"
    )
    parser.add_argument(
        "--dynamic-range", type=float, default=60, help="display range below the frame peak, in dB"
    )
    parser.add_argument(
        "--output-dir", type=Path, help="save the sonar and beam-profile PNGs here"
    )
    parser.add_argument(
        "--no-show", action="store_true", help="render without opening plot windows"
    )
    args = parser.parse_args()
    if args.beam_step < 1:
        parser.error("--beam-step must be positive")
    if not np.isfinite(args.dynamic_range) or args.dynamic_range <= 0:
        parser.error("--dynamic-range must be finite and positive")
    if args.range_max is not None and (not np.isfinite(args.range_max) or args.range_max <= 0):
        parser.error("--range-max must be finite and positive")
    if args.no_show and args.output_dir is None:
        parser.error("--no-show requires --output-dir")
    angles_path = args.angles or args.data.with_name("SonarRawData_beam_angles.csv")
    try:
        ranges, angles, echoes = load_recording(args.data, angles_path)
    except (OSError, ValueError) as error:
        parser.error(str(error))
    maximum = args.range_max or ranges[-1]
    selected = ranges <= maximum
    if not selected.any():
        parser.error("no recorded ranges fall within --range-max")
    ranges, echoes = ranges[selected], echoes[selected]

    import matplotlib

    if args.no_show:
        matplotlib.use("Agg")
    import matplotlib.pyplot as plt
    from matplotlib.ticker import MaxNLocator

    amplitude = np.abs(echoes)
    peak = max(float(amplitude.max()), np.finfo(float).tiny)
    relative_db = 20 * np.log10(np.maximum(amplitude / peak, 1e-12))
    range_grid, beam_grid = np.meshgrid(ranges, angles[:: args.beam_step], indexing="ij")
    x, y = range_grid * np.cos(beam_grid), range_grid * np.sin(beam_grid)
    sonar, axis = plt.subplots(figsize=(6, 8), layout="compressed")
    scatter = axis.scatter(
        x.ravel(),
        y.ravel(),
        s=8,
        c=relative_db[:, :: args.beam_step].ravel(),
        cmap="hot",
        marker="o",
        vmin=-args.dynamic_range,
        vmax=0,
    )
    sonar.colorbar(scatter, ax=axis, label="Relative echo level (dB)")
    axis.set(
        title=f"{args.data.name}\n{angles.size} beams · {ranges[-1]:.2f} m recorded range",
        xlabel="X [m]",
        ylabel="Y [m]",
        facecolor="black",
    )
    axis.set_aspect("equal", adjustable="box")
    axis.set_xlim(min(0, float(x.min())), max(0.1, float(x.max())) * 1.02)
    extent = max(0.1, float(np.abs(y).max())) * 1.02
    axis.set_ylim(-extent, extent)

    indices = np.unique(np.linspace(0, angles.size - 1, min(5, angles.size), dtype=int))
    profiles, axes = plt.subplots(
        1,
        len(indices),
        figsize=(max(4, 2.6 * len(indices)), 4),
        sharey=True,
        squeeze=False,
        layout="constrained",
    )
    for column, index in enumerate(indices):
        profile = axes[0, column]
        profile.plot(amplitude[:, index], ranges)
        profile.set(
            title=f"Beam {index + 1} · {np.degrees(angles[index]):.1f}°", xlabel="Echo amplitude"
        )
        profile.xaxis.set_major_locator(MaxNLocator(3))
        profile.ticklabel_format(axis="x", style="sci", scilimits=(0, 0))
    axes[0, 0].set_ylabel("Range [m]")
    profiles.suptitle("Selected beam profiles")

    print(f"{args.data}: {ranges.size} ranges × {angles.size} beams")
    if args.output_dir:
        args.output_dir.mkdir(parents=True, exist_ok=True)
        for figure, suffix in [(sonar, "sonar"), (profiles, "beams")]:
            path = args.output_dir / f"{args.data.stem}-{suffix}.png"
            figure.savefig(path, dpi=160)
            print(f"Saved {path}")
    if not args.no_show:
        plt.show()
    plt.close("all")


if __name__ == "__main__":
    main()
