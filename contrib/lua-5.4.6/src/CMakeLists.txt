
# Treat the C files as C++, so we have good stack unwinding: https://sol2.readthedocs.io/en/latest/errors.html?highlight=unwinding#destructors-and-safety
SET_SOURCE_FILES_PROPERTIES( *.c PROPERTIES LANGUAGE CXX )

add_library(
	LuaLib	   	 
	lapi.c        
	lapi.h        
	lauxlib.c     
	lauxlib.h     
	lbaselib.c    
	lcode.c       
	lcode.h       
	lcorolib.c    
	lctype.c      
	lctype.h      
	ldblib.c      
	ldebug.c      
	ldebug.h      
	ldo.c         
	ldo.h         
	ldump.c       
	lfunc.c       
	lfunc.h       
	lgc.c         
	lgc.h         
	linit.c       
	liolib.c      
	ljumptab.h    
	llex.c        
	llex.h        
	llimits.h     
	lmathlib.c    
	lmem.c        
	lmem.h        
	loadlib.c     
	lobject.c     
	lobject.h     
	lopcodes.c    
	lopcodes.h    
	lopnames.h    
	loslib.c      
	lparser.c     
	lparser.h     
	lprefix.h     
	lstate.c      
	lstate.h      
	lstring.c     
	lstring.h     
	lstrlib.c     
	ltable.c      
	ltable.h      
	ltablib.c     
	ltm.c         
	ltm.h         
	lua.h         
	lua.hpp               
	luaconf.h     
	lualib.h      
	lundump.c     
	lundump.h     
	lutf8lib.c    
	lvm.c         
	lvm.h         
	lzio.c        
	lzio.h    	
)

if(WIN32)

	target_link_libraries(LuaLib)
	add_compile_definitions(_CRT_SECURE_NO_WARNINGS)       
      
else(WIN32)

	target_link_libraries(LuaLib)

endif(WIN32)

target_compile_features(LuaLib PUBLIC cxx_std_17)



