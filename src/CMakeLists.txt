add_compile_definitions(SPDLOG_FMT_EXTERNAL)

add_subdirectory(ArduinoLib)
add_subdirectory(Broker)
add_subdirectory(BrokerLib)
add_subdirectory(Common)
add_subdirectory(Embedded)
add_subdirectory(Emulator)
add_subdirectory(Launcher)
add_subdirectory(Terminal)

if (${DCCLITE_GUI_TOOLS})
  add_subdirectory(LitePanel)
  add_subdirectory(LitePanelLib)
  add_subdirectory(LiteWiring)
endif()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	if(${DCCLITE_GUI_SHARP_TOOLS})
        add_subdirectory(SharpDude)
		add_subdirectory(SharpTerminal)
	endif()

	add_subdirectory(UnitTest)
endif()
