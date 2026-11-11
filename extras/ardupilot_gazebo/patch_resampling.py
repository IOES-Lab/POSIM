"""Apply POSIM sensor resampling to the selected upstream Gazebo plugin."""

from pathlib import Path
import shutil
import sys

root = Path(sys.argv[1])
path = root / "src/ArduPilotPlugin.cc"
source = path.read_text()


def replace(before, after):
    global source
    if source.count(after) == 1:
        return
    if source.count(before) != 1:
        raise RuntimeError("Unsupported ArduPilotPlugin source: " + before)
    source = source.replace(before, after)


replace(
    '#include "ArduPilotPlugin.hh"', '#include "ArduPilotPlugin.hh"\n#include "SensorResampler.hh"'
)
replace(
    "#include <rapidjson/stringbuffer.h>",
    "#include <stdexcept>\n#include <rapidjson/stringbuffer.h>",
)
replace(
    "public: std::string json_str;",
    "public: std::string json_str;\n  public: posim::SensorResampler sensorResampler;\n  public: double sensorRate{0};",
)
replace(
    '  this->dataPtr->isLockStep =\n    sdfClone->Get("lock_step", this->dataPtr->isLockStep).first;',
    """  this->dataPtr->isLockStep =
    sdfClone->Get("lock_step", this->dataPtr->isLockStep).first;
  this->dataPtr->sensorRate = sdfClone->Get("sitl_sensor_rate", 0.0).first;
  if (!std::isfinite(this->dataPtr->sensorRate) ||
      this->dataPtr->sensorRate < 0 || this->dataPtr->sensorRate > 2000 ||
      (this->dataPtr->sensorRate > 0 && !this->dataPtr->isLockStep))
    throw std::invalid_argument("sitl_sensor_rate requires lock_step and a rate in [0, 2000] Hz");""",
)
replace(
    "        this->CreateStateJSON(t, _ecm);\n        this->SendState();",
    """        this->CreateStateJSON(t, _ecm);
        const auto samples = this->dataPtr->sensorResampler.Samples(
            this->dataPtr->json_str, this->dataPtr->sensorRate);
        for (size_t i = 0; i < samples.size(); ++i)
        {
            if (i > 0)
            {
                bool received = false;
                while (!(received = this->ReceiveServoPacket()) &&
                       this->dataPtr->arduPilotOnline && this->dataPtr->signal == 0) {}
                if (!received || this->dataPtr->signal != 0) break;
            }
            this->dataPtr->json_str = samples[i];
            this->SendState();
        }""",
)
path.write_text(source)
shutil.copy2(Path(__file__).with_name("SensorResampler.hh"), root / "include/SensorResampler.hh")
