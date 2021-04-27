
set(DecodersCommonLib_SRCS
    Console.cpp
	Decoder.cpp
    Strings.cpp
)

set(DecodersCommonLib_HDRS
    Console.h
    Decoder.h
    Pin.h
    Strings.h
)



add_library(DecodersCommonLib  ${DecodersCommonLib_SRCS} ${DecodersCommonLib_HDRS})
target_compile_features(DecodersCommonLib PUBLIC cxx_std_17)

target_include_directories(DecodersCommonLib 
  PUBLIC     
    ${DCCLite_SOURCE_DIR}/src/ArduinoLib
    ${DCCLite_SOURCE_DIR}/src/Embedded/lib/Shared
)