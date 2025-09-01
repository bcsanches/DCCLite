
#add_compile_definitions(ARDUINOLIB_EXPORTS)
add_compile_definitions(DCCLITE_ARDUINO_EMULATOR)
add_compile_definitions(_CRT_SECURE_NO_WARNINGS)

package_add_test(LiteDecoderUnitTest
	SensorTest.cpp
)

target_include_directories(LiteDecoderUnitTest PRIVATE
	${DCCLite_SOURCE_DIR}/src/ArduinoLib
	${DCCLite_SOURCE_DIR}/src/BrokerSys
	${DCCLite_SOURCE_DIR}/src/BrokerExec
	${DCCLite_SOURCE_DIR}/src/Common
	${DCCLite_SOURCE_DIR}/src/Embedded/lib/DecodersCommon	
	${GTEST_INCLUDE_DIRS}
	${GMOCK_INCLUDE_DIRS})  

target_link_libraries(
	LiteDecoderUnitTest
	ArduinoLib
	BrokerSysLib 
	BrokerExecLib 
	CityHash
	Common
	DecodersCommonLib	
	SharedLib 
	gtest 
	gtest_main 
	gmock
)


#target_include_directories(MainTest PRIVATE ${PROJECT_SOURCE_DIR}/include/)

