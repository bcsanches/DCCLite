
set(EmbeddedLib_SRCS
	Misc.cpp
  Parser.cpp
)

set(EmbeddedLib_HDRS
  BasicPin.h
	BitPack.h
  EmbeddedLibDefs.h
  Guid.h  
  Misc.h
  Packet.h
  Parser.h  
)

add_library(EmbeddedLib  ${EmbeddedLib_SRCS} ${EmbeddedLib_HDRS})
target_compile_features(EmbeddedLib PUBLIC cxx_std_17)

