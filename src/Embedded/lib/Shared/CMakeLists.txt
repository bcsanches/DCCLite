
# configure_file(Version.h Version.h @DCCLITE_VERSION)

# include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(SharedLib_SRCS    
	Misc.cpp
    Parser.cpp
	Printf.cpp
)

set(SharedLib_HDRS
    ArduinoTypes.h
    BasicPin.h
    BitPack.h    
    SharedLibDefs.h
    Guid.h  
    Misc.h
    Packet.h
    Parser.h  
	Printf.h
    Version.h
)

add_library(SharedLib STATIC ${SharedLib_SRCS} ${SharedLib_HDRS})

