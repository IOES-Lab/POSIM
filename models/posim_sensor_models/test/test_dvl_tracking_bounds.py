"""Catch DVL configurations rejected by Gazebo before publication starts."""

from pathlib import Path
import xml.etree.ElementTree as ET

import pytest

DESCRIPTION = Path(__file__).resolve().parents[1] / "description"
DVL_MODELS = [p for p in sorted(DESCRIPTION.glob("*/model.sdf")) if "<gz:dvl>" in p.read_text()]


def test_dvl_catalog_is_present():
    assert DVL_MODELS, "No DVL descriptors found; check the test resource path"


@pytest.mark.parametrize("path", DVL_MODELS, ids=lambda p: p.parent.name)
def test_water_mass_layer_fits_sensor_range(path):
    root = ET.parse(path).getroot()
    for dvl in root.iter("{http://gazebosim.org}dvl"):
        minimum = float(dvl.findtext("minimum_range"))
        maximum = float(dvl.findtext("maximum_range"))
        assert 0 <= minimum < maximum
        mode = dvl.find("tracking/water_mass_mode")
        if mode is None or mode.findtext("when") == "never":
            continue
        near = float(mode.findtext("boundaries/near", str(0.2 * maximum)))
        far = float(mode.findtext("boundaries/far", str(0.8 * maximum)))
        assert minimum <= near < far <= maximum, (
            f"{path.parent.name}: water-mass [{near}, {far}] exceeds "
            f"sensor range [{minimum}, {maximum}]"
        )
