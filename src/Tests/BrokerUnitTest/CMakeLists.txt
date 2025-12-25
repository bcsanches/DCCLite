
package_add_test(BrokerUnitTest
	BitPackUnitTest.cpp
	EventHubTest.cpp
	FolderObjectTest.cpp
	GuidTest.cpp
	NetMessengerTest.cpp
	NmraUtilUnitTest.cpp
	ObjectPathUnitTest.cpp
	PacketTest.cpp
	ParserUnitTest.cpp
	PinManagerTest.cpp
	PrintfUnitTest.cpp
	ProjectUnitTest.cpp
	RNameTest.cpp
	SensorDecoderTest.cpp
	ServoTurnoutDecoderTest.cpp
	SignalDecoderTest.cpp
	SimpleOutputDecoderTest.cpp
	SocketTest.cpp
	StringViewTest.cpp
	ThinkerTest.cpp
	TycoonUnitTest.cpp
	TurntableAutoInverterTest.cpp
	UtilUnitTest.cpp
)

target_include_directories(BrokerUnitTest PRIVATE
	${DCCLite_SOURCE_DIR}/src/BrokerSys
	${DCCLite_SOURCE_DIR}/src/BrokerExec
	${DCCLite_SOURCE_DIR}/src/BrokerTycoon
	${DCCLite_SOURCE_DIR}/src/Common	
	${GTEST_INCLUDE_DIRS}
	${GMOCK_INCLUDE_DIRS})  

target_link_libraries(BrokerUnitTest 
	BrokerSysLib 
	BrokerExecLib
	BrokerTycoonLib
	CityHash
	Common 
	SharedLib 
	gtest 
	gtest_main 
	gmock
	fmt
    spdlog
)


#target_include_directories(MainTest PRIVATE ${PROJECT_SOURCE_DIR}/include/)

