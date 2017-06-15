//
//  Terrain.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/16/17.
//
//

#include "Terrain/Terrain.hpp"
#include "Terrain/MesherNaiveSurfaceNets.hpp"
#include "SDL_image.h"
#include "Profiler.hpp"

Terrain::Terrain(const std::shared_ptr<GraphicsDevice> &graphicsDevice,
                 const std::shared_ptr<TaskDispatcher> &dispatcher,
                 const std::shared_ptr<TaskDispatcher> &dispatcherRebuildMesh)
 : _graphicsDevice(graphicsDevice),
   _dispatcher(dispatcher),
   _dispatcherRebuildMesh(dispatcherRebuildMesh),
   _mesher(new MesherNaiveSurfaceNets),
   _voxels(new VoxelDataStore),
   _meshFetchInFlight(0)
{
    // Load terrain texture array from a single image.
    // TODO: create a TextureArrayLoader class to encapsulate tex loading.
    SDL_Surface *surface = IMG_Load("terrain.png");
    
    if (!surface) {
        throw Exception("Failed to load terrain terrain.png.");
    }
    
    TextureDescriptor texDesc = {
        Texture2DArray,
        BGRA8,
        static_cast<size_t>(surface->w),
        static_cast<size_t>(surface->w),
        static_cast<size_t>(surface->h / surface->w),
        4,
        true,
    };
    auto texture = graphicsDevice->makeTexture(texDesc, surface->pixels);
    
    SDL_FreeSurface(surface);
    
    TextureSamplerDescriptor samplerDesc = {
        ClampToEdge,
        ClampToEdge,
        NearestMipMapNearest,
        Nearest
    };
    auto sampler = graphicsDevice->makeTextureSampler(samplerDesc);
    
    StaticMesh mesh; // An empty mesh still has a valid vertex format.
    auto shader = _graphicsDevice->makeShader(mesh.getVertexFormat(),
                                              "vert", "frag",
                                              false);
    
    TerrainUniforms uniforms;
    auto uniformBuffer = _graphicsDevice->makeBuffer(sizeof(uniforms),
                                                     &uniforms,
                                                     DynamicDraw,
                                                     UniformBuffer);
    uniformBuffer->addDebugMarker("Terrain Uniforms", 0, sizeof(uniforms));
    
    // We don't have vertices until the isosurface is extracted later.
    _defaultMesh = std::make_shared<RenderableStaticMesh>();
    _defaultMesh->vertexCount = 0;
    _defaultMesh->buffer = nullptr;
    _defaultMesh->uniforms = uniformBuffer;
    _defaultMesh->shader = shader;
    _defaultMesh->texture = texture;
    _defaultMesh->textureSampler = sampler;
    
    const AABB box = _voxels->boundingBox();
    const glm::ivec3 res(8, 8, 8);
    _drawList = std::make_unique<TerrainDrawList>(box, res);
    _meshes = std::make_unique<Array3D<MaybeTerrainMesh>>(box, res);
    
    // When voxels change, we need to extract a polygonal mesh representation
    // of the isosurface. This mesh is what we actually draw.
    // For now, we extract the entire isosurface in one step.
    _voxels->voxelDataChanged.connect([&](const ChangeLog &changeLog){
        asyncRebuildMeshes(changeLog);
    });
}

void Terrain::setTerrainUniforms(const TerrainUniforms &uniforms)
{
    // The uniforms referenced in the default mesh are also referenced by other
    // meshes in the terrain. Setting it once here sets it for all of them.
    _defaultMesh->uniforms->replace(sizeof(uniforms), &uniforms);
    
    // Extract the camera position from the camera transform.
    const glm::vec3 cameraPos = glm::vec3(glm::inverse(uniforms.view)[3]);
    _dispatcher->async([this, cameraPos]{
        std::unique_lock<std::shared_mutex> lock(_lockCameraPosition);
        _cameraPos = cameraPos;
    });
    
    // We'll use the MVP later to extract the camera frustum.
    _modelViewProjection = uniforms.proj * uniforms.view;
}

void Terrain::draw(const std::shared_ptr<CommandEncoder> &encoder)
{
    // If any meshes are missing then kick off an async task to fetch them.
    // Limit the number of these tasks that can be in flight at once.
    //
    // There's bit of race between comparing the counter and incrementing it,
    // but that doesn't matter since we merely want a good effort at limiting
    // the number of these tasks in the queue.
    if (_meshFetchInFlight < 1) {
        _meshFetchInFlight++;
        _dispatcherRebuildMesh->async([=]{
            PROFILER(TerrainFetchMeshes);
            std::lock_guard<std::mutex> lock(_lockMeshes);
            _meshes->forEachCell(_meshes->boundingBox(), [&](const AABB &cell){
                const MaybeTerrainMesh &maybeTerrainMesh = _meshes->get(cell.center);
                _drawList->tryUpdateDrawList(maybeTerrainMesh, cell);
                if (!maybeTerrainMesh) {
                    asyncRebuildAnotherMesh(cell);
                }
            });
            _meshFetchInFlight--;
        });
    }
    
    encoder->setShader(_defaultMesh->shader);
    encoder->setFragmentSampler(_defaultMesh->textureSampler, 0);
    encoder->setFragmentTexture(_defaultMesh->texture, 0);
    encoder->setVertexBuffer(_defaultMesh->uniforms, 1);
    
    _drawList->draw(encoder, _modelViewProjection);
}

void Terrain::asyncRebuildMeshes(const ChangeLog &changeLog)
{
    PROFILER(TerrainAsyncRebuildMeshes);
    
    // Get the set of meshes which are affected by the changes.
    std::set<AABB> affectedMeshes;
    {
        std::lock_guard<std::mutex> lock(_lockMeshes);
        for (const auto &change : changeLog) {
            _meshes->forEachCell(change.affectedRegion, [&](const AABB &cell){
                affectedMeshes.insert(cell);
            });
        }
    }
    
    // Kick off a task to rebuild each affected mesh.
    for (const AABB &cell : affectedMeshes) {
        asyncRebuildAnotherMesh(cell);
    }
}

void Terrain::asyncRebuildAnotherMesh(const AABB &cell)
{
    if (_meshesToRebuild.push(cell)) {
        _dispatcherRebuildMesh->async([=]{
            rebuildNextMesh();
        });
    }
}

void Terrain::rebuildNextMesh()
{
    PROFILER(TerrainRebuildNextMesh);
    
    static glm::vec3 cameraPosition;
    {
        std::shared_lock<std::shared_mutex> lock(_lockCameraPosition);
        cameraPosition = _cameraPos;
    }
    
    auto maybeCell = _meshesToRebuild.pop(cameraPosition);
    
    if (maybeCell) {
        const AABB &cell = *maybeCell;
        std::lock_guard<std::mutex> lock(_lockMeshes);
        MaybeTerrainMesh &maybeTerrainMesh = _meshes->mutableReference(cell.center);
        if (!maybeTerrainMesh) {
            maybeTerrainMesh.emplace(cell,
                                     _defaultMesh,
                                     _graphicsDevice,
                                     _mesher,
                                     _voxels);
        }
        maybeTerrainMesh->rebuild();
        _drawList->tryUpdateDrawList(maybeTerrainMesh, cell);
    }
}
