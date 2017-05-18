cmake_minimum_required (VERSION 3.5)
project (PinkTopaz)
set(APP_NAME "PinkTopaz")

include(conanbuildinfo.cmake)
conan_basic_setup()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    set(CMAKE_CXX_FLAGS "/EHsc /D_ALLOW_ITERATOR_DEBUG_LEVEL_MISMATCH")

    set(CMAKE_CXX_FLAGS_RELEASE "/MD")
    set(CMAKE_CXX_FLAGS_DEBUG "/MD /Zi")
else()
    if(APPLE)
    set(APPLE_FLAGS "-DMETAL_ENABLED")
    else(APPLE)
    set(APPLE_FLAGS "")
    endif(APPLE)
    
    set(CMAKE_CXX_FLAGS "-Wall -Wno-multichar -Werror -std=c++1z ${APPLE_FLAGS}")
endif()

set(SOURCE_FILES_TERRAIN
    "src/include/Terrain/Array3D.hpp"
    "src/include/Terrain/Voxel.hpp"
    "src/Terrain/Terrain.cpp" "src/include/Terrain/Terrain.hpp"
    "src/Terrain/VoxelData.cpp" "src/include/Terrain/VoxelData.hpp"
    "src/Terrain/VoxelDataStore.cpp" "src/include/Terrain/VoxelDataStore.hpp"
    "src/Terrain/ChangeLog.cpp" "src/include/Terrain/ChangeLog.hpp"
    "src/Terrain/VoxelDataLoader.cpp" "src/include/Terrain/VoxelDataLoader.hpp"
    "src/Terrain/MesherMarchingCubes.cpp" "src/include/Terrain/MesherMarchingCubes.hpp"
    "src/include/Terrain/Mesher.hpp"
    )

set(SOURCE_FILES_OPENGL
    "src/include/Renderer/OpenGL/opengl.hpp"
    "src/Renderer/OpenGL/glUtilities.cpp" "src/include/Renderer/OpenGL/glUtilities.hpp"
    "src/Renderer/OpenGL/GraphicsDeviceOpenGL.cpp" "src/include/Renderer/OpenGL/GraphicsDeviceOpenGL.hpp"
    "src/Renderer/OpenGL/CommandEncoderOpenGL.cpp" "src/include/Renderer/OpenGL/CommandEncoderOpenGL.hpp"
    "src/Renderer/OpenGL/BufferOpenGL.cpp" "src/include/Renderer/OpenGL/BufferOpenGL.hpp"
    "src/Renderer/OpenGL/ShaderOpenGL.cpp" "src/include/Renderer/OpenGL/ShaderOpenGL.hpp"
    "src/Renderer/OpenGL/TextureOpenGL.cpp" "src/include/Renderer/OpenGL/TextureOpenGL.hpp"
    "src/Renderer/OpenGL/TextureSamplerOpenGL.cpp" "src/include/Renderer/OpenGL/TextureSamplerOpenGL.hpp"
    )

if(APPLE)
    set(SOURCE_FILES_METAL
        "src/Renderer/Metal/GraphicsDeviceMetal.mm" "src/include/Renderer/Metal/GraphicsDeviceMetal.h"
        "src/Renderer/Metal/CommandEncoderMetal.mm" "src/include/Renderer/Metal/CommandEncoderMetal.h"
        "src/Renderer/Metal/ShaderMetal.mm" "src/include/Renderer/Metal/ShaderMetal.h"
        "src/Renderer/Metal/TextureMetal.mm" "src/include/Renderer/Metal/TextureMetal.h"
        "src/Renderer/Metal/TextureSamplerMetal.mm" "src/include/Renderer/Metal/TextureSamplerMetal.h"
        "src/Renderer/Metal/BufferMetal.mm" "src/include/Renderer/Metal/BufferMetal.h"
        "src/Library.metal"
        )
else(APPLE)
    set(SOURCE_FILES_METAL "")
endif(APPLE)

set(SOURCE_FILES_RENDERER
    "src/Renderer/StringRenderer.cpp" "src/include/Renderer/StringRenderer.hpp"
    "src/Renderer/StaticMesh.cpp" "src/include/Renderer/StaticMesh.hpp"
    "src/Renderer/StaticMeshLoader.cpp" "src/include/Renderer/StaticMeshLoader.hpp"
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
    "src/CameraMovementSystem.cpp" "src/include/CameraMovementSystem.hpp"
    )

set(SOURCE_FILES_COMPONENTS
    "src/include/ActiveCamera.hpp"
    "src/include/Transform.hpp"
    "src/include/RenderableStaticMesh.hpp"
    "src/include/TerrainComponent.hpp"
    )

set(SOURCE_FILES_EVENTS
    "src/include/WindowSizeChangedEvent.hpp"
    "src/include/KeypressEvent.hpp"
    "src/include/MouseMoveEvent.hpp"
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
    "src/FrameTimer.cpp" "src/include/FrameTimer.hpp"
    "src/include/AABB.hpp"
    "src/include/math.hpp"
    )

if(APPLE)
    set(SOURCE_FILES_PLATFORM_SUPPORT
        "src/include/RetinaSupport.h" "src/osx/RetinaSupport.m"
        "src/osx/GraphicsDeviceFactory.mm"
        )
