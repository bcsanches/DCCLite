
file(GLOB_RECURSE SRC_SYS
	"sys/*.cpp"
	"sys/*.h"
)

add_library(
	BrokerSysLib       	
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
		sys/Thinker.cpp
		sys/Thinker.h
		sys/Timeouts.h
		sys/ZeroConfSystem.cpp
		sys/ZeroConfSystem.h
)

source_group("sys"          		FILES ${SRC_SYS})

if(WIN32)	
	target_link_libraries(BrokerSysLib      
		PUBLIC spdlog
	)

	add_compile_definitions(_CRT_SECURE_NO_WARNINGS)       
	  
else(WIN32)

	find_package(OpenSSL REQUIRED)

	target_link_libraries(BrokerSysLib	
		CityHash
		Common 
		dl
		fmt 
		spdlog
		ldmonitor		
		OpenSSL::Crypto
		OpenSSL::SSL
		pthread	
		sigslot
		SharedLib
		stdc++fs		 		                        
	)

endif(WIN32)

target_compile_definitions(BrokerSysLib
	PRIVATE
		# If the debug configuration pass the DEBUG define to the compiler 
		DEBUG
)

target_include_directories(BrokerSysLib	
	PUBLIC ${DCCLite_SOURCE_DIR}/contrib/JsonCreator/include    
	PUBLIC ${DCCLite_SOURCE_DIR}/contrib/ldmonitor/include    
	PUBLIC ${DCCLite_SOURCE_DIR}/contrib/rapidjson/include
	PUBLIC ${DCCLite_SOURCE_DIR}/contrib/sigslot/include
	PUBLIC ${DCCLite_SOURCE_DIR}/src/Common
	PUBLIC ${DCCLite_SOURCE_DIR}/src/Embedded/lib/Shared
	PUBLIC ${DCCLite_SOURCE_DIR}/contrib/fmt/include
	PUBLIC ${DCCLite_SOURCE_DIR}/contrib/spdlog/include
	PRIVATE ${DCCLite_SOURCE_DIR}/contrib/magic_enum/include
)



