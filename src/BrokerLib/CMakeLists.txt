
set(BrokerLib_SRCS
	Broker.cpp    
    DccAddress.cpp
    DccLiteService.cpp  
    DccppService.cpp
    Decoder.cpp
    Device.cpp
    FileWatcher.cpp
    LocationManager.cpp
    LoconetService.cpp  
    NetworkDevice.cpp
    OutputDecoder.cpp
    PinManager.cpp
    Project.cpp
    RemoteDecoder.cpp
    SensorDecoder.cpp  
    Service.cpp
    SignalDecoder.cpp
    SimpleOutputDecoder.cpp
    SpecialFolders.cpp
    TerminalCmd.cpp
    TerminalService.cpp
    TurnoutDecoder.cpp
    VirtualDevice.cpp
)

set(BrokerLib_HDRS
	Broker.h
    DccAddress.h
    DccLiteService.h  
    DccppService.h
    Decoder.h
    Device.h
    FileWatcher.h
    IDccLiteService.h
    IDevice.h
    LocationManager.h
    LoconetService.h
    NetworkDevice.h
    OutputDecoder.h
    PinManager.h
    Project.h
    RemoteDecoder.h
    SensorDecoder.h
    Service.h
    SignalDecoder.h
    SimpleOutputDecoder.h
    SpecialFolders.h
    TerminalCmd.h
    TerminalService.h
    TurnoutDecoder.h
    VirtualDevice.h
)

add_library(BrokerLib  ${BrokerLib_SRCS} ${BrokerLib_HDRS})
target_compile_features(BrokerLib PUBLIC cxx_std_17)    

target_include_directories(BrokerLib 	
    PUBLIC ${DCCLite_SOURCE_DIR}/contrib/JsonCreator/include
    PUBLIC ${DCCLite_SOURCE_DIR}/contrib/lfwatch/include    
    PUBLIC ${DCCLite_SOURCE_DIR}/contrib/rapidjson/include
    PUBLIC ${DCCLite_SOURCE_DIR}/src/Common
    PUBLIC ${DCCLite_SOURCE_DIR}/src/Embedded/lib/Shared
    PUBLIC ${DCCLite_SOURCE_DIR}/contrib/fmt/include
    PUBLIC ${DCCLite_SOURCE_DIR}/contrib/spdlog/include
)

if(WIN32)
    target_link_libraries(BrokerLib         
		PRIVATE Common 
		PRIVATE SharedLib 		 
		PRIVATE fmt
        PRIVATE lfwatch
	)

	add_compile_definitions(_CRT_SECURE_NO_WARNINGS)       
      
else(WIN32)

	find_package(OpenSSL REQUIRED)

	target_link_libraries(Broker 
		OpenSSL::Crypto
		OpenSSL::SSL
		Common 
		SharedLib
		stdc++fs 
		fmt 
		uuid 		
        lfwatch
	)

endif(WIN32)

