
INCLUDE_DIRECTORIES(	
  ${DCCLite_SOURCE_DIR}/contrib/fmt/include
  ${DCCLite_SOURCE_DIR}/contrib/JSonCreator/include
  ${DCCLite_SOURCE_DIR}/contrib/rapidjson/include
  ${DCCLite_SOURCE_DIR}/contrib/spdlog/include
  
  ${DCCLite_SOURCE_DIR}/src/LiteDecoder/lib/Common
)

SET(Common_SRCS
	Clock.cpp
  ConsoleUtils.cpp
  GuidUtils.cpp
  LogUtils.cpp
  NetMessenger.cpp
  Object.cpp
  PathUtils.cpp
  Sha1.cpp
  Socket.cpp
  Util.cpp
)

SET(Common_HDRS
	ClassInfo.h
  Clock.h
  ConsoleUtils.h
  defs.h
  FmtUtils.h
  GuidUtils.h
  Log.h
  LogUtils.h
  NetMessenger.h
  Object.h
  PathUtils.h
  Sha1.h
  Socket.h  
  Util.h
)

add_library(Common  ${Common_SRCS} ${Common_HDRS})
target_compile_features(Common PUBLIC cxx_std_17)

