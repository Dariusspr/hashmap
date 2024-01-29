add_library(hashmap hashmap.c hashmap.h)

target_include_directories(hashmap PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_features(hashmap PUBLIC c_std_99)
