package_add_test(LitePanelLibUnitTest
	BlockIndexUnitTest.cpp
    BlockUnitTest.cpp
	BlockManagerUnitTest.cpp
    RailLinkTest.cpp
	SimpleRailObjectUnitTest.cpp
)

target_include_directories(LitePanelLibUnitTest PRIVATE
    ${DCCLite_SOURCE_DIR}/src/LitePanel/LitePanelLib
    ${GTEST_INCLUDE_DIRS}
    ${GMOCK_INCLUDE_DIRS}
)

target_link_libraries(LitePanelLibUnitTest
    LitePanelLib
    gtest
    gtest_main
    gmock
)