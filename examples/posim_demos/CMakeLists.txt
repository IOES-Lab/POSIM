cmake_minimum_required(VERSION 3.8)
project(posim_demos)

find_package(ament_cmake REQUIRED)

install(
  DIRECTORY launch
  DESTINATION share/posim_demos
)

ament_package()
