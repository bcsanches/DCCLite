
set(StandAloneDecoderLib_SRCS	
  main.cpp
)

set(StandAloneDecoderLib_HDRS	
  main.h  
)

add_compile_definitions(STANDALONEDECODERLIB_EXPORTS)
add_compile_definitions(_CRT_SECURE_NO_WARNINGS)

add_library(StandAloneDecoderLib SHARED ${StandAloneDecoderLib_SRCS} ${StandAloneDecoderLib_HDRS})
target_compile_features(StandAloneDecoderLib PUBLIC cxx_std_17)    

target_include_directories(StandAloneDecoderLib 
  PRIVATE 
    ${DCCLite_SOURCE_DIR}/src/Common 
    ${DCCLite_SOURCE_DIR}/src/ArduinoLib
    ${DCCLite_SOURCE_DIR}/src/LiteDecoder/lib/Common
)

target_link_libraries(StandAloneDecoderLib PUBLIC ArduinoLib Common EmbeddedLib fmt)

