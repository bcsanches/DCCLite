
file(GLOB_RECURSE SRC_TYCOON
    "tycoon/*.cpp"
    "tycoon/*.h"
)

file(GLOB_RECURSE SRC_TYCOON_DETAIL
    "tycoon/detail/*.cpp"
    "tycoon/detail/*.h"
)

add_library(
    BrokerTycoonLib
        tycoon/Cargo.cpp
        tycoon/Cargo.h
        tycoon/CarType.cpp
        tycoon/CarType.h
        tycoon/CmdsInitService.cpp
        tycoon/CmdsInitService.h
        tycoon/detail/CargoInfo.cpp
        tycoon/detail/CargoInfo.h
        tycoon/detail/CargoProducer.cpp
        tycoon/detail/CargoProducer.h
        tycoon/detail/Spot.cpp
        tycoon/detail/Spot.h
        tycoon/FastClock.cpp
        tycoon/FastClock.h
        tycoon/FastClockDefs.h
        tycoon/FastClockUtils.cpp
        tycoon/FastClockUtils.h
        tycoon/FastClockThinker.cpp
        tycoon/FastClockThinker.h
        tycoon/Industry.cpp
        tycoon/Industry.h        
        tycoon/Location.cpp
        tycoon/Location.h
		tycoon/TycoonService.cpp
		tycoon/TycoonService.h        
        tycoon/TycoonService_Load.cpp        
)

source_group("tycoon"	FILES ${SRC_TYCOON})
source_group("detail"	FILES ${SRC_TYCOON_DETAIL})

if(WIN32)
    target_link_libraries(BrokerTycoonLib        
        PRIVATE Common	    
		PRIVATE SharedLib
	)

	add_compile_definitions(_CRT_SECURE_NO_WARNINGS)       
      
else(WIN32)

	find_package(OpenSSL REQUIRED)

	target_link_libraries(BrokerTycoonLib        
        Common 
        dl
        fmt 
        spdlog
        ldmonitor		
        pthread
        sigslot
		SharedLib
		stdc++fs
	)

endif(WIN32)

target_compile_definitions(BrokerTycoonLib 
    PRIVATE
        # If the debug configuration pass the DEBUG define to the compiler 
        DEBUG
)

target_include_directories(BrokerTycoonLib 	
    PUBLIC ${DCCLite_SOURCE_DIR}/contrib/JsonCreator/include
    PUBLIC ${DCCLite_SOURCE_DIR}/contrib/lua-5.4.6/src
    PUBLIC ${DCCLite_SOURCE_DIR}/contrib/ldmonitor/include    
    PUBLIC ${DCCLite_SOURCE_DIR}/contrib/rapidjson/include
    PUBLIC ${DCCLite_SOURCE_DIR}/contrib/sigslot/include    
    PUBLIC ${DCCLite_SOURCE_DIR}/src/Common
    PUBLIC ${DCCLite_SOURCE_DIR}/src/BrokerSys
    PUBLIC ${DCCLite_SOURCE_DIR}/src/BrokerExec
    PUBLIC ${DCCLite_SOURCE_DIR}/src/BrokerShell
    PUBLIC ${DCCLite_SOURCE_DIR}/src/Embedded/lib/Shared
    PUBLIC ${DCCLite_SOURCE_DIR}/contrib/fmt/include
    PUBLIC ${DCCLite_SOURCE_DIR}/contrib/spdlog/include
    PRIVATE ${DCCLite_SOURCE_DIR}/contrib/magic_enum/include
)
