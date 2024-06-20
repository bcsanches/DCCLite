
set(Emulator_SRCS
	main.cpp
)

add_executable(Emulator  ${Emulator_SRCS})

target_include_directories(Emulator 
  PRIVATE 
    ${DCCLite_SOURCE_DIR}/src/Common
    ${DCCLite_SOURCE_DIR}/src/ArduinoLib
)

target_link_libraries(Emulator PRIVATE ArduinoLib Common SharedLib)

#set_target_properties( Emulator PROPERTIES RUNTIME_OUTPUT_DIRECTORY bin/ )

if(WIN32)      
    set_property(TARGET Emulator PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")        
endif(WIN32)
