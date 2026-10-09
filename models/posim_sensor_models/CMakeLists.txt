cmake_minimum_required(VERSION 3.8)
project(posim_sensor_models)

find_package(ament_cmake REQUIRED)

install(
  DIRECTORY description launch meshes config
  DESTINATION share/posim_sensor_models
)

ament_environment_hooks(
  "${CMAKE_CURRENT_SOURCE_DIR}/hooks/${PROJECT_NAME}.dsv.in")

if(BUILD_TESTING)
  find_package(ament_cmake_pytest REQUIRED)
  ament_add_pytest_test(test_dvl_tracking_bounds
    test/test_dvl_tracking_bounds.py)
endif()

ament_package()
