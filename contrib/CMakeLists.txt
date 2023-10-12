add_subdirectory(cityhash)
add_subdirectory(fmt)
add_subdirectory(ldmonitor)
add_subdirectory(lua-5.4.6)
#add_subdirectory(googletest)
add_subdirectory(sigslot)

if(${DCCLITE_IMGUI_TOOLS})
	add_subdirectory(SDL)
endif()

if(${DCCLITE_GUI_TOOLS})
  add_subdirectory(wxWidgets)
endif()
