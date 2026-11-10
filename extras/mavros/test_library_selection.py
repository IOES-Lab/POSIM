"""Check the dynamic loader uses the selected MAVROS installation."""

import ctypes
from pathlib import Path
import sys


def main():
    prefix = Path(sys.argv[1]).resolve()
    for name in ("libmavros.so", "libmavros_plugins.so"):
        ctypes.CDLL(name)
        paths = {
            Path(line.split()[-1]).resolve()
            for line in Path("/proc/self/maps").read_text().splitlines()
            if line.split()[-1].endswith("/" + name)
        }
        if not paths or any(not path.is_relative_to(prefix) for path in paths):
            raise SystemExit(f"{name} loaded outside {prefix}: {sorted(map(str, paths))}")
        print(f"{name}: {', '.join(map(str, sorted(paths)))}")


if __name__ == "__main__":
    main()
