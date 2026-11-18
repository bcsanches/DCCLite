
add_compile_definitions(IMGUI_DEFINE_MATH_OPERATORS)

set(LitePanelWx_SRCS	
	GL_Canvas.cpp
	GL_Renderer.cpp
	LitePanelApp.cpp      
	PanelDocument.cpp
	PanelDocumentView.cpp
	PanelEditorCanvas.cpp  
	TempObjects.cpp
	TileMapCanvas.cpp
	LitePanel.rc
)

set(LitePanelWx_HDRS
	GL_Canvas.h
	GL_Renderer.h
	LitePanelApp.h
	PanelDocument.h
	PanelDocumentView.h
	PanelEditorCanvas.h
	TempObjects.h
	TileMapCanvas.h
)

file(GLOB_RECURSE LITE_PANEL_EDITOR_SOURCES
    "*.cpp"
    "*.h"
	"*.rc"
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}
             FILES ${LITE_PANEL_EDITOR_SOURCES}
)

add_executable(LitePanelWx WIN32 ${LitePanelWx_SRCS} ${LitePanelWx_HDRS})

target_include_directories(LitePanelWx 
	PRIVATE ${DCCLite_SOURCE_DIR}/src/Common  
	PRIVATE ${DCCLite_SOURCE_DIR}/src/LitePanel/
)

#set_target_properties( Broker PROPERTIES RUNTIME_OUTPUT_DIRECTORY bin/ )

if(WIN32)
	target_link_libraries(LitePanelWx 
		PRIVATE Common 		
		PRIVATE bcrypt.lib 
		PRIVATE fmt      
		PRIVATE LitePanelLib		
		wx::net
		wx::core
		wx::base
		wx::gl
	)

    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
    add_compile_definitions(WXUSINGDLL)
    
    set_property(TARGET LitePanelWx PROPERTY VS_DEBUGGER_ENVIRONMENT "PATH=${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/../contrib/wxWidgets/lib/vc_dll;${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/../contrib/wxWidgets/lib/vc_x64_dll")
    set_property(TARGET LitePanelWx PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
    set_property(TARGET LitePanelWx PROPERTY VS_DEBUGGER_COMMAND_ARGUMENTS "..\\data\\EFMR")
      
else(WIN32)

	find_package(OpenSSL REQUIRED)

	target_link_libraries(LitePanelWx 
		PRIVATE Common 		
		PRIVATE bcrypt.lib 
		PRIVATE fmt
    	PRIVATE LitePanelLib
    	wx::net
    	wx::core
    	wx::base
    	wx::gl		
	)

endif(WIN32)

