
set(Broker_SRCS
	Broker.cpp
  DccLiteService.cpp  
  DccppService.cpp
  Decoder.cpp
  Device.cpp
  FileWatcher.cpp
  LocationManager.cpp
  main.cpp  
  OutputDecoder.cpp
  PinManager.cpp
  Project.cpp
  SensorDecoder.cpp  
  Service.cpp
  SimpleOutputDecoder.cpp
  SpecialFolders.cpp
  TerminalCmd.cpp
  TerminalService.cpp
  TurnoutDecoder.cpp
)

set(Broker_HDRS
	Broker.h
  DccLiteService.h  
  DccppService.h
  Decoder.h
  Device.h
  FileWatcher.h
  LocationManager.h
  OutputDecoder.h
  PinManager.h
  Project.h
  SensorDecoder.h
  Service.h
  SimpleOutputDecoder.h
  SpecialFolders.h
  TerminalCmd.h
  TerminalService.h
  TurnoutDecoder.h
)

add_executable(Broker  ${Broker_SRCS} ${Broker_HDRS})
target_compile_features(Broker PUBLIC cxx_std_17)    

target_include_directories(Broker 
	PRIVATE ${DCCLite_SOURCE_DIR}/src/Common
  PRIVATE ${DCCLite_SOURCE_DIR}/contrib/lfwatch/include	
)

#set_target_properties( Broker PROPERTIES RUNTIME_OUTPUT_DIRECTORY bin/ )

if(WIN32)
    target_link_libraries(Broker
    PRIVATE bcrypt.lib 
		PRIVATE Common 
		PRIVATE EmbeddedLib 		 
		PRIVATE fmt
    PRIVATE lfwatch
	)

	add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
    
    set_property(TARGET Broker PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
    set_property(TARGET Broker PROPERTY VS_DEBUGGER_COMMAND_ARGUMENTS "..\\data\\EFMR")
      
else(WIN32)

	find_package(OpenSSL REQUIRED)

	target_link_libraries(Broker 
		OpenSSL::Crypto
		OpenSSL::SSL
		Common 
		EmbeddedLib
		stdc++fs 
		fmt 
		uuid 		
    lfwatch
	)

endif(WIN32)

