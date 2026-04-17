
package_add_test(BrokerTycoonUnitTest	
	TycoonUnitTest.cpp	
)

target_include_directories(BrokerTycoonUnitTest PRIVATE
	${DCCLite_SOURCE_DIR}/src/BrokerSys
	${DCCLite_SOURCE_DIR}/src/BrokerExec
	${DCCLite_SOURCE_DIR}/src/BrokerTycoon
	${DCCLite_SOURCE_DIR}/src/Common	
	${GTEST_INCLUDE_DIRS}
	${GMOCK_INCLUDE_DIRS})  

target_link_libraries(BrokerTycoonUnitTest 
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


