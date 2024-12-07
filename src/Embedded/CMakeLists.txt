add_subdirectory(lib)

if(${DCCLITE_MSVC})
    add_subdirectory(LiteDecoder)
    add_subdirectory(Queralt)
    add_subdirectory(StandAloneDecoder)


    INSTALL(FILES .pio/build/megaatmega2560/firmware.hex DESTINATION bin/ RENAME dcclite.firmware.mega2560.${DCCLITE_VERSION}.hex OPTIONAL)
    INSTALL(FILES .pio/build/uno/firmware.hex DESTINATION bin/ RENAME dcclite.firmware.uno.${DCCLITE_VERSION}.hex OPTIONAL)
    INSTALL(FILES .pio/build/leonardo/firmware.hex DESTINATION bin/ RENAME dcclite.firmware.leonardo.${DCCLITE_VERSION}.hex OPTIONAL)
endif()
