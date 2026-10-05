cmake_minimum_required(VERSION 3.8)
project(posim_object_models)

find_package(ament_cmake REQUIRED)

install(
  DIRECTORY description launch
  DESTINATION share/posim_object_models
)

ament_environment_hooks(
  "${CMAKE_CURRENT_SOURCE_DIR}/hooks/${PROJECT_NAME}.dsv.in")

ament_package()
