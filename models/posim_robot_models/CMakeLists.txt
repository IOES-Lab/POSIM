cmake_minimum_required(VERSION 3.8)
project(posim_robot_models)

find_package(ament_cmake REQUIRED)
install(
  FILES ../../gazebo/posim_gz_gnss/config/profiles.json
  DESTINATION share/posim_robot_models/gnss
)

install(
  PROGRAMS
    scripts/ardusub_manual_control.py
    scripts/keyboard_publisher.py
    scripts/ws_to_joy.py
  DESTINATION lib/${PROJECT_NAME}
)

install(
  DIRECTORY description launch meshes config
  DESTINATION share/posim_robot_models
)

install(FILES ../../extras/virtual_joystick.html
  DESTINATION share/${PROJECT_NAME}/web)

ament_environment_hooks(
  "${CMAKE_CURRENT_SOURCE_DIR}/hooks/${PROJECT_NAME}.dsv.in")

if(BUILD_TESTING)
  find_package(ament_cmake_pytest REQUIRED)
  # These callback tests use mocked services and need no external ROS peers.
  # Keep their short-lived nodes out of concurrent tests' DDS discovery.
  ament_add_pytest_test(
    test_manual_control_buttons test/test_manual_control_buttons.py
    ENV ROS_AUTOMATIC_DISCOVERY_RANGE=OFF)
  ament_add_pytest_test(
    test_manual_control_services test/test_manual_control_services.py
    ENV ROS_AUTOMATIC_DISCOVERY_RANGE=OFF)
  ament_add_pytest_test(test_ws_lifecycle test/test_ws_lifecycle.py)
  ament_add_pytest_test(test_sim_clock_configs test/test_sim_clock_configs.py)
  ament_add_pytest_test(test_rexrov_inertias test/test_rexrov_inertias.py)
  ament_add_pytest_test(test_desktop_launch test/test_desktop_launch.py)
endif()

ament_package()
