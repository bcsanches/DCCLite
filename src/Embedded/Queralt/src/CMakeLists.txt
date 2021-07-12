
set(QueraltLib_SRCS	
  main.cpp
)

set(QueraltLib_HDRS	
  main.h  
)

add_compile_definitions(QUERALTLIB_EXPORTS)
add_compile_definitions(_CRT_SECURE_NO_WARNINGS)

add_library(QueraltLib SHARED ${QueraltLib_SRCS} ${QueraltLib_HDRS})
target_compile_features(QueraltLib PUBLIC cxx_std_17)    

target_include_directories(QueraltLib 
  PRIVATE 
    ${DCCLite_SOURCE_DIR}/src/Common 
    ${DCCLite_SOURCE_DIR}/src/ArduinoLib
    ${DCCLite_SOURCE_DIR}/src/LiteDecoder/lib/Common
    ${DCCLite_SOURCE_DIR}/src/Embedded/lib/DecodersCommon
)

target_link_libraries(QueraltLib PUBLIC ArduinoLib Common DecodersCommonLib SharedLib fmt)

