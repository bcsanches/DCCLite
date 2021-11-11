
if(WIN32)
	set(CommonPlatSrcs 
		ConsoleUtils_win.cpp
		GuidUtils_win.cpp
        SerialPort_win.cpp
		Sha1_win.cpp        
	)
    
    set(CommonPlatHeaders
        SerialPort_win.h
    )

else(WIN32)
	set(CommonPlatSrcs 
		ConsoleUtils_linux.cpp
		GuidUtils_linux.cpp
		Sha1_linux.cpp
		SerialPort_linux.cpp
	)
    
    set(CommonPlatHeaders
        SerialPort_linux.h
    )
    
endif(WIN32)

set(Common_SRCS
	${CommonPlatSrcs}
	Clock.cpp	
	GuidUtils.cpp
	LogUtils.cpp
	NetMessenger.cpp
	NmraUtil.cpp
	Object.cpp
	Sha1.cpp
	PathUtils.cpp	
	Socket.cpp
	Util.cpp
)

set(Common_HDRS	
    ${CommonPlatHeaders}	
	Clock.h
	ConsoleUtils.h
	defs.h
	FileSystem.h
	FmtUtils.h
	GuidUtils.h
	Log.h
	LogUtils.h
	NetMessenger.h
	NmraUtil.h
	Object.h
	PathUtils.h
    SerialPort.h
	Sha1.h
	Socket.h  
	Util.h
)

if(WIN32)

	add_compile_definitions(_CRT_SECURE_NO_WARNINGS)

endif(WIN32)

add_library(Common  ${Common_SRCS} ${Common_HDRS})
target_compile_features(Common PUBLIC cxx_std_17)     

target_include_directories(Common
  PUBLIC  ${DCCLite_SOURCE_DIR}/contrib/fmt/include
  PUBLIC  ${DCCLite_SOURCE_DIR}/contrib/JsonCreator/include
  PUBLIC  ${DCCLite_SOURCE_DIR}/contrib/rapidjson/include
          ${DCCLite_SOURCE_DIR}/contrib/spdlog/include
  PUBLIC  ${DCCLite_SOURCE_DIR}/src/Embedded/lib/Shared
  PRIVATE ${DCCLite_SOURCE_DIR}/contrib/magic_enum/include	
)
