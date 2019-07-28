
set(Launcher_SRCS
	main.cpp
)

add_executable(Launcher  ${Launcher_SRCS})
target_compile_features(Launcher PUBLIC cxx_std_17)    

target_include_directories(Launcher 
  PRIVATE 
    ${DCCLite_SOURCE_DIR}/src/Common    
)


if(WIN32)
    target_link_libraries(Launcher 
		PRIVATE Common 		
		PRIVATE fmt
	)

	add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
              
else(WIN32)
	
	target_link_libraries(Launcher 		
		Common 		
		stdc++fs 
		fmt 		
	)

endif(WIN32)



