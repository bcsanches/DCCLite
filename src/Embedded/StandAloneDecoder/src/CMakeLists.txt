
set(StandAloneDecoderLib_SRCS	
  main.cpp
)

set(StandAloneDecoderLib_HDRS	
  main.h  
)

add_compile_definitions(STANDALONEDECODERLIB_EXPORTS DCCLITE_ARDUINO_EMULATOR)
add_compile_definitions(_CRT_SECURE_NO_WARNINGS)

add_library(StandAloneDecoderLib SHARED ${StandAloneDecoderLib_SRCS} ${StandAloneDecoderLib_HDRS})

target_include_directories(StandAloneDecoderLib 
  PRIVATE 
    ${DCCLite_SOURCE_DIR}/src/Common 
    ${DCCLite_SOURCE_DIR}/src/ArduinoLib
    ${DCCLite_SOURCE_DIR}/src/LiteDecoder/lib/Common
    ${DCCLite_SOURCE_DIR}/src/Embedded/lib/DecodersCommon
)

target_link_libraries(StandAloneDecoderLib PUBLIC ArduinoLib Common DecodersCommonLib SharedLib fmt)

