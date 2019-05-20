cmake_minimum_required (VERSION 3.14)
project (DCCLite)

set_property( GLOBAL PROPERTY USE_FOLDERS ON)

add_subdirectory(src)
add_subdirectory(contrib)
 
set_property(TARGET fmt PROPERTY FOLDER "contrib")
