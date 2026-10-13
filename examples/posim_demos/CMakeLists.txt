cmake_minimum_required(VERSION 3.8)
project(posim_demos)

find_package(ament_cmake REQUIRED)

install(
  DIRECTORY launch
  DESTINATION share/posim_demos
)

if(BUILD_TESTING)
  find_package(ament_cmake_pytest REQUIRED)
  ament_add_pytest_test(test_world_paths test/test_world_paths.py)
endif()

ament_package()
