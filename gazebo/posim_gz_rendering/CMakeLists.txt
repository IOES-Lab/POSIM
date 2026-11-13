cmake_minimum_required(VERSION 3.22)
project(posim_gz_rendering LANGUAGES CXX)
find_package(ament_cmake QUIET)
find_package(gz-rendering REQUIRED COMPONENTS ogre2)
find_package(gz-sim REQUIRED COMPONENTS gui)
find_package(gz-gui REQUIRED)
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
target_compile_definitions(
  posim_underwater
  PRIVATE POSIM_LIBRARY_SUFFIX="${CMAKE_SHARED_LIBRARY_SUFFIX}")
target_link_libraries(
  posim_underwater PRIVATE gz-sim::gz-sim gz-plugin::register
                           gz-rendering::gz-rendering ${CMAKE_DL_LIBS})
qt_add_resources(UNDERWATER_GUI_RESOURCES underwater.qrc)
add_library(posim_underwater_gui SHARED UnderwaterGui.cc
                                      ${UNDERWATER_GUI_RESOURCES})
target_compile_features(posim_underwater_gui PRIVATE cxx_std_17)
target_compile_definitions(
  posim_underwater_gui
  PRIVATE POSIM_LIBRARY_SUFFIX="${CMAKE_SHARED_LIBRARY_SUFFIX}")
target_link_libraries(
  posim_underwater_gui PRIVATE gz-sim::gui gz-gui::gz-gui gz-plugin::register
                               gz-rendering::gz-rendering ${CMAKE_DL_LIBS})
set_target_properties(
  posim_underwater posim_underwater_render posim_underwater_gui
  PROPERTIES PREFIX "")
install(TARGETS posim_underwater posim_underwater_render posim_underwater_gui
        LIBRARY DESTINATION lib)
install(DIRECTORY shaders/ DESTINATION share/posim/underwater)
include(CTest)
if(BUILD_TESTING)
  add_executable(underwater-settings-test tests/AppearanceTest.cc)
  target_compile_features(underwater-settings-test PRIVATE cxx_std_17)
  target_include_directories(underwater-settings-test PRIVATE .)
  add_test(NAME underwater-settings COMMAND underwater-settings-test)
  add_executable(underwater-system-lifecycle-test tests/SystemLifecycleTest.cc)
  target_compile_features(underwater-system-lifecycle-test PRIVATE cxx_std_17)
  target_compile_definitions(
    underwater-system-lifecycle-test
    PRIVATE POSIM_LIBRARY_SUFFIX="${CMAKE_SHARED_LIBRARY_SUFFIX}")
  target_link_libraries(
    underwater-system-lifecycle-test
    PRIVATE gz-sim::gz-sim gz-plugin::register gz-rendering::gz-rendering
            ${CMAKE_DL_LIBS})
  add_test(NAME underwater-system-lifecycle
           COMMAND underwater-system-lifecycle-test)
  add_executable(underwater-render-test tests/RenderTest.cc)
  target_compile_features(underwater-render-test PRIVATE cxx_std_17)
  target_include_directories(underwater-render-test PRIVATE .)
  target_link_libraries(
    underwater-render-test PRIVATE posim_underwater_render
                                   gz-rendering::gz-rendering)
  add_executable(underwater-particle-render-test tests/ParticleRenderTest.cc)
  target_compile_features(underwater-particle-render-test PRIVATE cxx_std_17)
  target_include_directories(underwater-particle-render-test PRIVATE .)
  target_link_libraries(
    underwater-particle-render-test PRIVATE posim_underwater_render
                                            gz-rendering::gz-rendering)
endif()
if(ament_cmake_FOUND)
  ament_environment_hooks(rendering.dsv rendering_compat.sh)
  ament_package()
endif()
