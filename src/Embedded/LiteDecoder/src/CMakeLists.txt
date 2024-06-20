
set(LiteDecoderLib_SRCS
	Blinker.cpp
  Config.cpp  
  DecoderManager.cpp
  main.cpp
  NetUdp.cpp
  OutputDecoder.cpp
  QuadInverterDecoder.cpp
  ServoProgrammer.cpp  
  Session.cpp
  TurntableAutoInverterDecoder.cpp        
)

set(LiteDecoderLib_HDRS
	Blinker.h
  Config.h 
  DecoderManager.h  
  main.h
  NetUdp.h
  OutputDecoder.h
  QuadInverterDecoder.h
  ServoProgrammer.h    
  Session.h  
  TurntableAutoInverterDecoder.h    
)

add_compile_definitions(LITEDECODERLIB_EXPORTS DCCLITE_ARDUINO_EMULATOR)
add_compile_definitions(_CRT_SECURE_NO_WARNINGS)

add_library(LiteDecoderLib SHARED ${LiteDecoderLib_SRCS} ${LiteDecoderLib_HDRS})

target_include_directories(LiteDecoderLib 
    PRIVATE 
        ${DCCLite_SOURCE_DIR}/src/Common 
        ${DCCLite_SOURCE_DIR}/src/ArduinoLib
        ${DCCLite_SOURCE_DIR}/src/Embedded/lib/Shared
        ${DCCLite_SOURCE_DIR}/src/Embedded/lib/DecodersCommon
    )

if(WIN32)
    target_link_libraries(LiteDecoderLib PUBLIC ArduinoLib Common DecodersCommonLib SharedLib fmt)    
else(WIN32)
    target_link_libraries(LiteDecoderLib PUBLIC ArduinoLibStatic Common DecodersCommonLib SharedLib fmt)
endif()


