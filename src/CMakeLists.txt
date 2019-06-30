add_compile_definitions(SPDLOG_FMT_EXTERNAL)

add_subdirectory(ArduinoLib)
add_subdirectory(Common)
add_subdirectory(Terminal)
add_subdirectory(Broker)
add_subdirectory(Emulator)
add_subdirectory(LiteDecoder)

if(WIN32)
	add_subdirectory(SharpTerminal)
	add_subdirectory(UnitTest)
endif(WIN32)
