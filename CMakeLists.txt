cmake_minimum_required (VERSION 3.14)

#main project
project (DCCLite)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(VERSION_MAJOR "0" )
set(VERSION_MINOR "10" )
set(VERSION_PATCH "0" )
set(DCCLITE_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")

add_compile_definitions(DCCLITE_VERSION=\"${DCCLITE_VERSION}\")
add_compile_definitions(SharedLib PUBLIC DCCLITE_DESKTOP=1)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	set (DCCLITE_MSVC true)
	add_compile_options(/utf-8)
else()
	set (DCCLITE_MSVC false)
endif()

option(DCCLITE_GUI_TOOLS "Build GUI tools (LitePanel and LiteWiring)")
option(DCCLITE_IMGUI_TOOLS "Build IMGUI tools")

if(${DCCLITE_IMGUI_TOOLS})
	option(DCCLITE_IMGUI_TOOLS_FREETYPE "Build IMGUI tools with Freetype fonts support")
endif()

set( CMAKE_EXPORT_COMPILE_COMMANDS on)
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib )

file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

set_property( GLOBAL PROPERTY USE_FOLDERS ON)

add_subdirectory(contrib)
add_subdirectory(src)

set_property(TARGET CityHash PROPERTY FOLDER "contrib") 
set_property(TARGET fmt PROPERTY FOLDER "contrib")
set_property(TARGET spdlog PROPERTY FOLDER "contrib")

if (${DCCLITE_MSVC})
	set_property(TARGET gtest PROPERTY FOLDER "contrib")
	set_property(TARGET gtest_main PROPERTY FOLDER "contrib")
	set_property(TARGET gmock PROPERTY FOLDER "contrib")
	set_property(TARGET gmock_main PROPERTY FOLDER "contrib")	

	set_property(TARGET ArduinoLib PROPERTY FOLDER "Emulator")
	set_property(TARGET Emulator PROPERTY FOLDER "Emulator")

	set_property(TARGET DecodersCommonLib PROPERTY FOLDER "Embedded")	
	set_property(TARGET LiteDecoderLib PROPERTY FOLDER "Embedded")
	set_property(TARGET QueraltLib PROPERTY FOLDER "Embedded")
	set_property(TARGET StandAloneDecoderLib PROPERTY FOLDER "Embedded")
	
	set_property(TARGET Terminal PROPERTY FOLDER "Tools")	
endif()

set_property(TARGET ldmonitor PROPERTY FOLDER "contrib")
set_property(TARGET LuaLib PROPERTY FOLDER "contrib")

set_property(TARGET Launcher PROPERTY FOLDER "Tools")
set_property(TARGET SharedLib PROPERTY FOLDER "Embedded")

set(SPDLOG_FMT_EXTERNAL ON CACHE BOOL "" FORCE)

set(BUILD_GMOCK ON CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

if (${DCCLITE_GUI_TOOLS})
	set_property(TARGET wxrc PROPERTY FOLDER "contrib/wxWidgets/Utilities")
	
	set_property(TARGET wxadv PROPERTY FOLDER "contrib/wxWidgets/Libraries")
	set_property(TARGET wxaui PROPERTY FOLDER "contrib/wxWidgets/Libraries")
	set_property(TARGET wxbase PROPERTY FOLDER "contrib/wxWidgets/Libraries")
	set_property(TARGET wxcore PROPERTY FOLDER "contrib/wxWidgets/Libraries")
	set_property(TARGET wxgl PROPERTY FOLDER "contrib/wxWidgets/Libraries")
	set_property(TARGET wxhtml PROPERTY FOLDER "contrib/wxWidgets/Libraries")
	set_property(TARGET wxmedia PROPERTY FOLDER "contrib/wxWidgets/Libraries")
	set_property(TARGET wxnet PROPERTY FOLDER "contrib/wxWidgets/Libraries")
	set_property(TARGET wxpropgrid PROPERTY FOLDER "contrib/wxWidgets/Libraries")
	set_property(TARGET wxqa PROPERTY FOLDER "contrib/wxWidgets/Libraries")
	set_property(TARGET wxribbon PROPERTY FOLDER "contrib/wxWidgets/Libraries")
	set_property(TARGET wxrichtext PROPERTY FOLDER "contrib/wxWidgets/Libraries")
	set_property(TARGET wxstc PROPERTY FOLDER "contrib/wxWidgets/Libraries")
	set_property(TARGET wxwebview PROPERTY FOLDER "contrib/wxWidgets/Libraries")
	set_property(TARGET wxxml PROPERTY FOLDER "contrib/wxWidgets/Libraries")
	set_property(TARGET wxxrc PROPERTY FOLDER "contrib/wxWidgets/Libraries")
	
	set_property(TARGET wxexpat PROPERTY FOLDER "contrib/wxWidgets/Third Party Libraries")
	set_property(TARGET wxjpeg PROPERTY FOLDER "contrib/wxWidgets/Third Party Libraries")
	set_property(TARGET wxpng PROPERTY FOLDER "contrib/wxWidgets/Third Party Libraries")
	set_property(TARGET wxregex PROPERTY FOLDER "contrib/wxWidgets/Third Party Libraries")
	set_property(TARGET wxscintilla PROPERTY FOLDER "contrib/wxWidgets/Third Party Libraries")
	set_property(TARGET wxtiff PROPERTY FOLDER "contrib/wxWidgets/Third Party Libraries")
	set_property(TARGET wxzlib PROPERTY FOLDER "contrib/wxWidgets/Third Party Libraries")           
	
	set_property(TARGET LitePanelWx PROPERTY FOLDER "CTC")
	set_property(TARGET LitePanelLib PROPERTY FOLDER "CTC")    
	
	#force shared build for wxWidgets
	set(wxBUILD_SHARED ON CACHE BOOL "" FORCE)
	set(wxBUILD_INSTALL OFF CACHE BOOL "" FORCE)
	
endif()

if (${DCCLITE_GUI_SHARP_TOOLS})
	INCLUDE_EXTERNAL_MSPROJECT(SharpDude ${CMAKE_SOURCE_DIR}/src/SharpDude/SharpDude.csproj)

	set_property(TARGET SharpCommon PROPERTY FOLDER "SharpTools")
	set_property(TARGET SharpDude PROPERTY FOLDER "SharpTools")
	set_property(TARGET SharpEEpromViewer PROPERTY FOLDER "SharpTools")
	set_property(TARGET SharpTerminal PROPERTY FOLDER "SharpTools")            
endif()

if(${DCCLITE_IMGUI_TOOLS})
	set_property(TARGET IMGUI PROPERTY FOLDER "contrib")
	set_property(TARGET SDL3-shared PROPERTY FOLDER "contrib/SDL")
	set_property(TARGET uninstall PROPERTY FOLDER "contrib/SDL")
	set_property(TARGET LitePanelLib PROPERTY FOLDER "CTC")  

	set_property(TARGET LitePanelEditorSDL PROPERTY FOLDER "CTC")

	set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
	set(SDL_CAMERA OFF CACHE BOOL "" FORCE)
	set(SDL_SENSOR OFF CACHE BOOL "" FORCE)

	if(${DCCLITE_IMGUI_TOOLS_FREETYPE})
		set_property(TARGET freetype PROPERTY FOLDER "contrib/freetype")
		set_property(TARGET dist PROPERTY FOLDER "contrib/freetype")
	endif()

	INSTALL(FILES contrib/imgui/misc/fonts/DroidSans.ttf DESTINATION bin/ )

endif()

include(InstallRequiredSystemLibraries)

if (${DCCLITE_MSVC})
	if(${DCCLITE_GUI_SHARP_TOOLS})
		install(TARGETS Broker Launcher SharpTerminal SharpEEpromViewer)
	else()
		install(TARGETS Broker Launcher)
	endif()
else()
	install(TARGETS Broker Launcher)
endif()

INSTALL(FILES data/MyRailroad/broker.config.json DESTINATION bin/MyRailroad/ )
INSTALL(FILES data/MyRailroad/TestDevice.decoders.json DESTINATION bin/MyRailroad/ )
INSTALL(FILES data/MyRailroad/TestDeviceConfig.txt DESTINATION bin/MyRailroad/ )