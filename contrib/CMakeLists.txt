add_subdirectory(fmt)
add_subdirectory(lfwatch)

if(${GUI_TOOLS})
  add_subdirectory(wxWidgets)
endif()
