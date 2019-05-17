
INCLUDE_DIRECTORIES(	
  ${DCCLite_SOURCE_DIR}/contrib/fmt/include
  ${DCCLite_SOURCE_DIR}/contrib/JSonCreator/include
  ${DCCLite_SOURCE_DIR}/contrib/rapidjson/include
  ${DCCLite_SOURCE_DIR}/contrib/spdlog/include
  
  ${DCCLite_SOURCE_DIR}/src/Common
  ${DCCLite_SOURCE_DIR}/src/LiteDecoder/lib/Common/   
)

LINK_LIBRARIES(
 Common
 EmbeddedLib
)

SET(Broker_SRCS
	Broker.cpp
  DccLiteService.cpp
  Decoder.cpp
  Device.cpp
  main.cpp
  OutputDecoder.cpp
  Project.cpp
  SensorDecoder.cpp  
  Service.cpp
  SpecialFolders.cpp
  TerminalCmd.cpp
  TerminalService.cpp
)

SET(Broker_HDRS
	Broker.h
  DccLiteService.h
  Decoder.h
  Device.h
  OutputDecoder.h
  Project.h
  SensorDecoder.h
  Service.h
  SpecialFolders.h
  TerminalCmd.h
  TerminalService.h
)

add_executable(Broker  ${Broker_SRCS} ${Broker_HDRS})
target_compile_features(Broker PUBLIC cxx_std_17)

IF(WIN32)
    TARGET_LINK_LIBRARIES(Broker bcrypt.lib)
ENDIF(WIN32)
