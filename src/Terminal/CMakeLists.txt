
set(Terminal_SRCS
	main.cpp
)

add_executable(Terminal  ${Terminal_SRCS})

target_include_directories(Terminal 
	PRIVATE 
    ${DCCLite_SOURCE_DIR}/src/Common    
)

target_link_libraries(
	Terminal 
	PRIVATE Common 
	PRIVATE fmt
	PRIVATE spdlog
	PRIVATE SharedLib
)


