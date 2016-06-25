cmake_minimum_required (VERSION 2.8)
project (PinkTopaz)
set(APP_NAME "PinkTopaz")

set(SOURCE_FILES src/main.cpp)
add_executable(${APP_NAME} MACOSX_BUNDLE ${SOURCE_FILES})

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
find_package(SDL2 REQUIRED)

string(REPLACE "-framework Cocoa" "" SDL2_LIBRARY ${SDL2_LIBRARY})
message(STATUS "SDL2_LIBRARY: \"${SDL2_LIBRARY}\"")

include_directories(${SDL2_INCLUDE_DIR})
target_link_libraries(PinkTopaz ${SDL2_LIBRARY})    

if(APPLE)
	#install(DIRECTORY ${SDL2_LIBRARY} DESTINATION ${APP_NAME}.app/Contents/Frameworks)
	add_custom_command(TARGET PinkTopaz POST_BUILD
	                   COMMAND ${CMAKE_COMMAND} -E copy_directory
	                   ${SDL2_LIBRARY} $<TARGET_FILE_DIR:PinkTopaz>/../Frameworks/SDL2.framework)
endif(APPLE)
