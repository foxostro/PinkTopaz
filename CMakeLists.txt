cmake_minimum_required (VERSION 2.8)
project (PinkTopaz)
set(APP_NAME "PinkTopaz")

set(CMAKE_CXX_FLAGS "-Wall -Werror -std=c++1z")

# Define the target and specify source files for it. This must be done before the target can be referenced.
set(SOURCE_FILES "src/main.cpp" "src/Renderer.cpp" "src/Renderer.hpp")
add_executable(${APP_NAME} MACOSX_BUNDLE ${SOURCE_FILES})

# Have Cmake find the SDL framework.
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
find_package(SDL2 REQUIRED)
string(REPLACE "-framework Cocoa" "" SDL2_LIBRARY ${SDL2_LIBRARY})

# And OpenGL, we need that too.
find_package(OpenGL)

# Cmake configures a header file with some macros.
configure_file("${PROJECT_SOURCE_DIR}/src/config.h.in" "${PROJECT_BINARY_DIR}/config.h")

# Now link them all together.
include_directories(${SDL2_INCLUDE_DIR} ${OPENGL_INCLUDE_DIRS} ${PROJECT_BINARY_DIR})
target_link_libraries(PinkTopaz ${SDL2_LIBRARY} ${OPENGL_LIBRARIES})    

if(APPLE)
	# On Mac OS, we use a custom template for Info.plist so that we can specify the
	# NSHighResolutionCapable key and enable HiDPI support.
	set_target_properties(PinkTopaz PROPERTIES MACOSX_BUNDLE_INFO_PLIST src/MacOSXBundleInfo.plist.in)
	set(MACOSX_BUNDLE_GUI_IDENTIFIER "com.foxostro.pinktopaz")

	# On Mac OS, copy the SDL framework into the app bundle. Important for redistributing the game.
	add_custom_command(TARGET PinkTopaz POST_BUILD
	                   COMMAND ${CMAKE_COMMAND} -E copy_directory
	                   ${SDL2_LIBRARY} $<TARGET_FILE_DIR:PinkTopaz>/../Frameworks/SDL2.framework)
endif(APPLE)
