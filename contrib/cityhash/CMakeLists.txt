

add_library(
  CityHash
  config.h
  city.cc
  city.h
  citycrc.h
)

target_link_libraries(CityHash)    
