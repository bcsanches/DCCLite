file(GLOB_RECURSE SRC_EXEC
    "exec/*.cpp"
    "exec/*.h"
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
        exec/DccAddress.cpp
        exec/DccAddress.h
        exec/DccLiteService.cpp
        exec/DccLiteService_script.cpp
        exec/DccLiteService.h  
        exec/DccppService.cpp
        exec/DccppService.h
        exec/Decoder.cpp
        exec/Decoder.h
        exec/Device.cpp
        exec/Device.h      
        exec/IDccLiteService.h  
        exec/IDevice.h
        exec/IResettableObject.h
        exec/LiteManager.cpp
        exec/LiteManager.h
        exec/LocationManager.cpp
        exec/LocationManager.h
        exec/NetworkDevice.cpp
        exec/NetworkDevice.h
        exec/NetworkDeviceTasks.cpp
        exec/NetworkDeviceTasks.h
        exec/OutputDecoder.cpp
        exec/OutputDecoder.h
        exec/PinManager.cpp
        exec/PinManager.h
        exec/QuadInverter.cpp
        exec/QuadInverter.h
        exec/RemoteDecoder.cpp
        exec/RemoteDecoder.h
        exec/StateDecoder.cpp
        exec/StateDecoder.h
        exec/SensorDecoder.cpp
        exec/SensorDecoder.h
        exec/SignalDecoder.cpp
        exec/SignalDecoder.h
        exec/SimpleOutputDecoder.cpp
        exec/SimpleOutputDecoder.h
		exec/StorageManager.cpp
		exec/StorageManager.h        
        exec/TurnoutDecoder.cpp
        exec/TurnoutDecoder.h 
        exec/TurntableAutoInverterDecoder.cpp
        exec/TurntableAutoInverterDecoder.h       
        exec/VirtualDevice.cpp
        exec/VirtualDevice.h
		exec/VirtualSensorDecoder.cpp
		exec/VirtualSensorDecoder.h
		exec/VirtualTurnoutDecoder.cpp
		exec/VirtualTurnoutDecoder.h        
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
		terminal/CmdHost.cpp
		terminal/CmdHost.h
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
		terminal/TerminalContext.cpp
		terminal/TerminalContext.h		
        terminal/TerminalService.cpp
        terminal/TerminalService.h
		terminal/TerminalUtils.cpp
        terminal/TerminalUtils.h
)

source_group("exec"          FILES ${SRC_EXEC})
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



