"""Loading the sonar plugin must not create or overwrite caller files."""

import os
import subprocess
import sys

import pytest


@pytest.mark.parametrize("existing_file", [False, True])
def test_library_load_preserves_working_directory(tmp_path, existing_file):
    sentinel = tmp_path / "debug_timings.txt"
    contents = b"Existing timing records\n"
    if existing_file:
        sentinel.write_bytes(contents)

    # Each case needs a fresh process so the shared library is loaded again.
    subprocess.run(
        [
            sys.executable,
            "-c",
            "import ctypes, sys; ctypes.CDLL(sys.argv[1])",
            os.environ["POSIM_SONAR_LIBRARY"],
        ],
        cwd=tmp_path,
        check=True,
        capture_output=True,
        timeout=30,
    )

    if existing_file:
        assert sentinel.read_bytes() == contents
        assert list(tmp_path.iterdir()) == [sentinel]
    else:
        assert list(tmp_path.iterdir()) == []
