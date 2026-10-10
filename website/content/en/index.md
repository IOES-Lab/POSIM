# Platform for Ocean Simulation

POSIM is a ROS 2 and Gazebo library for maritime robotics, derived from [Project DAVE](https://field-robotics-lab.github.io/dave.doc/). DAVE builds on [UUV Simulator](https://uuvsimulator.github.io/). Combine ocean environments, robots, simulated sensors and control interfaces for repeatable experiments.

<figure class="docs-abstract"><div class="docs-abstract-grid"><a href="rovs.html" aria-label="ROV · BlueROV2"><img width="577" height="797" src="{{ASSET_PREFIX}}media/overview/bluerov2.png" alt="ROV · BlueROV2" decoding="async"><span>ROV · BlueROV2</span></a><a href="surface.html" aria-label="Surface robot · WAM-V"><img width="1280" height="720" src="{{ASSET_PREFIX}}media/overview/wamv.jpg" alt="Surface robot · WAM-V" decoding="async"><span>Surface robot · WAM-V</span></a><a href="objects.html" aria-label="Seabed · Task scenes"><img width="2200" height="1650" src="{{ASSET_PREFIX}}media/notion/camera-6eec9419.png" alt="Seabed · Task scenes" decoding="async"><span>Seabed · Task scenes</span></a><a href="sonar.html" aria-label="Sonar · Imagery and point clouds"><img width="1738" height="1066" src="{{ASSET_PREFIX}}media/notion/sonar-f4ac9419.png" alt="Sonar · Imagery and point clouds" decoding="async"><span>Sonar · Imagery and point clouds</span></a><a href="dvl.html" aria-label="DVL · Velocity sensing"><img width="1417" height="846" src="{{ASSET_PREFIX}}media/notion/dvl-e7cc9419.png" alt="DVL · Velocity sensing" decoding="async"><span>DVL · Velocity sensing</span></a><a href="camera.html" aria-label="Camera · Underwater vision"><img width="2202" height="1650" src="{{ASSET_PREFIX}}media/notion/camera-f14c9419.png" alt="Camera · Underwater vision" decoding="async"><span>Camera · Underwater vision</span></a></div><figcaption>Ocean environments → robots and dynamics → sensor observations → ROS 2 control. Sensor figures from the POSIM Notion Wiki and POSIM robot scenes captured in WWW-POSIM. Select a panel to open its guide.</figcaption></figure>

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
