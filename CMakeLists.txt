cmake_minimum_required (VERSION 3.9)
project (PinkTopaz)
set(APP_NAME "PinkTopaz")

include(conanbuildinfo.cmake)
conan_basic_setup()

if(APPLE)
    option(USE_METAL "Use the Metal graphics API on macOS." TRUE)
endif()

option(FORCE_REBUILD_FONT_TEXTURE_ATLAS "Always regenerate font texture atlases and do not cache them on disk." FALSE)

set(CMAKE_CXX_STANDARD 17)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    set(CMAKE_CXX_FLAGS "/arch:AVX2 /EHsc /D__BMI2__ /D__LZCNT__ /D__POPCNT__ /D_ALLOW_ITERATOR_DEBUG_LEVEL_MISMATCH")

    if(MSVC_VERSION LESS 1900)
        message(WARNING "This probably won't build with Visual Studio older than MS Visual Studio 2015.")
    elseif(MSVC_VERSION EQUAL 1900)
        # Visual Studio 2015 does not support /std:c++17 so we have to specify /std:c++latest here.
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++latest")
    endif()

    set(CMAKE_CXX_FLAGS_RELEASE "/MD")
    set(CMAKE_CXX_FLAGS_DEBUG "/MD /Zi")

    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT "${APP_NAME}")
elseif(APPLE)
    if(USE_METAL)
        set(APPLE_FLAGS "-DMETAL_ENABLED ${APPLE_FLAGS}")
    else(USE_METAL)
        set(APPLE_FLAGS "")
    endif(USE_METAL)
    
    set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG -g -Oz")
    set(CMAKE_CXX_FLAGS_DEBUG "-g -Og")
    set(CMAKE_CXX_FLAGS "-mbmi2 -mlzcnt -mavx2 -Wall -Wno-multichar -Werror ${APPLE_FLAGS}")
else()
    set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG -g -Oz")
    set(CMAKE_CXX_FLAGS_DEBUG "-g -Og")
    set(CMAKE_CXX_FLAGS "-mbmi2 -mlzcnt -mavx2 -Wall -Wno-multichar -Werror")
endif()

if(FORCE_REBUILD_FONT_TEXTURE_ATLAS)
    set(CMAKE_CXX_FLAGS "-DFORCE_REBUILD_FONT_TEXTURE_ATLAS ${CMAKE_CXX_FLAGS}")
endif()

set(SOURCE_FILES_GRID
    "src/include/Grid/GridIndexer.hpp"
    "src/include/Grid/Array3D.hpp"
    "src/include/Grid/GridLRU.hpp"
    "src/include/Grid/SparseGrid.hpp"
    "src/include/Grid/RegionMutualExclusionArbitrator.hpp"
    "src/include/Grid/FrustumRange.hpp"
    "src/include/Grid/GridIndexerRange.hpp"
    "src/include/Grid/GridPoints.hpp"
    "src/include/Grid/GridRaycast.hpp"
    )

set(SOURCE_FILES_TERRAIN
    "src/include/Terrain/Voxel.hpp"
    "src/include/Terrain/TerrainConfig.hpp"
    "src/include/Terrain/Mesher.hpp"
    "src/include/Terrain/TerrainMeshGrid.hpp"
    "src/include/Terrain/TerrainHorizonDistance.hpp"
    "src/Terrain/MesherMarchingCubes.cpp" "src/include/Terrain/MesherMarchingCubes.hpp"
    "src/Terrain/MesherNaiveSurfaceNets.cpp" "src/include/Terrain/MesherNaiveSurfaceNets.hpp"
    "src/Terrain/VoxelData.cpp" "src/include/Terrain/VoxelData.hpp"
    "src/Terrain/TransactedVoxelData.cpp" "src/include/Terrain/TransactedVoxelData.hpp"
    "src/Terrain/VoxelDataGenerator.cpp" "src/include/Terrain/VoxelDataGenerator.hpp"
    "src/Terrain/Terrain.cpp" "src/include/Terrain/Terrain.hpp"
    "src/Terrain/TerrainMesh.cpp" "src/include/Terrain/TerrainMesh.hpp"
    "src/Terrain/TerrainProgressTracker.cpp" "src/include/Terrain/TerrainProgressTracker.hpp"
    "src/Terrain/VoxelDataSerializer.cpp" "src/include/Terrain/VoxelDataSerializer.hpp"
    "src/Terrain/MapRegionStore.cpp" "src/include/Terrain/MapRegionStore.hpp"
    "src/Terrain/MapRegion.cpp" "src/include/Terrain/MapRegion.hpp"
    "src/Terrain/TerrainRebuildActor.cpp" "src/include/Terrain/TerrainRebuildActor.hpp"
    "src/include/Terrain/TerrainOperation.hpp"
    "src/Terrain/TerrainOperationEditPoint.cpp" "src/include/Terrain/TerrainOperationEditPoint.hpp"
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
    "src/Renderer/OpenGL/CommandQueue.cpp" "src/include/Renderer/OpenGL/CommandQueue.hpp"
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
    "src/Renderer/TextRenderer.cpp" "src/include/Renderer/TextRenderer.hpp"
    "src/Renderer/StaticMesh.cpp" "src/include/Renderer/StaticMesh.hpp"
    "src/Renderer/StaticMeshSerializer.cpp" "src/include/Renderer/StaticMeshSerializer.hpp"
    "src/include/Renderer/GraphicsDevice.hpp"
    "src/include/Renderer/CommandEncoder.hpp"
    "src/include/Renderer/Shader.hpp"
    "src/include/Renderer/VertexFormat.hpp"
    "src/include/Renderer/Buffer.hpp"
    "src/include/Renderer/RenderPassDescriptor.hpp"
    "src/include/Renderer/Texture.hpp"
    "src/include/Renderer/TextureSampler.hpp"
    "src/Renderer/TextureArrayLoader.cpp" "src/include/Renderer/TextureArrayLoader.hpp"
    "src/include/Renderer/TerrainVertex.hpp"
    "src/Renderer/WireframeCube.cpp" "src/include/WireframeCube.hpp"
    )

