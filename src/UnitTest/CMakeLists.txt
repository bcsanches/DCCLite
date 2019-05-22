
set(UnitTest_SRCS
	BitPackUnitTest.cpp
  ObjectPathUnitTest.cpp
  ProjectUnitTest.cpp
  stdafx.cpp
  UtilUnitTest.cpp
)

set(UnitTest_HDRS
	stdafx.h
  targetver.h  
)

#add_compile_definitions(ARDUINOLIB_EXPORTS)
#add_compile_definitions(_CRT_SECURE_NO_WARNINGS)

add_library(UnitTest SHARED ${UnitTest_SRCS} ${UnitTest_HDRS})
target_compile_features(ArduinoLib PUBLIC cxx_std_17)    

target_include_directories(UnitTest PRIVATE ${DCCLite_SOURCE_DIR}/src/Common)

target_link_libraries(UnitTest PUBLIC Common fmt)

