cmake_minimum_required (VERSION 2.8)
project (PinkTopaz)
set(APP_NAME "PinkTopaz")

set(CMAKE_CXX_FLAGS "-Wall -Wno-multichar -Werror -std=c++1z")

set(SOURCE_FILES_OPENGL
    "src/Shader.cpp" "src/Shader.hpp"
    "src/glUtilities.cpp" "src/glUtilities.hpp"
    "src/StaticMesh.cpp" "src/StaticMesh.hpp"
    "src/TextureArray.cpp" "src/TextureArray.hpp"
    "src/opengl.hpp"
  )

set(SOURCE_FILES_SYSTEMS
    "src/RenderSystem.cpp" "src/RenderSystem.hpp"
  )

set(SOURCE_FILES_COMPONENTS
    "src/ActiveCamera.hpp"
    "src/Transform.hpp"
    "src/RenderableStaticMesh.hpp"
  )

set(SOURCE_FILES_EVENTS
    "src/WindowSizeChangedEvent.hpp"
  )

set(SOURCE_FILES_OTHER_ECS
    "src/World.cpp" "src/World.hpp"
  )

set(SOURCE_FILES_MISC
    "src/main.cpp"
    "src/Application.cpp" "src/Application.hpp"
    "src/FileUtilities.cpp" "src/FileUtilities.hpp"
    "src/Exception.cpp" "src/Exception.hpp"
    "src/RetinaSupport.h"
  )

if(APPLE)
    set(SOURCE_FILES_PLATFORM_SUPPORT "src/RetinaSupport.h" "src/osx/RetinaSupport.m")
else()
    set(SOURCE_FILES_PLATFORM_SUPPORT "src/RetinaSupport.h" "src/linux/RetinaSupport.c")
endif(APPLE)

set(SOURCE_FILES
    ${SOURCE_FILES_OTHER_ECS}
    ${SOURCE_FILES_SYSTEMS}
    ${SOURCE_FILES_COMPONENTS}
    ${SOURCE_FILES_EVENTS}
    ${SOURCE_FILES_OPENGL}
    ${SOURCE_FILES_PLATFORM_SUPPORT}
    ${SOURCE_FILES_MISC}
    )

source_group("ECS\\Systems" FILES ${SOURCE_FILES_SYSTEMS})
source_group("ECS\\Components" FILES ${SOURCE_FILES_COMPONENTS})
source_group("ECS\\Events" FILES ${SOURCE_FILES_EVENTS})
source_group("ECS\\" FILES ${SOURCE_FILES_OTHER_ECS})
source_group("OpenGL Wrapper" FILES ${SOURCE_FILES_OPENGL})
source_group("Platform Support" FILES ${SOURCE_FILES_PLATFORM_SUPPORT})
source_group("Misc" FILES ${SOURCE_FILES_MISC})

add_executable(${APP_NAME} MACOSX_BUNDLE ${SOURCE_FILES})

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
find_package(OpenGL)
find_package(GLM)
find_package(SDL2_image REQUIRED)
find_package(SDL2 REQUIRED)

# EntityX ECS library.
find_package(PkgConfig)
pkg_check_modules(PC_LIBENTITYX QUIET entityx)
set(ENTITYX_CFLAGS ${PC_LIBENTITYX_CFLAGS_OTHER})

find_path(ENTITYX_INCLUDE_DIR entityx/entityx.h
          HINTS ${PC_LIBENTITYX_INCLUDEDIR} ${PC_LIBENTITYX_INCLUDE_DIRS}
          PATH_SUFFIXES )

find_library(ENTITYX_LIBRARY NAMES entityx
             HINTS ${PC_LIBENTITYX_LIBDIR} ${PC_LIBENTITYX_LIBRARY_DIRS} )

# Cmake configures a header file with some macros.
configure_file("${PROJECT_SOURCE_DIR}/src/config.h.in" "${PROJECT_BINARY_DIR}/config.h")

include_directories(${SDL2_INCLUDE_DIR}
                    ${SDL2_IMAGE_INCLUDE_DIR}
                    ${GLM_INCLUDE_DIR}
                    ${OPENGL_INCLUDE_DIRS}
                    ${PROJECT_BINARY_DIR}
                    ${ENTITYX_INCLUDE_DIR} )
target_link_libraries(PinkTopaz
                      ${SDL2_LIBRARY}
                      ${SDL2_IMAGE_LIBRARY}
                      ${OPENGL_LIBRARIES}
                      ${ENTITYX_LIBRARY} )

if(APPLE)
	# On Mac OS, we use a custom template for Info.plist so that we can specify the
	# NSHighResolutionCapable key and enable HiDPI support.
	set_target_properties(PinkTopaz PROPERTIES MACOSX_BUNDLE_INFO_PLIST src/MacOSXBundleInfo.plist.in)
	set(MACOSX_BUNDLE_GUI_IDENTIFIER "com.foxostro.pinktopaz")

	# On Mac OS, copy the SDL framework into the app bundle. Important for redistributing the game.
	add_custom_command(TARGET PinkTopaz POST_BUILD
	                   COMMAND ${CMAKE_COMMAND} -E copy_directory
	                   ${SDL2_LIBRARY} $<TARGET_FILE_DIR:PinkTopaz>/../Frameworks/SDL2.framework)

	# On Mac OS, copy the resources directory into the app bundle. Important for redistributing the game.
	add_custom_command(TARGET PinkTopaz POST_BUILD
	                   COMMAND ${CMAKE_COMMAND} -E copy_directory
	                   "./res" $<TARGET_FILE_DIR:PinkTopaz>/../Resources)
endif(APPLE)
