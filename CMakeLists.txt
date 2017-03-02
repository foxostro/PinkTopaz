cmake_minimum_required (VERSION 3.6)
project (PinkTopaz)
set(APP_NAME "PinkTopaz")

set(CMAKE_CXX_FLAGS "-Wall -Wno-multichar -Werror -std=c++1z")

set(SOURCE_FILES_OPENGL
    "src/include/Renderer/OpenGL/opengl.hpp"
    "src/Renderer/OpenGL/glUtilities.cpp" "src/include/Renderer/OpenGL/glUtilities.hpp"
    "src/Renderer/OpenGL/GraphicsDeviceOpenGL.cpp" "src/include/Renderer/OpenGL/GraphicsDeviceOpenGL.hpp"
    "src/Renderer/OpenGL/CommandQueue.cpp" "src/include/Renderer/OpenGL/CommandQueue.hpp"
    "src/Renderer/OpenGL/CommandEncoderOpenGL.cpp" "src/include/Renderer/OpenGL/CommandEncoderOpenGL.hpp"
    "src/Renderer/OpenGL/BufferOpenGL.cpp" "src/include/Renderer/OpenGL/BufferOpenGL.hpp"
    "src/include/Renderer/OpenGL/ShaderOpenGL.hpp" "src/Renderer/OpenGL/ShaderOpenGL.cpp"
    "src/Renderer/OpenGL/TextureOpenGL.cpp" "src/include/Renderer/OpenGL/TextureOpenGL.hpp"
    "src/Renderer/OpenGL/TextureSamplerOpenGL.cpp" "src/include/Renderer/OpenGL/TextureSamplerOpenGL.hpp"
    )

set(SOURCE_FILES_RENDERER
    "src/Renderer/StringRenderer.cpp" "src/include/Renderer/StringRenderer.hpp"
    "src/Renderer/String.cpp" "src/include/Renderer/String.hpp"
    "src/Renderer/StaticMesh.cpp" "src/include/Renderer/StaticMesh.hpp"
    "src/include/Renderer/GraphicsDevice.hpp"
    "src/include/Renderer/CommandEncoder.hpp"
    "src/include/Renderer/Shader.hpp"
    "src/include/Renderer/VertexFormat.hpp"
    "src/include/Renderer/Buffer.hpp"
    "src/include/Renderer/RenderPassDescriptor.hpp"
    "src/include/Renderer/Texture.hpp"
    "src/include/Renderer/TextureSampler.hpp"
    )

set(SOURCE_FILES_SYSTEMS
    "src/RenderSystem.cpp" "src/include/RenderSystem.hpp"
    )

set(SOURCE_FILES_COMPONENTS
    "src/include/ActiveCamera.hpp"
    "src/include/Transform.hpp"
    "src/include/RenderableStaticMesh.hpp"
    )

set(SOURCE_FILES_EVENTS
    "src/include/WindowSizeChangedEvent.hpp"
    )

set(SOURCE_FILES_OTHER_ECS
    "src/World.cpp" "src/include/World.hpp"
    )

set(SOURCE_FILES_MISC
    "src/main.cpp"
    "src/Application.cpp" "src/include/Application.hpp"
    "src/FileUtilities.cpp" "src/include/FileUtilities.hpp"
    "src/Exception.cpp" "src/include/Exception.hpp"
    "src/include/RetinaSupport.h"
    )

set(SOURCE_FILES_TEST
    "src/test/test.cpp"
    )

if(APPLE)
    set(SOURCE_FILES_PLATFORM_SUPPORT "src/include/RetinaSupport.h" "src/osx/RetinaSupport.m")
else()
    set(SOURCE_FILES_PLATFORM_SUPPORT "src/include/RetinaSupport.h" "src/linux/RetinaSupport.c")
endif(APPLE)

set(SOURCE_FILES
    ${SOURCE_FILES_OTHER_ECS}
    ${SOURCE_FILES_RENDERER}
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
source_group("Renderer" FILES ${SOURCE_FILES_RENDERER})
source_group("Renderer\\OpenGL" FILES ${SOURCE_FILES_OPENGL})
source_group("Platform Support" FILES ${SOURCE_FILES_PLATFORM_SUPPORT})
source_group("Misc" FILES ${SOURCE_FILES_MISC})

add_executable(${APP_NAME} MACOSX_BUNDLE ${SOURCE_FILES})

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
find_package(OpenGL REQUIRED)
find_package(GLM REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2 REQUIRED)
find_package(Freetype REQUIRED)

# EntityX ECS library.
find_package(PkgConfig REQUIRED)
pkg_check_modules(PC_LIBENTITYX QUIET entityx)
set(ENTITYX_CFLAGS ${PC_LIBENTITYX_CFLAGS_OTHER})

find_path(ENTITYX_INCLUDE_DIR entityx/entityx.h
          HINTS ${PC_LIBENTITYX_INCLUDEDIR} ${PC_LIBENTITYX_INCLUDE_DIRS}
          PATH_SUFFIXES
          )

find_library(ENTITYX_LIBRARY NAMES entityx
             HINTS ${PC_LIBENTITYX_LIBDIR} ${PC_LIBENTITYX_LIBRARY_DIRS}
             )

# Cmake configures a header file with some macros.
configure_file("${PROJECT_SOURCE_DIR}/src/config.h.in" "${PROJECT_BINARY_DIR}/config.h")

include_directories("${PROJECT_SOURCE_DIR}/src/include"
                    ${SDL2_INCLUDE_DIR}
                    ${SDL2_IMAGE_INCLUDE_DIR}
                    ${FREETYPE_INCLUDE_DIR_ft2build}
                    ${GLM_INCLUDE_DIR}
                    ${OPENGL_INCLUDE_DIRS}
                    ${PROJECT_BINARY_DIR}
                    ${ENTITYX_INCLUDE_DIR}
                    )
target_link_libraries(PinkTopaz
                      ${SDL2_LIBRARY}
                      ${SDL2_IMAGE_LIBRARY}
                      ${FREETYPE_LIBRARY}
                      ${OPENGL_LIBRARIES}
                      ${ENTITYX_LIBRARY}
                      )

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

# Set up unit test support.
enable_testing()
set(PROJECT_TEST_NAME ${APP_NAME}Test)
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})
add_executable(${PROJECT_TEST_NAME} ${SOURCE_FILES_TEST})
target_link_libraries(${PROJECT_TEST_NAME} ${GTEST_BOTH_LIBRARIES})
add_test(${PROJECT_TEST_NAME} ${PROJECT_TEST_NAME})