set(SOURCE_FILES_FONTS
    "src/include/Fonts/TextAttributes.hpp"
    "src/Fonts/Glyph.cpp" "src/include/Fonts/Glyph.hpp"
    "src/include/Fonts/GlyphRenderer.hpp"
    "src/Fonts/GlyphRendererOutline.cpp" "src/include/Fonts/GlyphRendererOutline.hpp"
    "src/Fonts/GlyphRendererRegular.cpp" "src/include/Fonts/GlyphRendererRegular.hpp"
    "src/include/Fonts/PackedGlyph.hpp"
    "src/Fonts/FontTextureAtlasPacker.cpp" "src/include/Fonts/FontTextureAtlasPacker.hpp"
    "src/Fonts/FontTextureAtlasBuilder.cpp" "src/include/Fonts/FontTextureAtlasBuilder.hpp"
    "src/Fonts/FontTextureAtlas.cpp" "src/include/Fonts/FontTextureAtlas.hpp"
    )

set(SOURCE_FILES_SYSTEMS
    "src/RenderSystem.cpp" "src/include/RenderSystem.hpp"
    "src/CameraMovementSystem.cpp" "src/include/CameraMovementSystem.hpp"
    "src/TerrainCursorSystem.cpp" "src/include/TerrainCursorSystem.hpp"
    "src/TerrainProgressSystem.cpp" "src/include/TerrainProgressSystem.hpp"
    "src/TerrainEditSystem.cpp" "src/include/TerrainEditSystem.hpp"
    )

set(SOURCE_FILES_COMPONENTS
    "src/include/ActiveCamera.hpp"
    "src/include/Transform.hpp"
    "src/include/RenderableStaticMesh.hpp"
    "src/include/TerrainComponent.hpp"
    "src/include/TerrainCursor.hpp"
    )

set(SOURCE_FILES_EVENTS
    "src/include/WindowSizeChangedEvent.hpp"
    "src/include/KeypressEvent.hpp"
    "src/include/MouseButtonEvent.hpp"
    "src/include/MouseMoveEvent.hpp"
    "src/include/CameraMovedEvent.hpp"
    "src/include/TerrainCursorInvalidatedEvent.hpp"
    "src/include/TerrainProgressEvent.hpp"
    )

set(SOURCE_FILES_OTHER_ECS
    "src/World.cpp" "src/include/World.hpp"
    )

set(SOURCE_FILES_MISC
    "src/include/UniqueName.hpp"
    "src/include/CerealGLM.hpp"
    "src/include/Exception.hpp" "src/Exception.cpp" 
    "src/include/FileUtilities.hpp" "src/FileUtilities.cpp"
    "src/include/FrameTimer.hpp" "src/FrameTimer.cpp"
    "src/include/TaskDispatcher.hpp" "src/TaskDispatcher.cpp"
    "src/include/MemoryMappedFile.hpp" "src/MemoryMappedFile.cpp"
    )

set(SOURCE_FILES_NOISE
    "src/include/Noise/Noise.hpp"
    "src/include/Noise/SimplexNoise.hpp" "src/Noise/SimplexNoise.cpp"
    )

