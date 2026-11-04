"""REXROV's declared propeller moments must reach the physics model."""

from pathlib import Path
import subprocess
import xml.etree.ElementTree as ET

import pytest


@pytest.fixture(scope="module")
def parsed_rexrov():
    model = Path(__file__).parents[1] / "description" / "rexrov" / "model.sdf"
    result = subprocess.run(
        ["gz", "sdf", "-p", str(model)], capture_output=True, text=True, timeout=30, check=True
    )
    return ET.fromstring(result.stdout).find("model")


@pytest.mark.parametrize("thruster", range(1, 9))
def test_thruster_moments_are_not_sdf_defaults(parsed_rexrov, thruster):
    inertia = parsed_rexrov.find(f"link[@name='thruster{thruster}']/inertial/inertia")
    assert inertia is not None
    # Bare ixx="..." text inside <inertia> is silently ignored by SDFormat.
    # Verify the parsed model, which is what the physics engine consumes.
    for axis in ("ixx", "iyy", "izz"):
        assert float(inertia.findtext(axis)) == pytest.approx(0.0005)
    for axis in ("ixy", "ixz", "iyz"):
        assert float(inertia.findtext(axis)) == 0
