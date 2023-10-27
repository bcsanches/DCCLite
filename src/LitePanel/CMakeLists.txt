if (${DCCLITE_GUI_TOOLS} OR ${DCCLITE_IMGUI_TOOLS})
	add_subdirectory(LitePanelLib)
endif()

if (${DCCLITE_GUI_TOOLS})
  add_subdirectory(LitePanelEditorWx)
endif()

if(${DCCLITE_IMGUI_TOOLS})
	add_subdirectory(LitePanelEditorSDL)
endif()
