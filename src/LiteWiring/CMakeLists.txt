
set(LiteWiring_SRCS	
  Cable.cpp
  Device.cpp
  main.cpp    
  Project.cpp
  ProjectItem.cpp
  ProjectView.cpp
)

set(LiteWiring_HEADERS
  Cable.h
  Device.h    
  LiteWiring.h
  Project.h
  ProjectItem.h
  ProjectView.h
)

add_executable(LiteWiring WIN32 ${LiteWiring_SRCS} ${LiteWiring_HEADERS})
target_compile_features(LiteWiring PUBLIC cxx_std_17)    

target_include_directories(LiteWiring 
	PRIVATE ${DCCLite_SOURCE_DIR}/src/Common	
)

#set_target_properties( Broker PROPERTIES RUNTIME_OUTPUT_DIRECTORY bin/ )

if(WIN32)
    target_link_libraries(LiteWiring 
		PRIVATE Common 
		PRIVATE SharedLib 
		PRIVATE bcrypt.lib 
		PRIVATE fmt
    wx::net
    wx::core
    wx::base
	)

	add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
	add_compile_definitions(WXUSINGDLL)
    
  set_property(TARGET LiteWiring PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
  set_property(TARGET LiteWiring PROPERTY VS_DEBUGGER_COMMAND_ARGUMENTS "..\\data\\EFMR")
  set_property(TARGET LiteWiring PROPERTY VS_DEBUGGER_ENVIRONMENT "PATH=${LIBRARY_OUTPUT_DIRECTORY_DEBUG};%PATH%")
      
else(WIN32)

	find_package(OpenSSL REQUIRED)

	target_link_libraries(LiteWiring 
		OpenSSL::Crypto
		OpenSSL::SSL
		Common 
		EmbeddedLib
		stdc++fs 
		fmt 
		uuid 		
    wx::net
    wx::core
    wx::base
	)

endif(WIN32)

