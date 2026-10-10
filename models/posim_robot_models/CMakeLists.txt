cmake_minimum_required(VERSION 3.8)
project(posim_robot_models)

find_package(ament_cmake REQUIRED)

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

ament_environment_hooks(
  "${CMAKE_CURRENT_SOURCE_DIR}/hooks/${PROJECT_NAME}.dsv.in")

if(BUILD_TESTING)
  find_package(ament_cmake_pytest REQUIRED)
  ament_add_pytest_test(
    test_manual_control_buttons test/test_manual_control_buttons.py)
  ament_add_pytest_test(
    test_manual_control_services test/test_manual_control_services.py)
  ament_add_pytest_test(test_ws_lifecycle test/test_ws_lifecycle.py)
endif()

ament_package()
