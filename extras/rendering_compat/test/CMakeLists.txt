cmake_minimum_required(VERSION 3.22)
project(posim_dynamic_material_regression LANGUAGES CXX)
find_package(gz-rendering 10.0.2 EXACT REQUIRED COMPONENTS ogre2)
find_package(PkgConfig REQUIRED)
pkg_check_modules(OGRE REQUIRED IMPORTED_TARGET OGRE-Next)
pkg_check_modules(HLMS REQUIRED IMPORTED_TARGET OGRE-Next-Hlms)
find_path(
  HLMS_COMMON OgreHlmsTextureBaseClass.h
  HINTS ${HLMS_INCLUDE_DIRS}
  PATH_SUFFIXES Common REQUIRED)
foreach(name dynamic waves)
  add_executable(${name}-material-test dynamic_material.cc)
  target_include_directories(${name}-material-test PRIVATE ${HLMS_COMMON})
  target_compile_features(${name}-material-test PRIVATE cxx_std_17)
  target_link_libraries(
    ${name}-material-test
    PRIVATE gz-rendering::gz-rendering gz-rendering::ogre2 PkgConfig::OGRE
            PkgConfig::HLMS)
endforeach()
find_library(
  WAVES_OGRE2 gz-waves1-rendering-ogre2
  HINTS "${WAVE_PREFIX}/lib"
  NO_DEFAULT_PATH REQUIRED)
target_compile_definitions(waves-material-test PRIVATE POSIM_TEST_WAVES)
target_include_directories(waves-material-test
                           PRIVATE "${WAVE_SOURCE}/gz-waves/src/systems/waves")
target_link_libraries(waves-material-test PRIVATE ${WAVES_OGRE2})
# Resolve Wave Sim's indirect dependencies during a fresh native installation,
# before its library directory has been added to the runtime environment.
target_link_options(waves-material-test PRIVATE
                    "LINKER:-rpath-link,${WAVE_PREFIX}/lib")
