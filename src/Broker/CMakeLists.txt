
add_executable(Broker
	main.cpp
)

target_include_directories(Broker 
	PRIVATE ${DCCLite_SOURCE_DIR}/src/BrokerLib
	PRIVATE ${DCCLite_SOURCE_DIR}/src/BrokerShell
	PRIVATE ${DCCLite_SOURCE_DIR}/src/BrokerSys
    PRIVATE ${DCCLite_SOURCE_DIR}/contrib/spdlog/include
    PRIVATE ${DCCLite_SOURCE_DIR}/contrib/fmt/include
)

#set_target_properties( Broker PROPERTIES RUNTIME_OUTPUT_DIRECTORY bin/ )

if(WIN32)
    target_link_libraries(Broker   
		PRIVATE bcrypt.lib
        PRIVATE BrokerSysLib
		PRIVATE BrokerExecLib
		PRIVATE BrokerShellLib		
		PRIVATE CityHash
		PRIVATE Common
		PRIVATE fmt
		PRIVATE ldmonitor        
	)

	add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
    
    set_property(TARGET Broker PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
    set_property(TARGET Broker PROPERTY VS_DEBUGGER_COMMAND_ARGUMENTS "..\\data\\EFMR")
      
else(WIN32)

	target_link_libraries(Broker         
		PRIVATE BrokerSysLib
		PRIVATE BrokerExecLib
		PRIVATE BrokerShellLib
	)

endif(WIN32)

