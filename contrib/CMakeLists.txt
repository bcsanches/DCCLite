add_subdirectory(fmt)
add_subdirectory(lfwatch)
add_subdirectory(googletest)
add_subdirectory(sigslot)

if(${DCCLITE_GUI_TOOLS})
  add_subdirectory(wxWidgets)
endif()
