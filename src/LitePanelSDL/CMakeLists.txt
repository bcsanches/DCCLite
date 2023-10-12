
add_executable(LitePanelSDL main.cpp)
target_compile_features(LitePanelSDL PUBLIC cxx_std_17)    

target_include_directories(LitePanelSDL 
	PRIVATE ${DCCLite_SOURCE_DIR}/contrib/fmt/include
	PRIVATE ${DCCLite_SOURCE_DIR}/contrib/SDL/include
	PRIVATE ${DCCLite_SOURCE_DIR}/contrib/spdlog/include
	PRIVATE ${DCCLite_SOURCE_DIR}/src/Common
)

if(WIN32)
    target_link_libraries(LitePanelSDL 
		PRIVATE Common	
		PRIVATE SDL3-shared
    )

    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
        
    set_property(TARGET LitePanelSDL PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
    set_property(TARGET LitePanelSDL PROPERTY VS_DEBUGGER_COMMAND_ARGUMENTS "..\\data\\EFMR")
      
else(WIN32)

	find_package(OpenSSL REQUIRED)

	target_link_libraries(LitePanelSDL 
		PRIVATE Common	
	)

endif(WIN32)

