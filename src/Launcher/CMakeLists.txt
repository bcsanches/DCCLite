
set(Launcher_SRCS
	main.cpp
)

add_executable(Launcher  ${Launcher_SRCS})
target_compile_features(Launcher PUBLIC cxx_std_17)    

target_include_directories(Launcher 
  PRIVATE 
    ${DCCLite_SOURCE_DIR}/src/Common    
)

target_link_libraries(Launcher PRIVATE Common fmt)


