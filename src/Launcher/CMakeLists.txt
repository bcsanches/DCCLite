
set(Launcher_SRCS
	main.cpp
)

add_executable(Launcher  ${Launcher_SRCS})

add_dependencies(Launcher Broker)

target_include_directories(Launcher 
  PRIVATE 
    ${DCCLite_SOURCE_DIR}/src/Common    
)


if(WIN32)
    target_link_libraries(Launcher 
		PRIVATE Common 		
		PRIVATE fmt
		PRIVATE spdlog
        PRIVATE SharedLib
	)

	add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
              
else(WIN32)
	
	target_link_libraries(Launcher 		
		Common 	
        SharedLib	
		stdc++fs 
		PRIVATE fmt
		PRIVATE spdlog	
	)

endif(WIN32)



