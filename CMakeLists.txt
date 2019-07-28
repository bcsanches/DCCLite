cmake_minimum_required (VERSION 3.14)
project (DCCLite)

set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib )

set_property( GLOBAL PROPERTY USE_FOLDERS ON)

add_subdirectory(src)
add_subdirectory(contrib)
 
set_property(TARGET fmt PROPERTY FOLDER "contrib")

include(InstallRequiredSystemLibraries)

install(TARGETS Broker Launcher)
