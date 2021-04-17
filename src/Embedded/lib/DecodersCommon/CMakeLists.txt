
set(DecodersCommonLib_SRCS
	Decoder.cpp
)

set(DecodersCommonLib_HDRS
  Decoder.h
  Pin.h
)



add_library(DecodersCommonLib  ${DecodersCommonLib_SRCS} ${DecodersCommonLib_HDRS})
target_compile_features(DecodersCommonLib PUBLIC cxx_std_17)

target_include_directories(DecodersCommonLib 
  PUBLIC     
    ${DCCLite_SOURCE_DIR}/src/ArduinoLib
    ${DCCLite_SOURCE_DIR}/src/Embedded/lib/Shared
)