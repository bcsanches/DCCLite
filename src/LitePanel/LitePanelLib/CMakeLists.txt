file(GLOB_RECURSE SRC_RENDER
    "render/*.cpp"
    "render/*.h"
)

 
set(LitePanelLib_SRCS	
    	EditCmds.cpp
	MapObject.cpp
    	MapObjectFactory.cpp
	Panel.cpp
	RailObject.cpp
	TileMap.cpp	
	render/Color.cpp
	render/ColorStyle.cpp
	render/IRenderer.cpp
	render/TileMapView.cpp
)

set(LitePanelLib_HDRS
	EditCmds.h
	LitePanelLibDefs.h
	MapObject.h
    	MapObjectFactory.h
	Panel.h
	Point.h
	RailObject.h
	TileMap.h
	render/Color.h
	render/ColorStyle.h
	render/IRenderer.h
	render/TileMapView.h
)

source_group("render"          FILES ${SRC_RENDER})

if(WIN32)

	add_compile_definitions(_CRT_SECURE_NO_WARNINGS)

endif(WIN32)

add_library(LitePanelLib  ${LitePanelLib_SRCS} ${LitePanelLib_HDRS})
target_compile_features(LitePanelLib PUBLIC cxx_std_17)     

target_include_directories(LitePanelLib
  PUBLIC  ${DCCLite_SOURCE_DIR}/src/LitePanelLib/
  PUBLIC  ${DCCLite_SOURCE_DIR}/contrib/fmt/include
  PUBLIC  ${DCCLite_SOURCE_DIR}/contrib/JsonCreator/include
          ${DCCLite_SOURCE_DIR}/contrib/rapidjson/include
          ${DCCLite_SOURCE_DIR}/contrib/spdlog/include          
)
