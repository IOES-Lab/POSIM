cmake_minimum_required(VERSION 3.8)
project(posim_demos)

find_package(ament_cmake REQUIRED)
install(PROGRAMS scripts/sonar_display.py DESTINATION lib/${PROJECT_NAME})

install(
  DIRECTORY launch rviz
  DESTINATION share/posim_demos
)

install(
  FILES ../../extras/surface/wamv.launch.py ../../extras/surface/wamv.py
        ../../extras/surface/waves.py
        ../../extras/surface/external_navigation.py
        ../../extras/surface/rover.parm ../../extras/surface/mavros.yaml
  DESTINATION share/posim_demos/surface
)

if(BUILD_TESTING)
  find_package(ament_cmake_pytest REQUIRED)
  ament_add_pytest_test(test_world_paths test/test_world_paths.py)
  ament_add_pytest_test(test_ned_frames test/test_ned_frames.py)
endif()

ament_package()
