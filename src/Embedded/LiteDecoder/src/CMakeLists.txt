
set(LiteDecoderLib_SRCS
	Blinker.cpp
  Config.cpp  
  DecoderManager.cpp
  main.cpp
  NetUdp.cpp
  OutputDecoder.cpp  
  Session.cpp
)

set(LiteDecoderLib_HDRS
	Blinker.h
  Config.h 
  DecoderManager.h  
  main.h
  NetUdp.h
  OutputDecoder.h    
  Session.h  
)

add_compile_definitions(LITEDECODERLIB_EXPORTS)
add_compile_definitions(_CRT_SECURE_NO_WARNINGS)

add_library(LiteDecoderLib SHARED ${LiteDecoderLib_SRCS} ${LiteDecoderLib_HDRS})
target_compile_features(LiteDecoderLib PUBLIC cxx_std_17)    

target_include_directories(LiteDecoderLib 
  PRIVATE 
    ${DCCLite_SOURCE_DIR}/src/Common 
    ${DCCLite_SOURCE_DIR}/src/ArduinoLib
    ${DCCLite_SOURCE_DIR}/src/Embedded/lib/Shared
    ${DCCLite_SOURCE_DIR}/src/Embedded/lib/DecodersCommon
)

target_link_libraries(LiteDecoderLib PUBLIC ArduinoLib Common DecodersCommonLib SharedLib fmt)

