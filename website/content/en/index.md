# Platform for Ocean Simulation

POSIM is an open-source maritime robotics simulation library for ROS 2 and Gazebo. Run ocean environments, surface and underwater robots, simulated sensors and control experiments directly in your workspace. POSIM derives from [Project DAVE](https://field-robotics-lab.github.io/dave.doc/), which builds on [UUV Simulator](https://uuvsimulator.github.io/).

<figure class="docs-abstract"><div class="docs-abstract-grid"><a href="rovs.html" aria-label="ROV · BlueROV2"><img width="577" height="797" src="{{ASSET_PREFIX}}media/overview/bluerov2.png" alt="ROV · BlueROV2" decoding="async"><span>ROV · BlueROV2</span></a><a href="surface.html" aria-label="Surface robot · WAM-V"><img width="1280" height="720" src="{{ASSET_PREFIX}}media/overview/wamv.jpg" alt="Surface robot · WAM-V" decoding="async"><span>Surface robot · WAM-V</span></a><a href="objects.html" aria-label="Seabed · Task scenes"><img width="2200" height="1650" src="{{ASSET_PREFIX}}media/notion/camera-6eec9419.png" alt="Seabed · Task scenes" decoding="async"><span>Seabed · Task scenes</span></a><a href="sonar.html" aria-label="Sonar · Imagery and point clouds"><img width="1738" height="1066" src="{{ASSET_PREFIX}}media/notion/sonar-f4ac9419.png" alt="Sonar · Imagery and point clouds" decoding="async"><span>Sonar · Imagery and point clouds</span></a><a href="dvl.html" aria-label="DVL · Velocity sensing"><img width="1417" height="846" src="{{ASSET_PREFIX}}media/notion/dvl-e7cc9419.png" alt="DVL · Velocity sensing" decoding="async"><span>DVL · Velocity sensing</span></a><a href="camera.html" aria-label="Camera · Underwater vision"><img width="2202" height="1650" src="{{ASSET_PREFIX}}media/notion/camera-f14c9419.png" alt="Camera · Underwater vision" decoding="async"><span>Camera · Underwater vision</span></a></div><figcaption>Ocean environments → robots and dynamics → sensor observations → ROS 2 control. Select a panel to open its guide.</figcaption></figure>

## Start here

<div class="docs-architecture"><a href="requirements.html"><strong>Install</strong><span>Choose an Ubuntu, macOS or Windows environment.</span></a><a href="quickstart.html"><strong>Run</strong><span>Start an ocean world and receive sensor data.</span></a><a href="custom-robots.html"><strong>Extend</strong><span>Add robots, terrain and control code.</span></a></div>

Choose your installation route in [Choose an environment](requirements.md), then follow [Prepare your terminals](terminals.md) and [First simulation](quickstart.md).

- ROS: **ROS 2 Lyrical** — LTS; support ends [May 2031](https://docs.ros.org/en/lyrical/Releases/Release-Lyrical-Luth.html)
- Gazebo Sim simulator: **Gazebo Jetty** — LTS; support ends [May 2031](https://gazebosim.org/docs/latest/releases/#release-list)

## Library components

Use [Features & examples](features.md) to find environment effects, sensors, robot integrations with their source and usage guides.

| Component | Purpose | Guide |
| --- | --- | --- |
| Ocean worlds | Surface, seabed and task scenes | [World library](worlds.md) |
| Robots | REXROV, BlueROV2, Slocum and surface vehicles | [ROVs](rovs.md), [Gliders](gliders.md), [Surface robots](surface.md) |
| Underwater rendering | Haze, absorption, sunlight, caustics, marine snow and seabed overlays | [Scene effects and controls](underwater-rendering.md) |
| Sensors | Camera, marine GNSS, DVL, pressure, USBL and sonar | [Camera](camera.md), [GNSS](gnss.md), [DVL](dvl.md), [Sonar](sonar.md) |
| Environment plugins | Currents and geographic coordinates | [Currents](currents.md), [Coordinates](coordinates.md) |
| ROS interfaces | Sensor subscriptions, commands and recording | [ROS 2 and control](ros.md) |

## Simulation structure

1. Define the environment and world systems in world SDF.
2. Define links, collision, inertia and sensors in model SDF.
3. Launch Gazebo, the model and ROS bridges.
4. Subscribe to sensors and send commands from ROS nodes.

## Find a guide

| Your next task | Guide |
|---|---|
| Start and stop one working simulation | [First simulation](quickstart.md) |
| Understand the ROS connection | [ROS terminals & topics](ros.md) |
| Write a controller | [Control with Python](python-control.md), then [Thrusters & joystick control](robot-control.md) |
| Display or save measurements | [View & record sensor data](visualization.md) |
| Change the experiment | [Create a world](custom-worlds.md), [Add a robot](custom-robots.md), [Add & configure sensors](custom-sensors.md) |
| Look up a plugin's exact settings | The **Plugin reference** group: quickstart, SDF, parameters and behavior |

Each page links related source and reference material in its footer.

## WWW-POSIM

[WWW-POSIM](https://www-posim.vercel.app/) brings POSIM together with POSIM-Terrain for geographic terrain, POSIM-Routing for route planning and POSIM-Control for vehicle control. It adds a graphical interface and web and desktop applications for preparing scenes, operating robots and viewing simulation data.

The introduction below shows the application's main features. For setup and use, follow the [WWW-POSIM documentation](https://www-posim.vercel.app/guide/).

<iframe class="docs-embed" title="WWW-POSIM introduction" src="https://www.youtube-nocookie.com/embed/_cQtl5JzN2E?rel=0" loading="lazy" referrerpolicy="strict-origin-when-cross-origin" allow="accelerometer; autoplay; clipboard-write; encrypted-media; gyroscope; picture-in-picture; web-share" allowfullscreen></iframe>

[Watch the introduction on YouTube](https://www.youtube.com/watch?v=_cQtl5JzN2E).
