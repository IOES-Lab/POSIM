cmake_minimum_required(VERSION 3.18 FATAL_ERROR)
if(POLICY CMP0144)
  cmake_policy(SET CMP0144 NEW)
endif()

project(multibeam_sonar)

# Keep explicit legacy architecture settings without overriding
# the native property.
if(DEFINED CUDA_ARCHITECTURE AND NOT DEFINED CMAKE_CUDA_ARCHITECTURES)
  set(CMAKE_CUDA_ARCHITECTURES "${CUDA_ARCHITECTURE}"
    CACHE STRING "CUDA architectures")
  message(DEPRECATION
    "Use CMAKE_CUDA_ARCHITECTURES instead of CUDA_ARCHITECTURE")
endif()

find_package(ament_cmake REQUIRED)
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
find_package(cv_bridge REQUIRED)
find_package(marine_acoustic_msgs REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(wgpu_vendor REQUIRED)
find_package(CUDAToolkit QUIET)

if(CUDAToolkit_FOUND)
  if(NOT CMAKE_CUDA_COMPILER)
    set(CMAKE_CUDA_COMPILER "${CUDAToolkit_NVCC_EXECUTABLE}"
      CACHE FILEPATH "CUDA compiler")
  endif()
  enable_language(CUDA)
  message(STATUS "CUDA found, enabling CUDA support.")
  if(CUDAToolkit_VERSION VERSION_GREATER_EQUAL 13)
    find_package(CCCL CONFIG REQUIRED
      HINTS "${CUDAToolkit_TARGET_DIR}/lib64/cmake/cccl"
            "${CUDAToolkit_TARGET_DIR}/lib/cmake/cccl")
  endif()
endif()

if(TARGET gz-rendering::ogre)
  add_definitions(-DWITH_OGRE)
endif()

if(TARGET gz-rendering::ogre2)
  add_definitions(-DWITH_OGRE2)
endif()

set(SONAR_SOURCES
  MultibeamSonarSensor.cc
  sonar_compute_cpu.cc
  PhysicalSonarCpu.cc
  sonar_compute_wgpu.cc
  sonar_compute_cuda.cc
)

if(CUDAToolkit_FOUND)
  list(APPEND SONAR_SOURCES sonar_calculation_cuda.cu)
endif()

add_library(${PROJECT_NAME} SHARED ${SONAR_SOURCES})
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)

if(CUDAToolkit_FOUND)
  set_target_properties(${PROJECT_NAME}
    PROPERTIES
      CUDA_SEPARABLE_COMPILATION ON
      CUDA_STANDARD 17
      CUDA_STANDARD_REQUIRED ON
  )
  target_compile_definitions(${PROJECT_NAME} PRIVATE POSIM_HAS_CUDA_BACKEND=1)
  if(CUDAToolkit_VERSION VERSION_GREATER_EQUAL 13)
    # Build-only: no installed header includes CCCL, so keep it out of
    # the exported link interface used by downstream packages.
    target_link_libraries(${PROJECT_NAME} $<BUILD_INTERFACE:CCCL::CCCL>)
  endif()
endif()

target_include_directories(${PROJECT_NAME} PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include/${PROJECT_NAME}>
  ${rclcpp_INCLUDE_DIRS}
  ${sensor_msgs_INCLUDE_DIRS}
  ${geometry_msgs_INCLUDE_DIRS}
  ${OpenCV_INCLUDE_DIRS}
  ${EIGEN3_INCLUDE_DIRS}
  ${marine_acoustic_msgs_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME}
  gz-sim::gz-sim
  gz-sensors::gz-sensors
  gz-msgs::gz-msgs
  gz-transport::gz-transport
  gz-rendering::gz-rendering
  rclcpp::rclcpp
  ${sensor_msgs_TARGETS}
  ${geometry_msgs_TARGETS}
  ${OpenCV_LIBS}
  Eigen3::Eigen
  ${marine_acoustic_msgs_TARGETS}
  cv_bridge::cv_bridge
)

if(APPLE)
  target_link_libraries(${PROJECT_NAME}
    "-framework Metal"
    "-framework QuartzCore"
  )
endif()

if(CUDAToolkit_FOUND)
  target_link_libraries(${PROJECT_NAME}
    CUDA::cudart CUDA::cufft CUDA::cublas)
endif()

