
set(Broker_SRCS
	Broker.cpp
  DccLiteService.cpp
  Decoder.cpp
  Device.cpp
  main.cpp
  OutputDecoder.cpp
  Project.cpp
  SensorDecoder.cpp  
  Service.cpp
  SpecialFolders.cpp
  TerminalCmd.cpp
  TerminalService.cpp
)

set(Broker_HDRS
	Broker.h
  DccLiteService.h
  Decoder.h
  Device.h
  OutputDecoder.h
  Project.h
  SensorDecoder.h
  Service.h
  SpecialFolders.h
  TerminalCmd.h
  TerminalService.h
)

add_executable(Broker  ${Broker_SRCS} ${Broker_HDRS})
target_compile_features(Broker PUBLIC cxx_std_17)    

target_include_directories(Broker PRIVATE ${DCCLite_SOURCE_DIR}/src/Common)

target_link_libraries(Broker PRIVATE Common EmbeddedLib)

#set_target_properties( Broker PROPERTIES RUNTIME_OUTPUT_DIRECTORY bin/ )

if(WIN32)
    target_link_libraries(Broker PRIVATE bcrypt.lib PRIVATE fmt)
    
    set_property(TARGET Broker PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
    set_property(TARGET Broker PROPERTY VS_DEBUGGER_COMMAND_ARGUMENTS "..\\data\\EFMR")
      
endif(WIN32)
