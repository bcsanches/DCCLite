
set(Terminal_SRCS
	main.cpp
)

add_executable(Terminal  ${Terminal_SRCS})
target_compile_features(Terminal PUBLIC cxx_std_17)    

target_include_directories(Terminal 
  PRIVATE 
    ${DCCLite_SOURCE_DIR}/src/Common    
)

target_link_libraries(Terminal PRIVATE Common fmt)