set(_wgpu_target_linked FALSE)
if(TARGET wgpu::wgpu_vendor)
  target_link_libraries(${PROJECT_NAME} wgpu::wgpu_vendor)
  set(_wgpu_target_linked TRUE)
elseif(TARGET wgpu_vendor)
  target_link_libraries(${PROJECT_NAME} wgpu_vendor)
  set(_wgpu_target_linked TRUE)
endif()

# Ensure the Rust static archive is linked into this shared library. The
# exported interface target may not always propagate non-installed imported
# targets across package boundaries in overlay builds.
if(NOT _wgpu_target_linked AND DEFINED wgpu_vendor_DIR)
  get_filename_component(
    _wgpu_vendor_prefix "${wgpu_vendor_DIR}/../../.." ABSOLUTE)
  set(_sonar_wgpu_static "${_wgpu_vendor_prefix}/lib/libsonar_wgpu.a")
  if(EXISTS "${_sonar_wgpu_static}")
    target_link_libraries(${PROJECT_NAME} "${_sonar_wgpu_static}")
  endif()
endif()

if(TARGET gz-sensors::gz-sensors-rendering)
  target_link_libraries(${PROJECT_NAME} gz-sensors::gz-sensors-rendering)
endif()

if(BUILD_TESTING)
  find_package(ament_cmake_gtest REQUIRED)
  ament_add_gtest(test_sonar_log test/test_sonar_log.cc)
  target_include_directories(test_sonar_log PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
  target_compile_features(test_sonar_log PRIVATE cxx_std_17)
  find_package(ament_cmake_pytest REQUIRED)
  ament_add_pytest_test(test_library_load test/test_library_load.py
    ENV "POSIM_SONAR_LIBRARY=$<TARGET_FILE:${PROJECT_NAME}>")
  enable_testing()
  add_executable(cuda_ray_layout_test test/CudaRayLayout_TEST.cc)
  target_compile_features(cuda_ray_layout_test PRIVATE cxx_std_17)
  target_include_directories(cuda_ray_layout_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR})
  add_test(NAME cuda_ray_layout COMMAND cuda_ray_layout_test)

  add_executable(cuda_input_validation_test test/CudaInputValidation_TEST.cc)
  target_compile_features(cuda_input_validation_test PRIVATE cxx_std_17)
  target_include_directories(cuda_input_validation_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR} ${OpenCV_INCLUDE_DIRS})
  target_link_libraries(cuda_input_validation_test PRIVATE ${OpenCV_LIBS})
  add_test(NAME cuda_input_validation COMMAND cuda_input_validation_test)

  add_executable(test_ray_surface_normals test/test_ray_surface_normals.cc)
  target_compile_features(test_ray_surface_normals PRIVATE cxx_std_17)
  target_include_directories(test_ray_surface_normals PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR} ${OpenCV_INCLUDE_DIRS})
  target_link_libraries(test_ray_surface_normals PRIVATE ${OpenCV_LIBS})
  add_test(NAME ray_surface_normals COMMAND test_ray_surface_normals)

  add_executable(perspective_geometry_test test/PerspectiveGeometry_TEST.cc)
  target_compile_features(perspective_geometry_test PRIVATE cxx_std_17)
  target_include_directories(perspective_geometry_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR} ${OpenCV_INCLUDE_DIRS})
  target_link_libraries(perspective_geometry_test PRIVATE ${OpenCV_LIBS})
  add_test(NAME perspective_geometry COMMAND perspective_geometry_test)

  add_executable(scattering_range_test test/ScatteringRange_TEST.cc)
  target_compile_features(scattering_range_test PRIVATE cxx_std_17)
  target_include_directories(scattering_range_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR} ${OpenCV_INCLUDE_DIRS})
  target_link_libraries(scattering_range_test PRIVATE
    ${PROJECT_NAME} ${OpenCV_LIBS})
  add_test(NAME scattering_range_cpu COMMAND scattering_range_test cpu)
  add_executable(physical_readout_test test/PhysicalReadout_TEST.cc)
  target_compile_features(physical_readout_test PRIVATE cxx_std_17)
  target_include_directories(physical_readout_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR} ${OpenCV_INCLUDE_DIRS})
  target_link_libraries(physical_readout_test PRIVATE
    ${PROJECT_NAME} ${OpenCV_LIBS})
  add_test(NAME physical_readout_cpu COMMAND physical_readout_test cpu)

  option(POSIM_SONAR_GPU_TESTS
    "Run sonar numerical tests requiring a WGPU adapter" OFF)
  if(POSIM_SONAR_GPU_TESTS)
    add_test(NAME scattering_range_wgpu COMMAND scattering_range_test wgpu)
    add_test(NAME physical_readout_wgpu COMMAND physical_readout_test wgpu)
    set_tests_properties(scattering_range_wgpu physical_readout_wgpu PROPERTIES
      RUN_SERIAL TRUE TIMEOUT 60)
  endif()

