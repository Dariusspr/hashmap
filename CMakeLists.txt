cmake_minimum_required(VERSION 3.28)

project(example)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)

add_executable(${PROJECT_NAME} hashmap_example.c)

add_subdirectory(hashmap)

target_link_libraries(${PROJECT_NAME} PRIVATE hashmap)
