
set(LiteDecoderLib_SRCS
	Blinker.cpp
  Config.cpp
  Console.cpp
  Decoder.cpp
  DecoderManager.cpp
  main.cpp
  NetUdp.cpp
  OutputDecoder.cpp
  SensorDecoder.cpp
  ServoTurnoutDecoder.cpp
  Session.cpp
  Storage.cpp
  Strings.cpp
)

set(LiteDecoderLib_HDRS
	Blinker.h
  Config.h
  Console.h
  Decoder.h
  DecoderManager.h
  LiteDecoder.h
  main.h
  NetUdp.h
  OutputDecoder.h
  Pin.h
  SensorDecoder.h
  ServoTurnoutDecoder.h
  Session.h
  Storage.h
  Strings.h
)

add_compile_definitions(LITEDECODERLIB_EXPORTS)
add_compile_definitions(_CRT_SECURE_NO_WARNINGS)

add_library(LiteDecoderLib SHARED ${LiteDecoderLib_SRCS} ${LiteDecoderLib_HDRS})
target_compile_features(LiteDecoderLib PUBLIC cxx_std_17)    

target_include_directories(LiteDecoderLib 
  PRIVATE 
    ${DCCLite_SOURCE_DIR}/src/Common 
    ${DCCLite_SOURCE_DIR}/src/ArduinoLib
    ${DCCLite_SOURCE_DIR}/src/LiteDecoder/lib/Common
)

target_link_libraries(LiteDecoderLib PUBLIC ArduinoLib Common EmbeddedLib fmt)

