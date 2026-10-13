"""
Apply build-only Jetty adaptation to a separately fetched GPL dependency.

No upstream wave implementation is vendored into POSIM or this repository.
Keep the modified upstream source alongside any distributed plugin binaries.
"""

from pathlib import Path
import sys

root = Path(sys.argv[1])
path = root / "gz-waves/CMakeLists.txt"
text = path.read_text()
text = text.replace(
    'elseif("$ENV{GZ_VERSION}" STREQUAL "ionic")\n  find_package(gz-cmake4',
    'elseif("$ENV{GZ_VERSION}" STREQUAL "jetty")\n  find_package(gz-cmake REQUIRED)\n  set(GZ_CMAKE_VER ${gz-cmake_VERSION_MAJOR})\nelseif("$ENV{GZ_VERSION}" STREQUAL "ionic")\n  find_package(gz-cmake4',
    1,
)
marker = 'else()\n  message(FATAL_ERROR "Unsupported GZ_VERSION: $ENV{GZ_VERSION}")'
jetty = 'elseif("$ENV{GZ_VERSION}" STREQUAL "jetty")\n'
for name, var, components in [
    ("math", "MATH", "eigen3"),
    ("plugin", "PLUGIN", "loader register"),
    ("common", "COMMON", "graphics events"),
    ("msgs", "MSGS", ""),
    ("transport", "TRANSPORT", ""),
    ("rendering", "RENDERING", "ogre2"),
    ("sim", "SIM", ""),
]:
    jetty += (
        f"  gz_find_package(gz-{name} REQUIRED"
        + (f" COMPONENTS {components}" if components else "")
        + ")\n"
        + f'  set(GZ_{var}_VER "")\n'
    )
jetty += '  gz_find_package(sdformat REQUIRED)\n  set(SDF_VER "")\n  message(STATUS "Compiling external Wave Sim against Gazebo Jetty")\n'
assert marker in text
text = text.replace(marker, jetty + marker).replace(
    "include(Add_gnuplot-iostream)", "if(BUILD_TESTING)\n  include(Add_gnuplot-iostream)\nendif()"
)
path.write_text(text)
start = text.index("gz_find_package(GzOGRE2 VERSION 2.3")
end = text.index("#####################################", start)
text = (
    text[:start]
    + """set(GZ_OGRE2_PROJECT_NAME "OGRE-Next")
gz_find_package(GzOGRE2 VERSION 2.3 COMPONENTS HlmsPbs HlmsUnlit Overlay REQUIRED)
set(HAVE_OGRE2 TRUE)
"""
    + text[end:]
)
path.write_text(text)
for path in (root / "gz-waves").rglob("CMakeLists.txt"):
    path.write_text(
        path.read_text().replace("gz-rendering${GZ_RENDERING_VER}-ogre2", "gz-rendering::ogre2")
    )
# The desktop Qt wave panel is unnecessary for the server; use transport control.
path = root / "gz-waves/src/CMakeLists.txt"
path.write_text(
    path.read_text().replace(
        "add_subdirectory(gui)",
        'if(NOT "$ENV{GZ_VERSION}" STREQUAL "jetty")\n  add_subdirectory(gui)\nendif()',
        1,
    )
)
# gz-cmake's Jetty components are independent targets. Define the core first,
# then plugins; older upstream ordering omitted the core dependency entirely.
path = root / "gz-waves/src/CMakeLists.txt"
text = path.read_text().replace("add_subdirectory(systems)", "")
text += "\nset_target_properties(${PROJECT_LIBRARY_TARGET_NAME} PROPERTIES CXX_VISIBILITY_PRESET default VISIBILITY_INLINES_HIDDEN OFF)\nadd_subdirectory(systems)\n"
path.write_text(text)
path = root / "gz-waves/src/systems/waves/CMakeLists.txt"
path.write_text(
    path.read_text()
    + "\nset_target_properties(${rendering_target} ${rendering_ogre2_target} PROPERTIES CXX_VISIBILITY_PRESET default VISIBILITY_INLINES_HIDDEN OFF)\n"
)
