# Platform for Ocean Simulation

POSIM is a ROS 2 and Gazebo library for maritime robotics. Combine ocean environments, robots, simulated sensors and control interfaces for repeatable experiments.

## Try WWW-POSIM

[WWW-POSIM](https://www-posim.vercel.app/) is an ocean robot simulator powered by POSIM. Use a web workspace or desktop app.

- Generate seabed and coast from latitude/longitude
- Operate surface/underwater robots and inspect sensors
- Test ArduPilot waypoint missions or ROS 2 code
- Watch the public LIVE world voyage

## Start here

<div class="docs-architecture"><a href="install.html"><strong>Install</strong><span>Build the ROS 2 and Gazebo workspace on Ubuntu.</span></a><a href="quickstart.html"><strong>Run</strong><span>Start an ocean world and receive sensor data.</span></a><a href="custom-robots.html"><strong>Extend</strong><span>Add robots, terrain and control code.</span></a></div>

Prepare an [Ubuntu installation](install.md) or [Docker environment](docker.md), then run the [first simulation](quickstart.md).

- Operating system: **Ubuntu 26.04**
- ROS: **ROS 2 Lyrical**
- Simulator: **Gazebo Jetty**

## Library components

| Component | Purpose | Guide |
| --- | --- | --- |
| Ocean worlds | Surface, seabed and task scenes | [World library](worlds.md) |
| Robots | REXROV, BlueROV2, Slocum and surface vehicles | [ROVs](rovs.md), [Gliders](gliders.md), [Surface robots](surface.md) |
| Sensors | Camera, DVL, pressure, USBL and CUDA sonar | [Camera](camera.md), [DVL](dvl.md), [Sonar](sonar.md) |
| Environment plugins | Currents and geographic coordinates | [Currents](currents.md), [Coordinates](coordinates.md) |
| ROS interfaces | Sensor subscriptions, commands and recording | [ROS 2 and control](ros.md) |

## Simulation structure

1. Define the environment and world systems in world SDF.
2. Define links, collision, inertia and sensors in model SDF.
3. Launch Gazebo, the model and ROS bridges.
4. Subscribe to sensors and send commands from ROS nodes.

Use POSIM directly in a ROS workspace. WWW-POSIM provides geographic world generation, a workbench and online sessions.

WWW-POSIM uses [POSIM-Terrain, POSIM-Routing and POSIM-Control](libraries.md) for terrain generation, route planning and vehicle control. Each is a public library with its own usage guide.

## Find a guide

- **Examples:** commands and data checks
- **Advanced guides:** models, terrain and build settings
- **Plugin reference:** values, units and topics
- **[Contributing](contributing.md):** source/documentation updates and checks

Each page links related source and reference material in its footer.
