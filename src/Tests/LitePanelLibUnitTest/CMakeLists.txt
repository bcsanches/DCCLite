package_add_test(LitePanelLibUnitTest
	BlockIndexTest.cpp
    BlockTest.cpp
	BlockManagerTest.cpp
    PanelTest.cpp
    RailLinkTest.cpp
	SimpleRailObjectTest.cpp
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