else(APPLE)
    set(SOURCE_FILES_PLATFORM_SUPPORT
        "src/include/RetinaSupport.h" "src/linux/RetinaSupport.c"
        "src/linux/GraphicsDeviceFactory.cpp"
        )
endif(APPLE)

set(SOURCE_FILES
    ${SOURCE_FILES_TERRAIN}
    ${SOURCE_FILES_OTHER_ECS}
    ${SOURCE_FILES_RENDERER}
    ${SOURCE_FILES_SYSTEMS}
    ${SOURCE_FILES_COMPONENTS}
    ${SOURCE_FILES_EVENTS}
    ${SOURCE_FILES_OPENGL}
    ${SOURCE_FILES_METAL}
    ${SOURCE_FILES_PLATFORM_SUPPORT}
    ${SOURCE_FILES_MISC}
    )

source_group("Terrain" FILES ${SOURCE_FILES_TERRAIN})
source_group("ECS\\Systems" FILES ${SOURCE_FILES_SYSTEMS})
source_group("ECS\\Components" FILES ${SOURCE_FILES_COMPONENTS})
source_group("ECS\\Events" FILES ${SOURCE_FILES_EVENTS})
source_group("ECS\\" FILES ${SOURCE_FILES_OTHER_ECS})
source_group("Renderer" FILES ${SOURCE_FILES_RENDERER})
source_group("Renderer\\OpenGL" FILES ${SOURCE_FILES_OPENGL})
source_group("Renderer\\Metal" FILES ${SOURCE_FILES_METAL})
source_group("Platform Support" FILES ${SOURCE_FILES_PLATFORM_SUPPORT})
source_group("Misc" FILES ${SOURCE_FILES_MISC})

# Build the app. If we're on Apple then this is a .app bundle.
if(APPLE)
add_executable(${APP_NAME} MACOSX_BUNDLE ${SOURCE_FILES})
else(APPLE)
add_executable(${APP_NAME} ${SOURCE_FILES})
endif(APPLE)

find_package(OpenGL REQUIRED)

# Apple's "Metal" graphics library.
if(APPLE)
    find_library(METAL_LIBRARY Metal)
    FIND_PATH(METAL_INCLUDE_DIR Metal/Metal.h)

    find_library(QUARTZ_CORE_LIBRARY QuartzCore)
    FIND_PATH(QUARTZ_CORE_INCLUDE_DIR QuartzCore/QuartzCore.h)

    set(MACOS_LIBRARIES
        ${METAL_LIBRARY}
        ${QUARTZ_CORE_LIBRARY}
        )
        
    set(MACOS_INCLUDE_DIRS
        ${METAL_INCLUDE_DIR}
        ${QUARTZ_CORE_INCLUDE_DIR}
        )
else()
    set(MACOS_LIBRARIES "")
    set(MACOS_INCLUDE_DIRS "")
endif(APPLE)

# Cmake configures a header file with some macros.
configure_file("${PROJECT_SOURCE_DIR}/src/pinktopaz_config.h.in"
               "${PROJECT_SOURCE_DIR}/src/pinktopaz_config.h")

include_directories("${PROJECT_SOURCE_DIR}/src/include"
                    ${OPENGL_INCLUDE_DIRS}
                    ${MACOS_INCLUDE_DIRS}
                    )
target_link_libraries(${APP_NAME}
                      ${CONAN_LIBS}
                      ${OPENGL_LIBRARIES}
                      ${MACOS_LIBRARIES}
                      )

if(APPLE)
	# On Mac OS, we use a custom template for Info.plist so that we can specify the
	# NSHighResolutionCapable key and enable HiDPI support.
	set_target_properties(PinkTopaz PROPERTIES MACOSX_BUNDLE_INFO_PLIST src/MacOSXBundleInfo.plist.in)
	set(MACOSX_BUNDLE_GUI_IDENTIFIER "com.foxostro.pinktopaz")

    add_custom_command(TARGET ${APP_NAME} POST_BUILD
                       COMMAND xcrun -sdk macosx metal src/Library.metal -gline-tables-only -o /tmp/${APP_NAME}.Library.air
                       COMMAND xcrun -sdk macosx metallib /tmp/${APP_NAME}.Library.air -o res/Library.metallib
                       )

	# On Mac OS, copy the resources directory into the app bundle. Important for redistributing the game.
	add_custom_command(TARGET ${APP_NAME} POST_BUILD
	                   COMMAND ${CMAKE_COMMAND} -E copy_directory "./res" $<TARGET_FILE_DIR:${APP_NAME}>/../Resources
                       )

    # On Mac OS, sign the app bundle.
    add_custom_command(TARGET ${APP_NAME} POST_BUILD
                       COMMAND ./sign_the_app_bundle $<TARGET_FILE_DIR:${APP_NAME}>/../..
                       )
endif(APPLE)

if(WIN32)
    # On Windows, copy the resources to the same directory as the executable.
    add_custom_command(TARGET ${APP_NAME} POST_BUILD
                       COMMAND ${CMAKE_COMMAND} -E copy_directory "./res" $<TARGET_FILE_DIR:${APP_NAME}>/
                       )
endif(WIN32)
