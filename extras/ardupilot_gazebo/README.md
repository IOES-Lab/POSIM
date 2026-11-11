# ArduPilot sensor resampling

This overlay adds `sitl_sensor_rate` to the pinned upstream `ArduPilotPlugin`.
The setting controls the target JSON sensor sample rate in Hz. Positive rates up to
2000 Hz require `lock_step=1`. Zero disables resampling. WAM-V selects 200 Hz,
and the BlueROV2 models select 1000 Hz.

Gazebo computes the physical state at its configured timestep. Between two
completed physics states, the plugin interpolates acceleration, angular
velocity, position and velocity. It interpolates orientation along the shortest
quaternion arc. Other measurements retain their preceding values until the next
physical state.

The sample count follows the actual physics timestamps, including timestep
changes. Each interval uses `max(1, ceil(duration * rate))` samples. At 200 Hz,
10, 20 and 30 ms intervals produce 2, 4 and 6 samples. A 7 ms interval produces
2 samples spaced 3.5 ms apart. An interval shorter than the target sample period
needs only its original physical sample. The timestamps still reach the exact
end of each physics interval.

Each intermediate sample has a simulation timestamp between the two physical
timestamps. The final sample is the unchanged current state. The plugin waits
for SITL's servo response before sending the next sample and uses the latest
motor command for the next physics update. A clock reset or a gap longer than
0.1 seconds starts a new interpolation interval.

This allows a 20 ms physics timestep to supply more frequent SITL sensor
samples. The physical information still has 50 Hz resolution; interpolation
does not recover faster motion. Measure tracking, stop behavior and real-time
factor for the intended vehicle and environment.

The POSIM installers apply the overlay before compiling the plugin. When
assembling a separate engine, apply it to the selected upstream source and
rebuild the plugin:

```sh
python3 extras/ardupilot_gazebo/patch_resampling.py /path/to/ardupilot_gazebo
cmake --build /path/to/ardupilot_gazebo/build --target ArduPilotPlugin -j2
```

Run the interpolation checks with a C++17 compiler and RapidJSON headers:

```sh
c++ -std=c++17 -Wall -Wextra -Werror -Iextras/ardupilot_gazebo \
  extras/ardupilot_gazebo/test_resampling.cc -o /tmp/test_sensor_resampling
/tmp/test_sensor_resampling
```
