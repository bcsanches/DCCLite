set(LitePanelLib_SRCS
    Block.cpp
    EditCmds.cpp
    MapObject.cpp
    MapObjectFactory.cpp
    Panel.cpp
    RailObject.cpp
    TileMap.cpp
    Render/Color.cpp
    Render/ColorStyle.cpp
    Render/IRenderer.cpp
    Render/TileMapRenderer.cpp
)

set(LitePanelLib_HDRS
    Block.h
    EditCmds.h
    LitePanelLibDefs.h
    MapObject.h
    MapObjectFactory.h
    Panel.h
    Point.h
    RailObject.h
    Size.h
    TileMap.h
    Render/Color.h
    Render/ColorStyle.h
    Render/IRenderer.h
    Render/TileMapRenderer.h
)

add_library(LitePanelLib
    ${LitePanelLib_SRCS}
    ${LitePanelLib_HDRS}
)

# Visual Studio filters
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}"
    FILES
        ${LitePanelLib_SRCS}
        ${LitePanelLib_HDRS}
)

if(WIN32)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()

target_include_directories(LitePanelLib
    PUBLIC  ${DCCLite_SOURCE_DIR}/src/LitePanelLib/
    PUBLIC  ${DCCLite_SOURCE_DIR}/contrib/fmt/include
    PUBLIC  ${DCCLite_SOURCE_DIR}/contrib/JsonCreator/include
            ${DCCLite_SOURCE_DIR}/contrib/rapidjson/include
    PRIVATE ${DCCLite_SOURCE_DIR}/contrib/magic_enum/include
)

target_link_libraries(LitePanelLib PRIVATE spdlog PRIVATE fmt)