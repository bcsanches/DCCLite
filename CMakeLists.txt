cmake_minimum_required (VERSION 3.14)

# Download and unpack googletest at configure time
configure_file(CMakeLists.txt.in googletest-download/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
if(result)
  message(FATAL_ERROR "CMake step for googletest failed: ${result}")
endif()
execute_process(COMMAND ${CMAKE_COMMAND} --build .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
if(result)
  message(FATAL_ERROR "Build step for googletest failed: ${result}")
endif()

# Prevent overriding the parent project's compiler/linker
# settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Add googletest directly to our build. This defines
# the gtest and gtest_main targets.
add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/googletest-src
                 ${CMAKE_CURRENT_BINARY_DIR}/googletest-build
                 EXCLUDE_FROM_ALL)

# The gtest/gtest_main targets carry header search path
# dependencies automatically when using CMake 2.8.11 or
# later. Otherwise we have to add them here ourselves.
if (CMAKE_VERSION VERSION_LESS 2.8.11)
  include_directories("${gtest_SOURCE_DIR}/include")
endif()

#main project
project (DCCLite)

set(VERSION_MAJOR "0" )
set(VERSION_MINOR "3" )
set(VERSION_PATCH "1" )
set(DCCLITE_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")

add_compile_definitions(DCCLITE_VERSION=\"${DCCLITE_VERSION}\")

option(DCCLITE_GUI_TOOLS "Build GUI tools (LitePanel and LiteWiring)")
option(DCCLITE_GUI_SHARP_TOOLS "Build C# GUI tools - SharpTerminal")


set( CMAKE_EXPORT_COMPILE_COMMANDS on)
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib )

set_property( GLOBAL PROPERTY USE_FOLDERS ON)

add_subdirectory(src)
add_subdirectory(contrib)
 
set_property(TARGET fmt PROPERTY FOLDER "contrib")
set_property(TARGET gtest PROPERTY FOLDER "contrib")
set_property(TARGET gtest_main PROPERTY FOLDER "contrib")
set_property(TARGET lfwatch PROPERTY FOLDER "contrib")

include(InstallRequiredSystemLibraries)

install(TARGETS Broker Launcher)


