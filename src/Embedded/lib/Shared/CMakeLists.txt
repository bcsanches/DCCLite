
# configure_file(Version.h.in Version.h @DCCLITE_VERSION)

# include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(SharedLib_SRCS    
	Misc.cpp
    Parser.cpp
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
    Version.h
)

add_library(SharedLib  ${SharedLib_SRCS} ${SharedLib_HDRS})
target_compile_features(SharedLib PUBLIC cxx_std_17)

