
set(ArduinoLib_SRCS
	Arduino.cpp
  DynamicLibrary.cpp
  EEPROM.cpp
  Ethercard.cpp
  Serial.cpp
)

set(ArduinoLib_HDRS
	Arduino.h
  ArduinoDefs.h
  ArduinoLib.h
  ArduinoLibDefs.h
  DynamicLibrary.h
  EEPROM.h
  EEPROMLib.h
  Ethercard.h
  Serial.h
  Servo.h
  avr/pgmspace.h
)

add_compile_definitions(ARDUINOLIB_EXPORTS)
add_compile_definitions(_CRT_SECURE_NO_WARNINGS)

add_library(ArduinoLib SHARED ${ArduinoLib_SRCS} ${ArduinoLib_HDRS})
target_compile_features(ArduinoLib PUBLIC cxx_std_17)    

target_include_directories(ArduinoLib PRIVATE ${DCCLite_SOURCE_DIR}/src/Common)

target_link_libraries(ArduinoLib PUBLIC Common fmt)

