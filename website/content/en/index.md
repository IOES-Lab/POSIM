# Platform for Ocean Simulation

POSIM is an open-source maritime robotics simulation library for ROS 2 and Gazebo. Run ocean environments, surface and underwater robots, simulated sensors and control experiments directly in your workspace. POSIM derives from [Project DAVE](https://field-robotics-lab.github.io/dave.doc/), which builds on [UUV Simulator](https://uuvsimulator.github.io/).

<figure class="docs-abstract"><div class="docs-abstract-grid"><a href="rovs.html" aria-label="ROV · BlueROV2"><img width="577" height="797" src="{{ASSET_PREFIX}}media/overview/bluerov2.png" alt="ROV · BlueROV2" decoding="async"><span>ROV · BlueROV2</span></a><a href="surface.html" aria-label="Surface robot · WAM-V"><img width="1280" height="720" src="{{ASSET_PREFIX}}media/overview/wamv.jpg" alt="Surface robot · WAM-V" decoding="async"><span>Surface robot · WAM-V</span></a><a href="objects.html" aria-label="Seabed · Task scenes"><img width="2200" height="1650" src="{{ASSET_PREFIX}}media/notion/camera-6eec9419.png" alt="Seabed · Task scenes" decoding="async"><span>Seabed · Task scenes</span></a><a href="sonar.html" aria-label="Sonar · Imagery and point clouds"><img width="1738" height="1066" src="{{ASSET_PREFIX}}media/notion/sonar-f4ac9419.png" alt="Sonar · Imagery and point clouds" decoding="async"><span>Sonar · Imagery and point clouds</span></a><a href="dvl.html" aria-label="DVL · Velocity sensing"><img width="1417" height="846" src="{{ASSET_PREFIX}}media/notion/dvl-e7cc9419.png" alt="DVL · Velocity sensing" decoding="async"><span>DVL · Velocity sensing</span></a><a href="camera.html" aria-label="Camera · Underwater vision"><img width="2202" height="1650" src="{{ASSET_PREFIX}}media/notion/camera-f14c9419.png" alt="Camera · Underwater vision" decoding="async"><span>Camera · Underwater vision</span></a></div><figcaption>Ocean environments → robots and dynamics → sensor observations → ROS 2 control. Sensor figures from the POSIM Notion Wiki and POSIM robot scenes captured in WWW-POSIM. Select a panel to open its guide.</figcaption></figure>

## Start here

<div class="docs-architecture"><a href="install.html"><strong>Install</strong><span>Build the ROS 2 and Gazebo workspace on Ubuntu.</span></a><a href="quickstart.html"><strong>Run</strong><span>Start an ocean world and receive sensor data.</span></a><a href="custom-robots.html"><strong>Extend</strong><span>Add robots, terrain and control code.</span></a></div>

Prepare an [Ubuntu installation](install.md) or [Docker environment](docker.md), then run the [first simulation](quickstart.md).

- Operating system: **Ubuntu 26.04**
- ROS: **ROS 2 Lyrical**
- Simulator: **Gazebo Jetty**

## Library components

The [public feature catalogue](features.md) maps environment effects, sensors, robot integrations and related libraries to their source and usage guides.

| Component | Purpose | Guide |
| --- | --- | --- |
| Ocean worlds | Surface, seabed and task scenes | [World library](worlds.md) |
| Robots | REXROV, BlueROV2, Slocum and surface vehicles | [ROVs](rovs.md), [Gliders](gliders.md), [Surface robots](surface.md) |
| Underwater rendering | Haze, absorption, sunlight, caustics, marine snow and seabed overlays | [Scene effects and controls](underwater-rendering.md) |
| Sensors | Camera, marine GNSS, DVL, pressure, USBL and CUDA sonar | [Camera](camera.md), [GNSS](gnss.md), [DVL](dvl.md), [Sonar](sonar.md) |
| Environment plugins | Currents and geographic coordinates | [Currents](currents.md), [Coordinates](coordinates.md) |
| ROS interfaces | Sensor subscriptions, commands and recording | [ROS 2 and control](ros.md) |

## Simulation structure

1. Define the environment and world systems in world SDF.
2. Define links, collision, inertia and sensors in model SDF.
3. Launch Gazebo, the model and ROS bridges.
4. Subscribe to sensors and send commands from ROS nodes.

## Find a guide

- **Examples:** commands and data checks
- **Advanced guides:** customize models and terrain, connect controllers and tune sensor experiments
- **Plugin reference:** quickstarts, SDF examples, parameters, units and topics
- **[Contributing](contributing.md):** source/documentation updates and checks

Each page links related source and reference material in its footer.

## Related projects

[POSIM-Terrain, POSIM-Routing and POSIM-Control](libraries.md) provide geographic terrain generation, route planning and vehicle control. Each library has its own usage guide.

[WWW-POSIM](https://www-posim.vercel.app/) is an optional web and desktop application built with POSIM. Follow this site's installation and simulation guides to use POSIM in your own ROS workspace.
