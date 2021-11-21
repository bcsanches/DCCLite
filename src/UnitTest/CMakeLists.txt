
set(UnitTest_SRCS
	BitPackUnitTest.cpp
    NetMessengerTest.cpp
    NmraUtilUnitTest.cpp
  ObjectPathUnitTest.cpp
  PacketTest.cpp
  ParserUnitTest.cpp
  ProjectUnitTest.cpp
  ServoTurnoutDecoderTest.cpp
  SignalDecoderTest.cpp
  SocketTest.cpp
  UtilUnitTest.cpp
)

set(UnitTest_HDRS
    BrokerMockups.h
)

# GTest
enable_testing()

#add_compile_definitions(ARDUINOLIB_EXPORTS)
#add_compile_definitions(_CRT_SECURE_NO_WARNINGS)

#add_library(UnitTest SHARED ${UnitTest_SRCS} ${UnitTest_HDRS})
add_executable(UnitTest ${UnitTest_SRCS} ${UnitTest_HDRS})
target_compile_features(UnitTest PUBLIC cxx_std_17)    

target_include_directories(UnitTest PRIVATE
    ${DCCLite_SOURCE_DIR}/src/BrokerLib 
    ${DCCLite_SOURCE_DIR}/src/Common
    ${DCCLite_SOURCE_DIR}/src/LiteDecoder/src     
    ${GTEST_INCLUDE_DIRS}
    ${GMOCK_INCLUDE_DIRS})

target_link_libraries(UnitTest PUBLIC BrokerLib Common SharedLib LiteDecoderLib fmt gtest gtest_main gmock)

