
# configure_file(Version.h Version.h @DCCLITE_VERSION)

# include_directories(${CMAKE_CURRENT_BINARY_DIR})

file(GLOB_RECURSE SRC_DCCLITE_SHARED
    "dcclite_shared/*.cpp"
    "dcclite_shared/*.h"
)

set(SharedLib_SRCS    
	dcclite_shared/Misc.cpp
    dcclite_shared/Parser.cpp
	dcclite_shared/Printf.cpp
)

set(SharedLib_HDRS
    dcclite_shared/ArduinoTypes.h
    dcclite_shared/BasicPin.h
    dcclite_shared/BitPack.h    
    dcclite_shared/SharedLibDefs.h
    dcclite_shared/GuidDefs.h  
    dcclite_shared/Misc.h
    dcclite_shared/Packet.h
    dcclite_shared/Parser.h  
	dcclite_shared/Printf.h
    dcclite_shared/Version.h
)

source_group("shared"          FILES ${SRC_DCCLITE_SHARED})

add_library(SharedLib STATIC ${SharedLib_SRCS} ${SharedLib_HDRS})

