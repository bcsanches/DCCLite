add_compile_definitions(IMGUI_DEFINE_MATH_OPERATORS)

add_executable(LitePanelEditorSDL 
	ConsoleWidget.cpp
	ConsoleWidget.h
	EditorWidget.cpp
	EditorWidget.h
	main.cpp
	PanelEditorApp.cpp
	PanelEditorApp.h
	StatusBarWidget.cpp
	StatusBarWidget.h
	ToolBarWidget.cpp
	ToolBarWidget.h
)

target_compile_features(LitePanelEditorSDL PUBLIC cxx_std_17)    

target_include_directories(LitePanelEditorSDL
	PRIVATE ${DCCLite_SOURCE_DIR}/contrib/fmt/include
	PRIVATE ${DCCLite_SOURCE_DIR}/contrib/imgui/
	PRIVATE ${DCCLite_SOURCE_DIR}/contrib/SDL/include
	PRIVATE ${DCCLite_SOURCE_DIR}/contrib/spdlog/include
	PRIVATE ${DCCLite_SOURCE_DIR}/src/Common
)

if(WIN32)
    target_link_libraries(LitePanelEditorSDL
		PRIVATE Common	
		PRIVATE SDL3-shared
		PRIVATE IMGUI
    )

    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
        
    set_property(TARGET LitePanelEditorSDL PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
    set_property(TARGET LitePanelEditorSDL PROPERTY VS_DEBUGGER_COMMAND_ARGUMENTS "..\\data\\EFMR")
      
else(WIN32)

	find_package(OpenSSL REQUIRED)

	target_link_libraries(LitePanelEditorSDL
		PRIVATE Common	
		PRIVATE SDL3-shared
		PRIVATE IMGUI
	)

endif(WIN32)

