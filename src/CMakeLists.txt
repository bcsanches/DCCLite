add_compile_definitions(SPDLOG_FMT_EXTERNAL)

set(MY_DEFINITION 
	$<$<CONFIG:Debug>:DCCLITE_DEBUG>
	$<$<CONFIG:RelWithDebInfo>:DCCLITE_DEBUG>
	$<$<CONFIG:Release>:DCCLITE_RELEASE> 
	$<$<CONFIG:MinSizeRel>:DCCLITE_RELEASE>
)

add_compile_definitions(${MY_DEFINITION})

add_subdirectory(ArduinoLib)
add_subdirectory(Broker)
add_subdirectory(BrokerLib)
add_subdirectory(Common)
add_subdirectory(Embedded)
add_subdirectory(Emulator)
add_subdirectory(Launcher)
add_subdirectory(Terminal)

add_subdirectory(LitePanel)

if (${DCCLITE_GUI_TOOLS})
  add_subdirectory(LiteWiring)
endif()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	if(${DCCLITE_GUI_SHARP_TOOLS})
		add_subdirectory(SharpTools/SharpCommon)                
		add_subdirectory(SharpTools/SharpEEpromViewer)
		add_subdirectory(SharpTools/SharpTerminal)
		add_subdirectory(SharpTools/SharpDude)
	endif()
	
endif()

add_subdirectory(UnitTest)
