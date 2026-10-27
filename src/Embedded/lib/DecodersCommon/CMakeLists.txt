
set(DecodersCommonLib_SRCS
    BlinkOutputDecoder.cpp
    Console.cpp
	Decoder.cpp
    LocalDecoderManager.cpp
    OutputDecoder.cpp
    OutputDecoderState.cpp
    SensorDecoder.cpp
    ServoTurnoutDecoder.cpp
    Storage.cpp
    Strings.cpp
)

set(DecodersCommonLib_HDRS
	BlinkOutputDecoder.h
    Console.h
    Decoder.h
    LocalDecoderManager.h
	OutputDecoder.h
    OutputDecoderState.h
    Pin.h
    SensorDecoder.h
    ServoTurnoutDecoder.h
    Storage.h
    Strings.h
)

add_compile_definitions(_CRT_SECURE_NO_WARNINGS DCCLITE_ARDUINO_EMULATOR)

add_library(DecodersCommonLib  ${DecodersCommonLib_SRCS} ${DecodersCommonLib_HDRS})

target_include_directories(DecodersCommonLib 
  PUBLIC     
    ${DCCLite_SOURCE_DIR}/src/ArduinoLib
    ${DCCLite_SOURCE_DIR}/src/Embedded/lib/Shared
)
