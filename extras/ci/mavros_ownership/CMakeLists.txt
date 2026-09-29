cmake_minimum_required(VERSION 3.16)
project(posim_mavros_ownership LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
find_package(mavros REQUIRED)
find_package(rclcpp REQUIRED)
find_package(libmavconn REQUIRED)
find_package(diagnostic_updater REQUIRED)
find_package(mavros_msgs REQUIRED)
find_package(mavlink REQUIRED)
find_package(Eigen3 REQUIRED)
add_executable(posim_mavros_ownership_check main.cc)
target_include_directories(posim_mavros_ownership_check PRIVATE
  ${mavros_INCLUDE_DIRS} ${libmavconn_INCLUDE_DIRS} ${mavlink_INCLUDE_DIRS})
target_link_libraries(posim_mavros_ownership_check PRIVATE
  ${mavros_LIBRARIES} ${libmavconn_LIBRARIES} ${mavros_msgs_TARGETS}
  rclcpp::rclcpp diagnostic_updater::diagnostic_updater Eigen3::Eigen)
install(TARGETS posim_mavros_ownership_check DESTINATION bin)
