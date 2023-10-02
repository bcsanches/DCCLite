
add_library(
    Common  
        Clock.cpp
        Clock.h
        ConsoleUtils.h	
        defs.h
        FileSystem.h
        FmtUtils.h
        GuidUtils.cpp
        GuidUtils.h
        Log.h
        LogUtils.cpp
        LogUtils.h
        NetMessenger.cpp
        NetMessenger.h
        NmraUtil.cpp
        NmraUtil.h
        Object.cpp
        Object.h
	    RName.cpp
	    RName.h
        Sha1.cpp
        PathUtils.cpp
        PathUtils.h	
        SerialPort.h
        Sha1.h
        Socket.cpp
        Socket.h
        Util.cpp
        Util.h
)

if(WIN32)

	target_compile_definitions(Common PRIVATE _CRT_SECURE_NO_WARNINGS)
    
    target_sources(Common PRIVATE
        ConsoleUtils_win.cpp
        GuidUtils_win.cpp
        SerialPort_win.cpp
        SerialPort_win.h
		Sha1_win.cpp
    )

else(WIN32)

    target_sources(Common PRIVATE
		ConsoleUtils_linux.cpp
		GuidUtils_linux.cpp
		Sha1_linux.cpp
		SerialPort_linux.cpp
        SerialPort_linux.h
	)   
     
endif(WIN32)
		
target_compile_features(Common PUBLIC cxx_std_17)     

target_link_libraries(Common PUBLIC CityHash fmt SharedLib)

target_include_directories(Common
  PUBLIC  ${DCCLite_SOURCE_DIR}/contrib/cityhash/
  PUBLIC  ${DCCLite_SOURCE_DIR}/contrib/fmt/include
  PUBLIC  ${DCCLite_SOURCE_DIR}/contrib/JsonCreator/include
  PUBLIC  ${DCCLite_SOURCE_DIR}/contrib/rapidjson/include
          ${DCCLite_SOURCE_DIR}/contrib/spdlog/include
  PUBLIC  ${DCCLite_SOURCE_DIR}/src/Embedded/lib/Shared
  PRIVATE ${DCCLite_SOURCE_DIR}/contrib/magic_enum/include	
)
