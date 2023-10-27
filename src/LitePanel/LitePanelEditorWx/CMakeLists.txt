
set(LitePanelWx_SRCS	
  GLCanvas.cpp
  LitePanel.cpp      
  PanelDocument.cpp
  PanelDocumentView.cpp
  PanelEditorCanvas.cpp  
  TempObjects.cpp
  TileMapCanvas.cpp
  LitePanel.rc
)

set(LitePanelWx_HDRS
  GLCanvas.h      
  LitePanel.h 
  PanelDocument.h 
  PanelDocumentView.h
  PanelEditorCanvas.h
  TempObjects.h
  TileMapCanvas.h
)

add_executable(LitePanelWx WIN32 ${LitePanelWx_SRCS} ${LitePanelWx_HDRS})
target_compile_features(LitePanelWx PUBLIC cxx_std_17)    

target_include_directories(LitePanelWx 
	PRIVATE ${DCCLite_SOURCE_DIR}/src/Common  
  PRIVATE ${DCCLite_SOURCE_DIR}/src/LitePanel/LitePanelLib
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
    
    set_property(TARGET LitePanelWx PROPERTY VS_DEBUGGER_ENVIRONMENT "PATH=${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/vc_dll;${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/vc_x64_dll")
    set_property(TARGET LitePanelWx PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
    set_property(TARGET LitePanelWx PROPERTY VS_DEBUGGER_COMMAND_ARGUMENTS "..\\data\\EFMR")
      
else(WIN32)

	find_package(OpenSSL REQUIRED)

	target_link_libraries(LitePanelWx 
		RIVATE Common 		
		PRIVATE bcrypt.lib 
		PRIVATE fmt
    PRIVATE LitePanelLib
    wx::net
    wx::core
    wx::base
    wx::gl		
	)

endif(WIN32)

