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
#include "Grid/Array3D.hpp"
#include <sstream>

#define TERRAIN_PROGRESS_TRACKER 0

Terrain::~Terrain()
{
    _dispatcher->shutdown();
    _dispatcherRebuildMesh->shutdown();
}

Terrain::Terrain(const std::shared_ptr<GraphicsDevice> &graphicsDevice,
                 const std::shared_ptr<TaskDispatcher> &dispatcher,
                 const std::shared_ptr<TaskDispatcher> &dispatcherRebuildMesh,
                 const std::shared_ptr<TaskDispatcher> &dispatcherVoxelData)
 : _graphicsDevice(graphicsDevice),
   _dispatcher(dispatcher),
   _dispatcherRebuildMesh(dispatcherRebuildMesh),
   _mesher(std::make_shared<MesherNaiveSurfaceNets>()),
   _voxelDataGenerator(std::make_shared<VoxelDataGenerator>(/* random seed = */ 52)),
   _voxels(std::make_shared<TransactedVoxelData>(std::make_unique<VoxelData>(_voxelDataGenerator, TERRAIN_CHUNK_SIZE, dispatcherVoxelData))),
   _cameraPosition(glm::vec3())
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
    
    const AABB box = _voxels->boundingBox().inset(glm::vec3((float)TERRAIN_CHUNK_SIZE, (float)TERRAIN_CHUNK_SIZE, (float)TERRAIN_CHUNK_SIZE));
    const glm::ivec3 res = _voxelDataGenerator->countCellsInRegion(box) / (int)TERRAIN_CHUNK_SIZE;
    _drawList = std::make_unique<TerrainDrawList>(box, res);
    _meshes = std::make_unique<TerrainMeshGrid>(box, res);

    // Limit the sparse grids to the number of chunks in the active region.
    // Now, the active region can change size over time but it will never be
    // larger than this size.
    const unsigned workingSetCount = std::pow(1 + 2*ACTIVE_REGION_SIZE / TERRAIN_CHUNK_SIZE, 3);
    _voxels->setChunkCountLimit(2*workingSetCount);
    _meshes->setCountLimit(2*workingSetCount);
    _drawList->setCountLimit(2*workingSetCount);
    
    // When voxels change, we need to extract a polygonal mesh representation
    // of the isosurface. This mesh is what we actually draw.
    // For now, we extract the entire isosurface in one step.
    _voxels->onWriterTransaction.connect([&](const ChangeLog &changeLog){
        rebuildMeshInResponseToChanges(changeLog);
    });
}

void Terrain::update(entityx::TimeDelta dt)
{
    _horizonDistance.update(dt);
}

void Terrain::setTerrainUniforms(const TerrainUniforms &uniforms)
{
    // The uniforms referenced in the default mesh are also referenced by other
    // meshes in the terrain. Setting it once here sets it for all of them.
    _defaultMesh->uniforms->replace(sizeof(uniforms), &uniforms);
    
    // Extract the camera position from the camera transform.
    const glm::vec3 cameraPos = glm::vec3(glm::inverse(uniforms.view)[3]);
    _dispatcher->async([this, cameraPos]{
        _cameraPosition = cameraPos;
    });
    
    // We'll use the MVP later to extract the camera frustum.
    _modelViewProjection = uniforms.proj * uniforms.view;
}

void Terrain::draw(const std::shared_ptr<CommandEncoder> &encoder)
{
    Frustum frustum(_modelViewProjection);
    
    encoder->setShader(_defaultMesh->shader);
    encoder->setFragmentSampler(_defaultMesh->textureSampler, 0);
    encoder->setFragmentTexture(_defaultMesh->texture, 0);
    encoder->setVertexBuffer(_defaultMesh->uniforms, 1);
    
    // Draw meshes in the camera frustum.
    auto missingMeshes = _drawList->draw(encoder, frustum, getActiveRegion());
    
    if (missingMeshes.empty()) {
        // If no meshes were missing then increase the horizon distance so
        // we can fetch meshes further away next time.
        float d = _horizonDistance.increment_clamp(ACTIVE_REGION_SIZE);
        SDL_Log("Increasing horizon distance to %.2f", d);
    } else {
        _dispatcher->async([this, missingMeshes{std::move(missingMeshes)}]{
            fetchMeshes(missingMeshes);
        });
    }
}

