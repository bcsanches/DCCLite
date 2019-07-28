add_compile_definitions(SPDLOG_FMT_EXTERNAL)

add_subdirectory(ArduinoLib)
add_subdirectory(Common)
add_subdirectory(Terminal)
add_subdirectory(Broker)
add_subdirectory(Emulator)
add_subdirectory(Launcher)
add_subdirectory(LiteDecoder)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	add_subdirectory(SharpTerminal)
	add_subdirectory(UnitTest)
endif()
