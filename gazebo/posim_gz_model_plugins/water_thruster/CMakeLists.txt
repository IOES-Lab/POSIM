cmake_minimum_required(VERSION 3.16)
project(posim_water_thruster LANGUAGES CXX)

find_package(gz-sim 10 CONFIG REQUIRED)
find_package(gz-plugin 4 CONFIG REQUIRED COMPONENTS register)
find_package(gz-transport 15 CONFIG REQUIRED)
find_package(gz-msgs 12 CONFIG REQUIRED)
find_package(gz-waves1 CONFIG REQUIRED)

add_library(PosimWaterThruster SHARED PosimWaterThruster.cc DiskWaveIndex.cc)
target_compile_features(PosimWaterThruster PRIVATE cxx_std_20)
target_link_libraries(PosimWaterThruster PRIVATE
  gz-sim::gz-sim
  gz-plugin::gz-plugin-register
  gz-transport::gz-transport
  gz-msgs::gz-msgs
  gz-waves1::gz-waves1
)

include(GNUInstallDirs)
install(TARGETS PosimWaterThruster LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}")
