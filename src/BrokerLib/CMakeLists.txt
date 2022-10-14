
add_library(
    BrokerLib          	   	 
        BonjourService.cpp
        BonjourService.h        
        Broker.cpp        
        Broker.h    
        DccAddress.cpp
        DccAddress.h
        DccLiteService.cpp
        DccLiteService.h  
        DccppService.cpp
        DccppService.h
        Decoder.cpp
        Decoder.h
        Device.cpp
        Device.h      
        IDccLiteService.h  
        IDevice.h
        FileWatcher.cpp
        FileWatcher.h
        LocationManager.cpp
        LocationManager.h
        LoconetService.cpp
        LoconetService.h  
        Messenger.cpp
        Messenger.h
        NetworkDevice.cpp
        NetworkDevice.h
        NetworkDeviceTasks.cpp
        NetworkDeviceTasks.h
        OutputDecoder.cpp
        OutputDecoder.h
        PinManager.cpp
        PinManager.h
        Project.cpp
        Project.h
        RemoteDecoder.cpp
        RemoteDecoder.h
        SensorDecoder.cpp
        SensorDecoder.h  
        Service.cpp
        Service.h
        SignalDecoder.cpp
        SignalDecoder.h
        SimpleOutputDecoder.cpp
        SimpleOutputDecoder.h
        SpecialFolders.cpp
        SpecialFolders.h
        TerminalCmd.cpp
        TerminalCmd.h
        TerminalService.cpp
        TerminalService.h
        Thinker.cpp
        Thinker.h
        ThrottleService.cpp
        ThrottleService.h
        TurnoutDecoder.cpp
        TurnoutDecoder.h 
        TurntableAutoInverterDecoder.cpp
        TurntableAutoInverterDecoder.h       
        VirtualDevice.cpp
        VirtualDevice.h
        ZeroconfService.cpp
        ZeroconfService.h
)

if(WIN32)
    target_link_libraries(BrokerLib         
		PRIVATE Common 
		PRIVATE SharedLib 		 
		PRIVATE fmt
        PRIVATE lfwatch
        PRIVATE sigslot
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
        dl
        sigslot
	)

endif(WIN32)

target_compile_features(BrokerLib PUBLIC cxx_std_17)

target_include_directories(BrokerLib 	
    PUBLIC ${DCCLite_SOURCE_DIR}/contrib/JsonCreator/include
    PUBLIC ${DCCLite_SOURCE_DIR}/contrib/lfwatch/include    
    PUBLIC ${DCCLite_SOURCE_DIR}/contrib/rapidjson/include
    PUBLIC ${DCCLite_SOURCE_DIR}/contrib/sigslot/include
    PUBLIC ${DCCLite_SOURCE_DIR}/src/Common
    PUBLIC ${DCCLite_SOURCE_DIR}/src/Embedded/lib/Shared
    PUBLIC ${DCCLite_SOURCE_DIR}/contrib/fmt/include
    PUBLIC ${DCCLite_SOURCE_DIR}/contrib/spdlog/include
    PRIVATE ${DCCLite_SOURCE_DIR}/contrib/magic_enum/include
)



