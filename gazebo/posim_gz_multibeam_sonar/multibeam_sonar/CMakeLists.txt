cmake_minimum_required(VERSION 3.18 FATAL_ERROR)
if(POLICY CMP0144)
  cmake_policy(SET CMP0144 NEW)
endif()

project(multibeam_sonar)

# Preserve explicit settings from existing workspaces while using CMake's
# architecture property for native CUDA targets.
if(DEFINED CUDA_ARCHITECTURE AND NOT DEFINED CMAKE_CUDA_ARCHITECTURES)
  set(CMAKE_CUDA_ARCHITECTURES "${CUDA_ARCHITECTURE}"
    CACHE STRING "CUDA architectures")
  message(DEPRECATION
    "Use CMAKE_CUDA_ARCHITECTURES instead of CUDA_ARCHITECTURE")
endif()

find_package(ament_cmake REQUIRED)
find_package(CUDAToolkit QUIET)

if(CUDAToolkit_FOUND)
  if(NOT CMAKE_CUDA_COMPILER)
    set(CMAKE_CUDA_COMPILER "${CUDAToolkit_NVCC_EXECUTABLE}"
      CACHE FILEPATH "CUDA compiler")
  endif()
  enable_language(CUDA)
  message(STATUS "CUDA found, enabling CUDA support.")
  find_package(tinyxml2 CONFIG REQUIRED)
  find_package(gz-cmake 5 REQUIRED)
  find_package(gz-sim 10 REQUIRED)
  find_package(gz-sensors 10 REQUIRED)
  find_package(gz-rendering 10 REQUIRED OPTIONAL_COMPONENTS ogre ogre2)
  find_package(gz-transport 15 REQUIRED)
  find_package(gz-msgs 12 REQUIRED)
  find_package(rclcpp REQUIRED)
  find_package(sensor_msgs REQUIRED)
  find_package(geometry_msgs REQUIRED)
  find_package(OpenCV REQUIRED)
  find_package(marine_acoustic_msgs REQUIRED)
  find_package(cv_bridge REQUIRED)

  if(TARGET gz-rendering::ogre)
    set(HAVE_OGRE TRUE)
    add_definitions(-DWITH_OGRE)
  endif()

  if(TARGET gz-rendering::ogre2)
    set(HAVE_OGRE2 TRUE)
    add_definitions(-DWITH_OGRE2)
  endif()

  add_library(${PROJECT_NAME}
    SHARED
      MultibeamSonarSensor.cc
      sonar_calculation_cuda.cu
  )

  set_target_properties(${PROJECT_NAME}
    PROPERTIES
    CUDA_SEPARABLE_COMPILATION ON
    CUDA_STANDARD 17
    CUDA_STANDARD_REQUIRED ON
  )
  target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)

  target_include_directories(${PROJECT_NAME}
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include/${PROJECT_NAME}>
    ${gz-sensors_INCLUDE_DIRS}
    ${gz-rendering_INCLUDE_DIRS}
    ${gz-msgs_INCLUDE_DIRS}
    ${gz-transport_INCLUDE_DIRS}
    ${gz-sim_INCLUDE_DIRS}
    ${OpenCV_INCLUDE_DIRS}
  )

  target_link_libraries(${PROJECT_NAME}
    gz-sim::gz-sim
    gz-sensors::gz-sensors
    gz-msgs::gz-msgs
    gz-transport::gz-transport
    gz-rendering::gz-rendering
    CUDA::cudart
    CUDA::cufft
    CUDA::cublas
    rclcpp::rclcpp
    ${sensor_msgs_TARGETS}
    ${geometry_msgs_TARGETS}
    ${marine_acoustic_msgs_TARGETS}
    cv_bridge::cv_bridge
    ${OpenCV_LIBRARIES}
  )

  if(TARGET gz-sensors::gz-sensors-rendering)
    target_link_libraries(${PROJECT_NAME} gz-sensors::gz-sensors-rendering)
  endif()

  install(TARGETS ${PROJECT_NAME}
    EXPORT export_${PROJECT_NAME}
    DESTINATION lib/${PROJECT_NAME}
  )
  install(FILES MultibeamSonarSensor.hh AcousticBeam.hh AxisAlignedPatch2.hh
    DESTINATION include/${PROJECT_NAME}
  )
  ament_export_targets(export_${PROJECT_NAME} HAS_LIBRARY_TARGET)
  ament_export_dependencies(
    CUDAToolkit gz-sim gz-sensors gz-msgs gz-transport gz-rendering
    rclcpp sensor_msgs geometry_msgs OpenCV marine_acoustic_msgs cv_bridge
  )

  ament_environment_hooks(
    "${CMAKE_CURRENT_SOURCE_DIR}/hooks/${PROJECT_NAME}.dsv.in"
  )

else()
  message(STATUS "CUDA Toolkit not found or disabled: "
                "Skipping CUDA-specific targets")
endif()

if(BUILD_TESTING)
  find_package(ament_cmake_gtest REQUIRED)
  ament_add_gtest(test_sonar_log test/test_sonar_log.cc)
  target_include_directories(test_sonar_log PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
  target_compile_features(test_sonar_log PRIVATE cxx_std_17)
  if(TARGET ${PROJECT_NAME})
    find_package(ament_cmake_pytest REQUIRED)
    ament_add_pytest_test(test_library_load test/test_library_load.py
      ENV "POSIM_SONAR_LIBRARY=$<TARGET_FILE:${PROJECT_NAME}>")
  endif()
endif()

ament_package()
