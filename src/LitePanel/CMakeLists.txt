
set(LitePanel_SRCS	
  main.cpp    
)

add_executable(LitePanel WIN32 ${LitePanel_SRCS} )
target_compile_features(LitePanel PUBLIC cxx_std_17)    

target_include_directories(LitePanel 
	PRIVATE ${DCCLite_SOURCE_DIR}/src/Common	
)

#set_target_properties( Broker PROPERTIES RUNTIME_OUTPUT_DIRECTORY bin/ )

if(WIN32)
    target_link_libraries(LitePanel 
		PRIVATE Common 
		PRIVATE EmbeddedLib 
		PRIVATE bcrypt.lib 
		PRIVATE fmt
    net
    core
    base
	)

	add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
    
    set_property(TARGET LitePanel PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
    set_property(TARGET LitePanel PROPERTY VS_DEBUGGER_COMMAND_ARGUMENTS "..\\data\\EFMR")
      
else(WIN32)

	find_package(OpenSSL REQUIRED)

	target_link_libraries(LitePanel 
		OpenSSL::Crypto
		OpenSSL::SSL
		Common 
		EmbeddedLib
		stdc++fs 
		fmt 
		uuid 		
    net
    core
    base
	)

endif(WIN32)