set(SOURCE_FILES_MATH
    "src/include/AABB.hpp"
    "src/include/Plane.hpp"
    "src/include/Frustum.hpp"
    "src/include/Ray.hpp"
    "src/include/math.hpp"
    "src/include/Morton.hpp"
    )

set(SOURCE_FILES_APPLICATION
    "src/main.cpp"
    "src/Application.cpp" "src/include/Application.hpp"
    )

set(SOURCE_FILES_PLATFORM_SUPPORT
    "src/include/RetinaSupport.h"
    "src/include/VideoRefreshRate.hpp"
    "src/include/Profiler.hpp"
    "src/include/RetinaSupport.h"
    "src/include/RetinaSupport.h"
    "src/include/ThreadName.hpp"
    "src/include/AutoreleasePool.hpp"
    )

set(SOURCE_FILES_BLOCK_DATA_STORE
    "src/BlockDataStore/MallocZone.cpp" "src/include/BlockDataStore/MallocZone.hpp"
    "src/BlockDataStore/BoxedMallocZone.cpp" "src/include/BlockDataStore/BoxedMallocZone.hpp"
    "src/BlockDataStore/ManagedMallocZone.cpp" "src/include/BlockDataStore/ManagedMallocZone.hpp"
    "src/BlockDataStore/BlockDataStore.cpp" "src/include/BlockDataStore/BlockDataStore.hpp"
    )

if(APPLE)
    list(APPEND SOURCE_FILES_PLATFORM_SUPPORT
        "src/osx/RetinaSupport.m"
        "src/osx/GraphicsDeviceFactory.mm"
        "src/osx/VideoRefreshRate.cpp"
        "src/osx/Profiler.cpp"
        "src/osx/ThreadName.cpp"
        "src/osx/AutoreleasePool.mm"
        )
else(APPLE)
	if(WIN32)
        list(APPEND SOURCE_FILES_PLATFORM_SUPPORT
            "src/windows/RetinaSupport.c"
            "src/windows/GraphicsDeviceFactory.cpp"
            "src/windows/VideoRefreshRate.cpp"
            "src/windows/Profiler.cpp"
            "src/windows/ThreadName.cpp"
            "src/windows/AutoreleasePool.cpp"
            )
	else(WIN32)
        list(APPEND SOURCE_FILES_PLATFORM_SUPPORT
            "src/linux/RetinaSupport.c"
            "src/linux/GraphicsDeviceFactory.cpp"
            "src/linux/VideoRefreshRate.cpp"
            "src/linux/Profiler.cpp"
            "src/linux/ThreadName.cpp"
            "src/linux/AutoreleasePool.cpp"
            )
	endif(WIN32)
endif(APPLE)

set(SOURCE_FILES
    ${SOURCE_FILES_GRID}
    ${SOURCE_FILES_TERRAIN}
    ${SOURCE_FILES_OTHER_ECS}
    ${SOURCE_FILES_RENDERER}
    ${SOURCE_FILES_FONTS}
    ${SOURCE_FILES_SYSTEMS}
    ${SOURCE_FILES_COMPONENTS}
    ${SOURCE_FILES_EVENTS}
    ${SOURCE_FILES_OPENGL}
    ${SOURCE_FILES_METAL}
    ${SOURCE_FILES_PLATFORM_SUPPORT}
    ${SOURCE_FILES_MISC}
    ${SOURCE_FILES_NOISE}
    ${SOURCE_FILES_MATH}
    ${SOURCE_FILES_APPLICATION}
    ${SOURCE_FILES_BLOCK_DATA_STORE}
    )

source_group("Grid" FILES ${SOURCE_FILES_GRID})
source_group("Terrain" FILES ${SOURCE_FILES_TERRAIN})
source_group("ECS\\Systems" FILES ${SOURCE_FILES_SYSTEMS})
source_group("ECS\\Components" FILES ${SOURCE_FILES_COMPONENTS})
source_group("ECS\\Events" FILES ${SOURCE_FILES_EVENTS})
source_group("ECS\\" FILES ${SOURCE_FILES_OTHER_ECS})
source_group("Fonts" FILES ${SOURCE_FILES_FONTS})
source_group("Renderer" FILES ${SOURCE_FILES_RENDERER})
source_group("Renderer\\OpenGL" FILES ${SOURCE_FILES_OPENGL})
source_group("Renderer\\Metal" FILES ${SOURCE_FILES_METAL})
source_group("Platform Support" FILES ${SOURCE_FILES_PLATFORM_SUPPORT})
source_group("Misc" FILES ${SOURCE_FILES_MISC})
source_group("Noise" FILES ${SOURCE_FILES_NOISE})
source_group("Math" FILES ${SOURCE_FILES_MATH})
source_group("Application" FILES ${SOURCE_FILES_APPLICATION})
source_group("Block Data Store" FILES ${SOURCE_FILES_BLOCK_DATA_STORE})

