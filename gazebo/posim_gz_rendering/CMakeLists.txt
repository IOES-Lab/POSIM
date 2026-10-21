cmake_minimum_required(VERSION 3.22)
project(posim_gz_rendering LANGUAGES CXX)
find_package(gz-rendering REQUIRED COMPONENTS ogre2)
find_package(gz-sim REQUIRED)
find_package(gz-plugin REQUIRED COMPONENTS register)
find_package(PkgConfig REQUIRED)
pkg_check_modules(OGRE REQUIRED IMPORTED_TARGET OGRE-Next>=2.3)
pkg_check_modules(HLMS REQUIRED IMPORTED_TARGET OGRE-Next-Hlms)
find_path(
  HLMS_COMMON OgreHlmsTextureBaseClass.h
  HINTS ${HLMS_INCLUDE_DIRS} ${OGRE_INCLUDE_DIRS}
  PATH_SUFFIXES Common ../Common Hlms/Common include/OGRE-Next/Hlms/Common
                OGRE-Next/Hlms/Common REQUIRED)
add_library(posim_underwater_render SHARED UnderwaterRenderer.cc)
target_compile_features(posim_underwater_render PRIVATE cxx_std_17)
target_include_directories(posim_underwater_render PRIVATE ${HLMS_COMMON})
target_link_libraries(
  posim_underwater_render
  PRIVATE gz-rendering::gz-rendering gz-rendering::ogre2 PkgConfig::OGRE
          PkgConfig::HLMS)
add_library(posim_underwater SHARED UnderwaterSystem.cc)
target_compile_features(posim_underwater PRIVATE cxx_std_17)
target_link_libraries(
  posim_underwater PRIVATE gz-sim::gz-sim gz-plugin::register
                           gz-rendering::gz-rendering ${CMAKE_DL_LIBS})
set_target_properties(
  posim_underwater posim_underwater_render
  PROPERTIES PREFIX "")
install(TARGETS posim_underwater posim_underwater_render
        LIBRARY DESTINATION lib)
install(DIRECTORY shaders/ DESTINATION share/posim/underwater)
include(CTest)
if(BUILD_TESTING)
  add_executable(underwater-settings-test tests/AppearanceTest.cc)
  target_compile_features(underwater-settings-test PRIVATE cxx_std_17)
  target_include_directories(underwater-settings-test PRIVATE .)
  add_test(NAME underwater-settings COMMAND underwater-settings-test)
  add_executable(underwater-render-test tests/RenderTest.cc)
  target_compile_features(underwater-render-test PRIVATE cxx_std_17)
  target_include_directories(underwater-render-test PRIVATE .)
  target_link_libraries(
    underwater-render-test PRIVATE posim_underwater_render
                                   gz-rendering::gz-rendering)
endif()
