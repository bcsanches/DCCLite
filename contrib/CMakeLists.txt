add_subdirectory(cityhash)
add_subdirectory(fmt)
add_subdirectory(ldmonitor)
add_subdirectory(lua-5.4.6)
add_subdirectory(sigslot)

if(${DCCLITE_IMGUI_TOOLS})
	add_subdirectory(SDL)
endif()

if(${DCCLITE_GUI_TOOLS})
	add_subdirectory(wxWidgets)
endif()

if(${DCCLITE_IMGUI_TOOLS})

	add_compile_definitions(IMGUI_DEFINE_MATH_OPERATORS)

	if(${DCCLITE_IMGUI_TOOLS_FREETYPE})
		add_compile_definitions(IMGUI_ENABLE_FREETYPE)
	
		set(IMGUI_FREETYPE_SRCS
			imgui/misc/freetype/imgui_freetype.cpp
			imgui/misc/freetype/imgui_freetype.h
      	)
  else()
		set(IMGUI_FREETYPE_SRCS)
  endif()

	add_library(IMGUI 
		imgui/imconfig.h
		imgui/imgui.cpp
		imgui/imgui.h
		imgui/imgui_demo.cpp
		imgui/imgui_draw.cpp
		imgui/imgui_internal.h
		imgui/imgui_tables.cpp
		imgui/imgui_widgets.cpp
		imgui/imstb_rectpack.h
		imgui/imstb_textedit.h
		imgui/imstb_truetype.h

		imgui/backends/imgui_impl_sdl3.h
		imgui/backends/imgui_impl_sdl3.cpp
		imgui/backends/imgui_impl_sdlrenderer3.cpp
		imgui/backends/imgui_impl_sdlrenderer3.h   

		${IMGUI_FREETYPE_SRCS} 
	)

	target_include_directories(IMGUI 	
		PRIVATE ${DCCLite_SOURCE_DIR}/contrib/imgui/	
		PRIVATE ${DCCLite_SOURCE_DIR}/contrib/SDL/include
	)

	target_link_libraries(IMGUI)

	target_compile_features(IMGUI PUBLIC cxx_std_17)

	if(${DCCLITE_IMGUI_TOOLS_FREETYPE})
		add_subdirectory(freetype-2.7)

		target_include_directories(IMGUI 	
			PRIVATE ${DCCLite_SOURCE_DIR}/contrib/freetype-2.7/include
		)

	endif()

endif()