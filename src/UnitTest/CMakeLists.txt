enable_testing()
include(GoogleTest)

#add_compile_definitions(ARDUINOLIB_EXPORTS)
#add_compile_definitions(_CRT_SECURE_NO_WARNINGS)

add_subdirectory("${PROJECT_SOURCE_DIR}/contrib/googletest" "contrib/googletest")

# The next line is optional, but keeps your CACHE cleaner:
mark_as_advanced(
    BUILD_GMOCK BUILD_GTEST BUILD_SHARED_LIBS
    gmock_build_tests gtest_build_samples gtest_build_tests
    gtest_disable_pthreads gtest_force_shared_crt gtest_hide_internal_symbols
)

set_target_properties(gtest PROPERTIES FOLDER contrib)
set_target_properties(gtest_main PROPERTIES FOLDER contrib)
set_target_properties(gmock PROPERTIES FOLDER contrib)
set_target_properties(gmock_main PROPERTIES FOLDER contrib)

# As sugested by the docs: http://cliutils.gitlab.io/modern-cmake/chapters/testing/googletest.html
macro(package_add_test TESTNAME)
    # create an executable in which the tests will be stored
    add_executable(${TESTNAME} ${ARGN})
    # link the Google test infrastructure, mocking library, and a default main function to
    # the test executable.  Remove g_test_main if writing your own main function.
    target_link_libraries(${TESTNAME} gtest gmock gtest_main)
    # gtest_discover_tests replaces gtest_add_tests,
    # see https://cmake.org/cmake/help/v3.10/module/GoogleTest.html for more options to pass to it
    gtest_discover_tests(${TESTNAME}
        # set a working directory so your project root so that you can find test data via paths relative to the project root
        WORKING_DIRECTORY ${PROJECT_DIR}
        PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_DIR}"
    )
    set_target_properties(${TESTNAME} PROPERTIES FOLDER utests)
endmacro()

package_add_test(UnitTest
	BitPackUnitTest.cpp
	EventHubTest.cpp
	NetMessengerTest.cpp
   	NmraUtilUnitTest.cpp
  	ObjectPathUnitTest.cpp
  	PacketTest.cpp
  	ParserUnitTest.cpp
	PrintfUnitTest.cpp
  	ProjectUnitTest.cpp
	RNameTest.cpp
  	ServoTurnoutDecoderTest.cpp
  	SignalDecoderTest.cpp
  	SocketTest.cpp
  	ThinkerTest.cpp
  	UtilUnitTest.cpp
)

target_compile_features(UnitTest PUBLIC cxx_std_17)  

target_include_directories(UnitTest PRIVATE
    ${DCCLite_SOURCE_DIR}/src/BrokerLib 
    ${DCCLite_SOURCE_DIR}/src/Common
    ${DCCLite_SOURCE_DIR}/src/LiteDecoder/src     
    ${GTEST_INCLUDE_DIRS}
    ${GMOCK_INCLUDE_DIRS})  

target_link_libraries(UnitTest BrokerLib Common SharedLib LiteDecoderLib fmt gtest gtest_main gmock)

#target_include_directories(MainTest PRIVATE ${PROJECT_SOURCE_DIR}/include/)

