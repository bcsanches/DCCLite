add_subdirectory(lib)
add_subdirectory(src)

INSTALL(FILES .pio/build/megaatmega2560/firmware.hex DESTINATION bin/ RENAME dcclite.firmware.mega2560.${DCCLITE_VERSION}.hex OPTIONAL)
INSTALL(FILES .pio/build/uno/firmware.hex DESTINATION bin/ RENAME dcclite.firmware.uno.${DCCLITE_VERSION}.hex OPTIONAL)
