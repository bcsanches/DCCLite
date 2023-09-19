

add_library(
  CityHash
  config.h
  city.cc
  city.h
  citycrc.h
)

target_link_library(City Hash) 

target_compile_features(CityHash PUBLIC cxx_std_17)    