# Build the app.
if(APPLE)
    add_executable(${APP_NAME} MACOSX_BUNDLE ${SOURCE_FILES})
elseif(WIN32)
    add_executable(${APP_NAME} WIN32 ${SOURCE_FILES})
else()
    add_executable(${APP_NAME} ${SOURCE_FILES})
endif()

find_package(OpenGL REQUIRED)

# Apple's "Metal" graphics library.
if(APPLE)
    find_library(METAL_LIBRARY Metal)
    find_path(METAL_INCLUDE_DIR Metal/Metal.h)

    find_library(QUARTZ_CORE_LIBRARY QuartzCore)
    find_path(QUARTZ_CORE_INCLUDE_DIR QuartzCore/QuartzCore.h)

    set(ADDITIONAL_LIBRARIES
        ${METAL_LIBRARY}
        ${QUARTZ_CORE_LIBRARY}
        )
        
    set(ADDITIONAL_INCLUDE_DIRS
        ${METAL_INCLUDE_DIR}
        ${QUARTZ_CORE_INCLUDE_DIR}
        )
else(APPLE)
    if (WIN32)
        set(ADDITIONAL_LIBRARIES "")
        set(ADDITIONAL_INCLUDE_DIRS "")
    else(WIN32)
        set(ADDITIONAL_LIBRARIES "atomic")
        set(ADDITIONAL_INCLUDE_DIRS "")
    endif(WIN32)
endif(APPLE)

# Cmake configures a header file with some macros.
configure_file("${PROJECT_SOURCE_DIR}/src/pinktopaz_config.h.in"
               "${PROJECT_SOURCE_DIR}/src/pinktopaz_config.h")

include_directories("${PROJECT_SOURCE_DIR}/src/include"
                    ${OPENGL_INCLUDE_DIRS}
                    ${ADDITIONAL_INCLUDE_DIRS}
                    )
target_link_libraries(${APP_NAME}
                      ${CONAN_LIBS}
                      ${OPENGL_LIBRARIES}
                      ${ADDITIONAL_LIBRARIES}
                      )

if(APPLE)
	# On Mac OS, we use a custom template for Info.plist so that we can specify the
	# NSHighResolutionCapable key and enable HiDPI support.
	set_target_properties(${APP_NAME} PROPERTIES MACOSX_BUNDLE_INFO_PLIST src/MacOSXBundleInfo.plist.in)
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
else(APPLE)
    # On Windows, copy the resources to the same directory as the executable.
    add_custom_command(TARGET ${APP_NAME} POST_BUILD
                       COMMAND ${CMAKE_COMMAND} -E copy_directory "./res" $<TARGET_FILE_DIR:${APP_NAME}>/
                       )
endif(APPLE)


# Build a benchmark program to measure the time to generate simplex noise.
add_executable("SimplexNoiseBenchmarks"
               "src/benchmarks/Noise/SimplexNoiseBenchmarks.cpp"               
               "src/Noise/SimplexNoise.cpp"
               )
target_link_libraries("SimplexNoiseBenchmarks"
                      ${CONAN_LIBS}
                      )


# Set up unit test support with the Catch unit test framework.
enable_testing()
set(TEST_PROG_NAME "${APP_NAME}Tests")
add_executable(${TEST_PROG_NAME}
               "src/test/PinkTopazTestsMain.cpp"
               "src/test/FrustumTests.cpp"
               "src/test/MortonTests.cpp"
               "src/test/Grid/Array3DTests.cpp"
               "src/test/Renderer/StaticMeshSerializerTests.cpp"
               "src/test/Terrain/MesherMarchingCubesTests.cpp"
               "src/test/Terrain/MesherNaiveSurfaceNetsTests.cpp"
               "src/test/Terrain/VoxelDataSerializerTests.cpp"
               "src/test/Noise/SimplexNoiseTests.cpp"
               "src/test/BlockDataStoreTests.cpp"
               
               ${SOURCE_FILES_GRID}
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
               ${SOURCE_FILES_NOISE}
               ${SOURCE_FILES_MATH}
               ${SOURCE_FILES_FONTS}
               ${SOURCE_FILES_BLOCK_DATA_STORE}
               )
target_link_libraries(${TEST_PROG_NAME}
                      ${CONAN_LIBS}
                      ${OPENGL_LIBRARIES}
                      ${ADDITIONAL_LIBRARIES}
                      )
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake.modules/")
include(ParseAndAddCatchTests)
ParseAndAddCatchTests(${TEST_PROG_NAME})
