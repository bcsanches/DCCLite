
file(GLOB_RECURSE SRC_SHELL_LN
    "shell/ln/*.cpp"
    "shell/ln/*.h"
)

file(GLOB_RECURSE SRC_SHELL_DISPATCHER
    "shell/dispatcher/*.cpp"
    "shell/dispatcher/*.h"
)

file(GLOB_RECURSE SRC_SHELL_SCRIPT
    "shell/script/*.cpp"
    "shell/script/*.h"
)

file(GLOB_RECURSE SRC_SHELL_TERMINAL
    "shell/terminal/*.cpp"
    "shell/terminal/*.h"
)

add_library(
    BrokerShellLib	   	             
		shell/dispatcher/DispatcherService.cpp
        shell/dispatcher/DispatcherService.h
		shell/dispatcher/DispatcherService_detail.h
		shell/ln/ILoconetSlot.h
        shell/ln/LoconetService.cpp
        shell/ln/LoconetService.h
        shell/ln/ThrottleService.cpp
        shell/ln/ThrottleService.h
		shell/script/Proxies.cpp
		shell/script/Proxies.h
		shell/script/ScriptService.cpp
	    shell/script/ScriptService.h
		shell/terminal/CmdHostService.cpp
		shell/terminal/CmdHostService.h
        shell/terminal/DccTerminalCmds.cpp
        shell/terminal/DccTerminalCmds.h
        shell/terminal/DeviceClearEEPromCmd.cpp
        shell/terminal/DeviceClearEEPromCmd.h
        shell/terminal/DeviceNetworkTestCmds.cpp
        shell/terminal/DeviceNetworkTestCmds.h      
        shell/terminal/DeviceRenameCmd.cpp
        shell/terminal/DeviceRenameCmd.h
		shell/terminal/ServoProgrammerCmds.cpp
		shell/terminal/ServoProgrammerCmds.h
		shell/terminal/TerminalClient.cpp
        shell/terminal/TerminalClient.h
        shell/terminal/TerminalCmd.cpp
        shell/terminal/TerminalCmd.h
        shell/terminal/TerminalCmdProvider.h
		shell/terminal/TerminalContext.cpp
		shell/terminal/TerminalContext.h		
        shell/terminal/TerminalService.cpp
        shell/terminal/TerminalService.h
        shell/terminal/TerminalServiceCmds.cpp
        shell/terminal/TerminalServiceCmds.h
		shell/terminal/TerminalUtils.cpp
        shell/terminal/TerminalUtils.h      
)

source_group("dispatcher"   	FILES ${SRC_SHELL_DISPATCHER})
source_group("ln"				FILES ${SRC_SHELL_LN})
source_group("script"        	FILES ${SRC_SHELL_SCRIPT})
source_group("terminal"     	FILES ${SRC_SHELL_TERMINAL})

if(WIN32)
    target_link_libraries(BrokerShellLib        
        PRIVATE Common
	    PRIVATE LuaLib
		PRIVATE SharedLib
	)

	add_compile_definitions(_CRT_SECURE_NO_WARNINGS)       
      
else(WIN32)

	find_package(OpenSSL REQUIRED)

	target_link_libraries(BrokerShellLib        
        Common 
        dl
        fmt 
        spdlog
        ldmonitor
		LuaLib        
        pthread
        sigslot
		SharedLib
		stdc++fs
	)

endif(WIN32)

target_compile_definitions(BrokerShellLib 
    PRIVATE
        # If the debug configuration pass the DEBUG define to the compiler 
        DEBUG
)

target_include_directories(BrokerShellLib 	
    PUBLIC ${DCCLite_SOURCE_DIR}/contrib/JsonCreator/include
    PUBLIC ${DCCLite_SOURCE_DIR}/contrib/lua-5.4.6/src
    PUBLIC ${DCCLite_SOURCE_DIR}/contrib/ldmonitor/include    
    PUBLIC ${DCCLite_SOURCE_DIR}/contrib/rapidjson/include
    PUBLIC ${DCCLite_SOURCE_DIR}/contrib/sigslot/include
    PUBLIC ${DCCLite_SOURCE_DIR}/contrib/sol2/include
    PUBLIC ${DCCLite_SOURCE_DIR}/src/Common
    PUBLIC ${DCCLite_SOURCE_DIR}/src/BrokerSys
    PUBLIC ${DCCLite_SOURCE_DIR}/src/BrokerExec    
    PUBLIC ${DCCLite_SOURCE_DIR}/src/Embedded/lib/Shared
    PUBLIC ${DCCLite_SOURCE_DIR}/contrib/fmt/include
    PUBLIC ${DCCLite_SOURCE_DIR}/contrib/spdlog/include
    PRIVATE ${DCCLite_SOURCE_DIR}/contrib/magic_enum/include
)
