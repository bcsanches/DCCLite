
file(GLOB_RECURSE SRC_DCCLITE
    "dcclite/*.cpp"
    "dcclite/*.h"
)

add_library(	
	Common
	dcclite/Benchmark.cpp
	dcclite/Benchmark.h
	dcclite/Clock.cpp
	dcclite/Clock.h
	dcclite/Console.h
	dcclite/dcclite.cpp
	dcclite/dcclite.h
	dcclite/defs.h
	dcclite/FileSystem.h
	dcclite/FolderObject.cpp
	dcclite/FolderObject.h
	dcclite/FmtUtils.h
	dcclite/Guid.cpp
	dcclite/Guid.h
	dcclite/IFolderObject.cpp
	dcclite/IFolderObject.h
	dcclite/JsonUtils.cpp
	dcclite/JsonUtils.h
	dcclite/Log.h
	dcclite/Log.cpp	
	dcclite/NetMessenger.cpp
	dcclite/NetMessenger.h
	dcclite/Nmra.cpp
	dcclite/Nmra.h
	dcclite/Object.cpp
	dcclite/Object.h
	dcclite/RName.cpp
	dcclite/RName.h
	dcclite/Sha1.cpp
	dcclite/PathUtils.cpp
	dcclite/PathUtils.h	
	dcclite/SerialPort.h
	dcclite/Sha1.h
	dcclite/Socket.cpp
	dcclite/Socket.h
	dcclite/Util.cpp
	dcclite/Util.h
)

source_group("dcclite"          FILES ${SRC_DCCLITE})

if(WIN32)

	target_compile_definitions(Common PRIVATE _CRT_SECURE_NO_WARNINGS)
		
		target_sources(Common PRIVATE
			dcclite/Console_win.cpp
			dcclite/Guid_win.cpp
			dcclite/SerialPort_win.cpp
			dcclite/SerialPort_win.h
			dcclite/Sha1_win.cpp
		)

else(WIN32)

	target_sources(Common PRIVATE
		dcclite/Console_linux.cpp
		dcclite/Guid_linux.cpp
		dcclite/Sha1_linux.cpp
		dcclite/SerialPort_linux.cpp
		dcclite/SerialPort_linux.h
	)   

endif(WIN32)

target_link_libraries(Common PUBLIC CityHash SharedLib)

target_include_directories(Common
	PUBLIC  ${DCCLite_SOURCE_DIR}/contrib/cityhash/
	PUBLIC  ${DCCLite_SOURCE_DIR}/contrib/fmt/include
	PUBLIC  ${DCCLite_SOURCE_DIR}/contrib/JsonCreator/include
	PUBLIC  ${DCCLite_SOURCE_DIR}/contrib/rapidjson/include
			${DCCLite_SOURCE_DIR}/contrib/spdlog/include
	PUBLIC  ${DCCLite_SOURCE_DIR}/src/Embedded/lib/Shared
	PRIVATE ${DCCLite_SOURCE_DIR}/contrib/magic_enum/include	
)
