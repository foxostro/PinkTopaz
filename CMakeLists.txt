cmake_minimum_required (VERSION 2.8)
project (PinkTopaz)

set(SOURCE_FILES src/main.cpp)
add_executable(PinkTopaz ${SOURCE_FILES})

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
find_package(SDL2 REQUIRED)
message(STATUS SDL2_INCLUDE_DIR: ${SDL2_INCLUDE_DIR})
include_directories(${SDL2_INCLUDE_DIR})
target_link_libraries(PinkTopaz ${SDL2_LIBRARY})    
