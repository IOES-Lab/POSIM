# Copyright 2026 POSIM contributors
# SPDX-License-Identifier: Apache-2.0

# Register the repository C++ style, analysis and offline manifest checks.
function(posim_add_lint_tests)
  # Use the same formatter and style as the repository hooks and CI.
  find_package(ament_cmake_clang_format REQUIRED)
  ament_clang_format(CONFIG_FILE
    "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/../.clang-format")
  find_package(ament_cmake_copyright REQUIRED)
  ament_copyright()
  find_package(ament_cmake_cppcheck REQUIRED)
  ament_cppcheck()
  find_package(ament_cmake_lint_cmake REQUIRED)
  ament_lint_cmake()

  # Keep full schema validation without downloading a schema during each test.
  find_package(ament_cmake_xmllint REQUIRED)
  set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/package.xml")
  file(READ "${CMAKE_CURRENT_SOURCE_DIR}/package.xml" manifest)
  string(REPLACE
    "http://download.ros.org/schema/package_format3.xsd"
    "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/ros_package_schema/package_format3.xsd"
    manifest "${manifest}")
  file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/posim_lint")
  file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/posim_lint/package.xml" "${manifest}")
  ament_xmllint(TESTNAME xmllint
    "${CMAKE_CURRENT_BINARY_DIR}/posim_lint/package.xml")
endfunction()
