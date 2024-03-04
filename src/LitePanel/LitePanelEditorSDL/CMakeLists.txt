add_compile_definitions(IMGUI_DEFINE_MATH_OPERATORS)

add_executable(LitePanelEditorSDL 
	ConsoleWidget.cpp
	ConsoleWidget.h
	Document.cpp
	Document.h
	DocumentWidget.cpp
	DocumentWidget.h
	EditorWidget.cpp
	EditorWidget.h
	ImGuiTileMapRenderer.cpp
	ImGuiTileMapRenderer.h
	KeyBindingManager.cpp
	KeyBindingManager.h
	main.cpp
	MessageBox.cpp
	MessageBox.h
	PanelEditorApp.cpp
	PanelEditorApp.h
	Settings.cpp
	Settings.h
	StatusBarWidget.cpp
	StatusBarWidget.h
	SystemTools.h
	ToolBarWidget.cpp
	ToolBarWidget.h
)

target_compile_features(LitePanelEditorSDL PUBLIC cxx_std_17)    

target_include_directories(LitePanelEditorSDL
	PRIVATE ${DCCLite_SOURCE_DIR}/contrib/fmt/include
	PRIVATE ${DCCLite_SOURCE_DIR}/contrib/imgui/
	PRIVATE ${DCCLite_SOURCE_DIR}/contrib/magic_enum/include
	PRIVATE ${DCCLite_SOURCE_DIR}/contrib/SDL/include
	PRIVATE ${DCCLite_SOURCE_DIR}/contrib/spdlog/include
	PRIVATE ${DCCLite_SOURCE_DIR}/src/Common
        PRIVATE ${DCCLite_SOURCE_DIR}/src/LitePanel/
)

if(${DCCLITE_IMGUI_TOOLS_FREETYPE})
	target_link_libraries(LitePanelEditorSDL
		PRIVATE freetype
	)
endif()

if(WIN32)

	target_sources(LitePanelEditorSDL PRIVATE SystemTools_win.cpp SystemTools_win.h)

    target_link_libraries(LitePanelEditorSDL
		PRIVATE Common	
		PRIVATE LitePanelLib
		PRIVATE SDL3-shared
		PRIVATE IMGUI
    )

    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
        
    set_property(TARGET LitePanelEditorSDL PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
    set_property(TARGET LitePanelEditorSDL PROPERTY VS_DEBUGGER_COMMAND_ARGUMENTS "..\\data\\EFMR")

    add_custom_command(
        TARGET LitePanelEditorSDL POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_SOURCE_DIR}/contrib/imgui/misc/fonts/DroidSans.ttf
                ${CMAKE_SOURCE_DIR}/bin/DroidSans.ttf)
      
else(WIN32)

	find_package(OpenSSL REQUIRED)

	target_link_libraries(LitePanelEditorSDL
		PRIVATE Common	
		PRIVATE LitePanelLib
		PRIVATE SDL3-shared
		PRIVATE IMGUI
		PRIVATE freetype
	)

endif(WIN32)

