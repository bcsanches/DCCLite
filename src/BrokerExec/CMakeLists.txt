file(GLOB_RECURSE SRC_DCC
    "exec/dcc/*.cpp"
    "exec/dcc/*.h"
)


add_library(
    BrokerExecLib	   	             
		exec/dcc/Address.cpp
        exec/dcc/Address.h
        exec/dcc/DccLiteService.cpp        
        exec/dcc/DccLiteService.h  
        exec/dcc/DccppService.cpp
        exec/dcc/DccppService.h
        exec/dcc/Decoder.cpp
        exec/dcc/Decoder.h
        exec/dcc/Device.cpp
        exec/dcc/Device.h      
        exec/dcc/IDccLiteService.h  
        exec/dcc/IDevice.h
        exec/dcc/IResettableObject.h
        exec/dcc/LocationManager.cpp
        exec/dcc/LocationManager.h
        exec/dcc/NetworkDevice.cpp
        exec/dcc/NetworkDevice.h
        exec/dcc/NetworkDeviceTasks.cpp
        exec/dcc/NetworkDeviceTasks.h
        exec/dcc/OutputDecoder.cpp
        exec/dcc/OutputDecoder.h
        exec/dcc/PinManager.cpp
        exec/dcc/PinManager.h
        exec/dcc/QuadInverter.cpp
        exec/dcc/QuadInverter.h
        exec/dcc/RemoteDecoder.cpp
        exec/dcc/RemoteDecoder.h
        exec/dcc/StateDecoder.cpp
        exec/dcc/StateDecoder.h
        exec/dcc/SensorDecoder.cpp
        exec/dcc/SensorDecoder.h
        exec/dcc/SignalDecoder.cpp
        exec/dcc/SignalDecoder.h
        exec/dcc/SimpleOutputDecoder.cpp
        exec/dcc/SimpleOutputDecoder.h
		exec/dcc/StorageManager.cpp
		exec/dcc/StorageManager.h        
        exec/dcc/TurnoutDecoder.cpp
        exec/dcc/TurnoutDecoder.h 
        exec/dcc/TurntableAutoInverterDecoder.cpp
        exec/dcc/TurntableAutoInverterDecoder.h       
        exec/dcc/VirtualDevice.cpp
        exec/dcc/VirtualDevice.h
		exec/dcc/VirtualSensorDecoder.cpp
		exec/dcc/VirtualSensorDecoder.h
		exec/dcc/VirtualTurnoutDecoder.cpp
		exec/dcc/VirtualTurnoutDecoder.h	      
)

source_group("dcc" FILES ${SRC_DCC})

if(WIN32)
    target_link_libraries(BrokerExecLib        
        PRIVATE Common        
		PRIVATE SharedLib
	)

	add_compile_definitions(_CRT_SECURE_NO_WARNINGS)       
      
else(WIN32)

	find_package(OpenSSL REQUIRED)

	target_link_libraries(BrokerExecLib        
        Common
        BrokerSysLib
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

target_compile_definitions(BrokerExecLib 
    PRIVATE
        # If the debug configuration pass the DEBUG define to the compiler 
        DEBUG
)

target_include_directories(BrokerExecLib 	
    PUBLIC ${DCCLite_SOURCE_DIR}/contrib/JsonCreator/include    
    PUBLIC ${DCCLite_SOURCE_DIR}/contrib/ldmonitor/include    
    PUBLIC ${DCCLite_SOURCE_DIR}/contrib/rapidjson/include
    PUBLIC ${DCCLite_SOURCE_DIR}/contrib/sigslot/include    
    PUBLIC ${DCCLite_SOURCE_DIR}/src/Common
    PUBLIC ${DCCLite_SOURCE_DIR}/src/BrokerSys
    PUBLIC ${DCCLite_SOURCE_DIR}/src/Embedded/lib/Shared
    PUBLIC ${DCCLite_SOURCE_DIR}/contrib/fmt/include
    PUBLIC ${DCCLite_SOURCE_DIR}/contrib/spdlog/include
    PRIVATE ${DCCLite_SOURCE_DIR}/contrib/magic_enum/include
)