endif()

install(TARGETS ${PROJECT_NAME}
  EXPORT export_${PROJECT_NAME}
  DESTINATION lib/${PROJECT_NAME}
)
install(FILES MultibeamSonarSensor.hh AcousticBeam.hh AxisAlignedPatch2.hh
  sonar_compute_backend.hh RangeSampling.hh RaySurfaceNormals.hh
  AcousticReadout.hh PerspectiveSonarGeometry.hh
  DESTINATION include/${PROJECT_NAME}
)
ament_export_targets(export_${PROJECT_NAME} HAS_LIBRARY_TARGET)
ament_export_dependencies(
  gz-sim gz-sensors gz-msgs gz-transport gz-rendering
  rclcpp sensor_msgs geometry_msgs OpenCV marine_acoustic_msgs cv_bridge
  Eigen3 wgpu_vendor
)
if(CUDAToolkit_FOUND)
  ament_export_dependencies(CUDAToolkit)
endif()

ament_environment_hooks(
  "${CMAKE_CURRENT_SOURCE_DIR}/hooks/${PROJECT_NAME}.dsv.in"
)

if(BUILD_TESTING)
  enable_testing()
  add_executable(range_sampling_test test/RangeSampling_TEST.cc)
  target_include_directories(range_sampling_test
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
  target_link_libraries(range_sampling_test ${PROJECT_NAME})
  add_test(NAME range_sampling COMMAND range_sampling_test)
endif()

if(BUILD_TESTING)
  option(POSIM_SONAR_RENDERING_TESTS
    "Run sonar lifecycle tests requiring an Ogre2 rendering device" OFF)
  if(POSIM_SONAR_RENDERING_TESTS)
    if(APPLE)
      enable_language(OBJCXX)
      set_source_files_properties(test/SensorLifecycle_TEST.cc
        PROPERTIES LANGUAGE OBJCXX)
    endif()
    add_executable(test_sensor_lifecycle test/SensorLifecycle_TEST.cc)
    target_compile_features(test_sensor_lifecycle PRIVATE cxx_std_17)
    target_include_directories(test_sensor_lifecycle PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(test_sensor_lifecycle PRIVATE ${PROJECT_NAME})
    if(APPLE)
      target_link_libraries(test_sensor_lifecycle PRIVATE "-framework Cocoa")
    endif()
    foreach(sonar_backend cpu wgpu)
      add_test(NAME sonar_scene_lifecycle_${sonar_backend}
        COMMAND test_sensor_lifecycle
          ${CMAKE_CURRENT_SOURCE_DIR}/test/lifecycle_sonar.sdf)
      set_tests_properties(sonar_scene_lifecycle_${sonar_backend} PROPERTIES
        ENVIRONMENT "POSIM_SONAR_COMPUTE_BACKEND=${sonar_backend}"
        RUN_SERIAL TRUE TIMEOUT 60)
      add_test(NAME sonar_physical_lifecycle_${sonar_backend}
        COMMAND test_sensor_lifecycle
          ${CMAKE_CURRENT_SOURCE_DIR}/test/physical_lifecycle_sonar.sdf)
      set_tests_properties(sonar_physical_lifecycle_${sonar_backend} PROPERTIES
        ENVIRONMENT "POSIM_SONAR_COMPUTE_BACKEND=${sonar_backend}"
        RUN_SERIAL TRUE TIMEOUT 60)
    endforeach()
    add_executable(test_wgpu_range_sampling test/WgpuRangeSampling_TEST.cc)
    target_compile_features(test_wgpu_range_sampling PRIVATE cxx_std_17)
    target_include_directories(test_wgpu_range_sampling PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(test_wgpu_range_sampling PRIVATE ${PROJECT_NAME})
    add_test(NAME sonar_wgpu_range_sampling COMMAND test_wgpu_range_sampling)
    set_tests_properties(sonar_wgpu_range_sampling PROPERTIES
      RUN_SERIAL TRUE TIMEOUT 120)
  endif()
endif()

ament_package()