void Terrain::fetchMeshes(const std::vector<AABB> &cells)
{
    PROFILER(TerrainFetchMeshes);
    
    // It's important to mark cells in the progress tracker in order of
    // distance to the camera. If we don't do this then close chunks do not
    // complete before far away chunks.
    const glm::vec3 cameraPos = _cameraPosition;
    std::vector<AABB> meshCells(cells);
    std::sort(meshCells.begin(),
              meshCells.end(),
              [cameraPos](const AABB &a, const AABB &b){
                  const auto distA = glm::distance(a.center, cameraPos);
                  const auto distB = glm::distance(b.center, cameraPos);
                  return distA < distB;
              });
    
    const auto meshesNewlyInflight = _progressTracker.beginCellsNotInflight(meshCells);
    _meshesToRebuild.push(meshesNewlyInflight);
    _dispatcherRebuildMesh->map(meshesNewlyInflight.size(), [this]{
        rebuildNextMesh();
    });
}

float Terrain::getFogDensity() const
{
    // Fog density decreases as the terrain horizon distance increases.
    // The density falls along an exponential decay curve. These particular
    // tuning values were picked because they look pretty good.
    constexpr float initialValue = 0.05f;
    constexpr float floorValue = 0.003f;
    constexpr float decayRate = -32.f;
    const float maxHorizon = glm::length(_voxels->boundingBox().extent);
    const float horizonDistance = _horizonDistance.get();
    const float t = std::min(horizonDistance / maxHorizon, 1.0f);
    const float fogDensity = initialValue * std::exp(decayRate * t) + floorValue;
    return fogDensity;
}

void Terrain::rebuildMeshInResponseToChanges(const ChangeLog &changeLog)
{
    PROFILER(TerrainRebuildMeshInResponseToChanges);
    
    // Kick off a task to rebuild each affected mesh.
    for (const auto &change : changeLog) {
        const AABB &region = change.affectedRegion;
        std::vector<AABB> meshCells;
        for (const auto &cellCoords : _meshes->slice(region)) {
            const AABB cell = _meshes->cellAtCellCoords(cellCoords);
            meshCells.push_back(cell);
        }
        fetchMeshes(meshCells);
    }
}

void Terrain::rebuildNextMesh()
{
    PROFILER(TerrainRebuildNextMesh);
    
    const glm::vec3 cameraPos = _cameraPosition;
    const float horizonDistance = _horizonDistance.get();
    const AABB horizonBox = {cameraPos, glm::vec3(horizonDistance, horizonDistance, horizonDistance)};
    AABB cell;
    
    // Grab the next mesh to rebuild that is inside the horizon.
    while (true) {
        auto maybeCell = _meshesToRebuild.pop();
        if (!maybeCell) {
            return; // Bail if there are no meshes to rebuild.
        }
        cell = *maybeCell;
        
        if (doBoxesIntersect(horizonBox, cell)) {
            // We found a good one. Break out of the loop now.
            break;
        } else {
            // This one isn't in the active region so cancel it and move on.
            SDL_Log("Cancelling mesh at %s because it fell out of the " \
                    "active region", cell.to_string().c_str());
            _progressTracker.cancel(cell);
            continue;
        }
    }
    
    // Rebuild the mesh and then stick it in the grid.
    auto terrainMesh = std::make_shared<TerrainMesh>(cell, _defaultMesh, _graphicsDevice, _mesher, _voxels);
    terrainMesh->rebuild();
    _meshes->set(cell.center, terrainMesh);
    _drawList->updateDrawList(*terrainMesh);

    // Mark the cell as being complete.
#if TERRAIN_PROGRESS_TRACKER
    {
        const auto duration = _progressTracker.finish(cell);
        
        // Log the amount of time it took.
        const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(duration);
        const std::string str = std::to_string(ms.count());
        SDL_Log("Completed mesh at %s in %s ms.",
                cell.to_string().c_str(), str.c_str());
    }
#else
    _progressTracker.finish(cell);
#endif
}

