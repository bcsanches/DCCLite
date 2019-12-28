add_subdirectory(fmt)
add_subdirectory(lfwatch)

if(${DCCLITE_GUI_TOOLS})
  add_subdirectory(wxWidgets)
endif()
