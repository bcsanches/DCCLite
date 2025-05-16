file(GLOB_RECURSE SRC_DCC
    "dcc/*.cpp"
    "dcc/*.h"
)

file(GLOB_RECURSE SRC_LN
    "ln/*.cpp"
    "ln/*.h"
)

file(GLOB_RECURSE SRC_SHELL
    "shell/*.cpp"
    "shell/*.h"
)

file(GLOB_RECURSE SRC_SYS
    "sys/*.cpp"
    "sys/*.h"
)

file(GLOB_RECURSE SRC_TERMINAL
    "terminal/*.cpp"
    "terminal/*.h"
)

file(GLOB_RECURSE SRC_DISPATCHER
    "dispatcher/*.cpp"
    "dispatcher/*.h"
)

add_library(
    BrokerLib	     	   	     
        dcc/DccAddress.cpp
        dcc/DccAddress.h
        dcc/DccLiteService.cpp
        dcc/DccLiteService_script.cpp
        dcc/DccLiteService.h  
        dcc/DccppService.cpp
        dcc/DccppService.h
        dcc/Decoder.cpp
        dcc/Decoder.h
        dcc/Device.cpp
        dcc/Device.h      
        dcc/IDccLiteService.h  
        dcc/IDevice.h
        dcc/IResettableObject.h
        dcc/LocationManager.cpp
        dcc/LocationManager.h
        dcc/NetworkDevice.cpp
        dcc/NetworkDevice.h
        dcc/NetworkDeviceTasks.cpp
        dcc/NetworkDeviceTasks.h
        dcc/OutputDecoder.cpp
        dcc/OutputDecoder.h
        dcc/PinManager.cpp
        dcc/PinManager.h
        dcc/QuadInverter.cpp
        dcc/QuadInverter.h
        dcc/RemoteDecoder.cpp
        dcc/RemoteDecoder.h
        dcc/StateDecoder.cpp
        dcc/StateDecoder.h
        dcc/SensorDecoder.cpp
        dcc/SensorDecoder.h
        dcc/SignalDecoder.cpp
        dcc/SignalDecoder.h
        dcc/SimpleOutputDecoder.cpp
        dcc/SimpleOutputDecoder.h
		dcc/StorageManager.cpp
		dcc/StorageManager.h        
        dcc/TurnoutDecoder.cpp
        dcc/TurnoutDecoder.h 
        dcc/TurntableAutoInverterDecoder.cpp
        dcc/TurntableAutoInverterDecoder.h       
        dcc/VirtualDevice.cpp
        dcc/VirtualDevice.h
		dcc/VirtualSensorDecoder.cpp
		dcc/VirtualSensorDecoder.h
		dcc/VirtualTurnoutDecoder.cpp
		dcc/VirtualTurnoutDecoder.h        
        dispatcher/DispatcherService.cpp
        dispatcher/DispatcherService.h
		ln/ILoconetSlot.h
        ln/LoconetService.cpp
        ln/LoconetService.h
        ln/ThrottleService.cpp
        ln/ThrottleService.h
		shell/ScriptSystem.cpp
	    shell/ScriptSystem.h
        sys/BonjourService.cpp
        sys/BonjourService.h        
        sys/Broker.cpp        
        sys/Broker.h
        sys/FileWatcher.cpp
        sys/FileWatcher.h
        sys/EventHub.cpp
        sys/EventHub.h
        sys/Project.cpp
        sys/Project.h
        sys/Service.cpp
        sys/Service.h
		sys/ServiceFactory.cpp
        sys/ServiceFactory.h
        sys/SpecialFolders.cpp
        sys/SpecialFolders.h
        sys/Thinker.cpp
        sys/Thinker.h
        sys/Timeouts.h
        sys/ZeroConfSystem.cpp
        sys/ZeroConfSystem.h
        terminal/DeviceClearEEPromCmd.cpp
        terminal/DeviceClearEEPromCmd.h
        terminal/DeviceNetworkTestCmds.cpp
        terminal/DeviceNetworkTestCmds.h      
        terminal/DeviceRenameCmd.cpp
        terminal/DeviceRenameCmd.h
		terminal/ServoProgrammerCmds.cpp
		terminal/ServoProgrammerCmds.h
		terminal/TerminalClient.cpp
        terminal/TerminalClient.h
        terminal/TerminalCmd.cpp
        terminal/TerminalCmd.h
        terminal/TerminalService.cpp
        terminal/TerminalService.h
		terminal/TerminalUtils.cpp
        terminal/TerminalUtils.h
)

source_group("dcc"          FILES ${SRC_DCC})
source_group("dispatcher"   FILES ${SRC_DISPATCHER})
source_group("ln"           FILES ${SRC_LN})
source_group("shell"        FILES ${SRC_SHELL})
source_group("sys"          FILES ${SRC_SYS})
source_group("terminal"     FILES ${SRC_TERMINAL})

if(WIN32)	
    target_link_libraries(BrokerLib         
        PRIVATE bcrypt.lib
        PRIVATE CityHash
        PRIVATE Common
        PRIVATE fmt
        PRIVATE spdlog
        PRIVATE ldmonitor
	    PRIVATE LuaLib
		PRIVATE SharedLib		        
        PRIVATE sigslot
	)

	add_compile_definitions(_CRT_SECURE_NO_WARNINGS)       
      
else(WIN32)

	find_package(OpenSSL REQUIRED)

	target_link_libraries(BrokerLib 		
        CityHash
        Common 
        dl
        fmt 
        spdlog
        ldmonitor
		LuaLib
        OpenSSL::Crypto
		OpenSSL::SSL
        pthread	
        sigslot
		SharedLib
		stdc++fs 		
		uuid 		                        
	)

endif(WIN32)

target_compile_definitions(BrokerLib 
    PRIVATE
        # If the debug configuration pass the DEBUG define to the compiler 
        DEBUG
)

target_include_directories(BrokerLib 	
    PUBLIC ${DCCLite_SOURCE_DIR}/contrib/JsonCreator/include
    PUBLIC ${DCCLite_SOURCE_DIR}/contrib/lua-5.4.6/src
    PUBLIC ${DCCLite_SOURCE_DIR}/contrib/ldmonitor/include    
    PUBLIC ${DCCLite_SOURCE_DIR}/contrib/rapidjson/include
    PUBLIC ${DCCLite_SOURCE_DIR}/contrib/sigslot/include
    PUBLIC ${DCCLite_SOURCE_DIR}/contrib/sol2/include
    PUBLIC ${DCCLite_SOURCE_DIR}/src/Common
    PUBLIC ${DCCLite_SOURCE_DIR}/src/Embedded/lib/Shared
    PUBLIC ${DCCLite_SOURCE_DIR}/contrib/fmt/include
    PUBLIC ${DCCLite_SOURCE_DIR}/contrib/spdlog/include
    PRIVATE ${DCCLite_SOURCE_DIR}/contrib/magic_enum/include
)



