# Platform for Ocean Simulation

POSIM is a ROS 2 and Gazebo library for maritime robotics. Combine ocean environments, vehicle descriptions, simulated sensors and control interfaces to build repeatable experiments.

## Start here

<div class="docs-architecture"><a href="install.html"><strong>Install</strong><span>Build the ROS 2 and Gazebo workspace on Ubuntu.</span></a><a href="quickstart.html"><strong>Run</strong><span>Start an ocean world and receive your first sensor data.</span></a><a href="custom-robots.html"><strong>Extend</strong><span>Add your robot, terrain and control code.</span></a></div>

Use [Ubuntu installation](install.md) for a native Linux workspace, or [Docker](docker.md) for an isolated environment. Then follow [First simulation](quickstart.md). The current source targets **Ubuntu 26.04, ROS 2 Lyrical and Gazebo Jetty**.

## What is in the library?

| Component | Purpose | Guide |
| --- | --- | --- |
| Ocean worlds | Assemble sea surface, seabed and task scenes | [World library](worlds.md) |
| Vehicles | REXROV, BlueROV2, Slocum and the surface integration | [ROVs](rovs.md), [glider](gliders.md), [surface robots](surface.md) |
| Sensors | Underwater camera, DVL, pressure, USBL and CUDA multibeam sonar | [Camera](camera.md), [DVL](dvl.md), [sonar](sonar.md) |
| Environmental plugins | Currents and spherical-coordinate services | [Currents](currents.md), [coordinates](coordinates.md) |
| ROS interfaces | Sensor subscriptions, vehicle control and experiment recording | [ROS 2 and control](ros.md) |

## How a simulation fits together

A world SDF defines the environment and world systems. A model SDF defines links, collisions, inertia, sensors and model systems. Launch files start Gazebo, spawn the selected description and configure ROS bridges. Your ROS node reads observations and publishes commands through the configured interface.

POSIM is the simulation library. Account management, online sessions and the WWW-POSIM web platform are separate applications built around it. You can use this library directly from a ROS workspace without either service.

## Find the right guide

The examples provide runnable entry points. Advanced guides cover custom models, terrain and build tuning. Plugin references explain parameters, units and topic names. Run one example at a time while learning the launch and resource layout.

This documentation adapts the IOES-Lab POSIM Notion Wiki to the current source layout. Each page links its reference material and source revision in the footer. [Contribution guidance](contributing.md) explains how to update the documentation alongside code changes.
