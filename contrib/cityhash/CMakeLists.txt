

add_library(
  CityHash
  config.h
  city.cc
  city.h
  citycrc.h
)

target_compile_features(CityHash PUBLIC cxx_std_17)    
