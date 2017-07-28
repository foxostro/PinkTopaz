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

Terrain::~Terrain()
{
    _dispatcher->shutdown();
    _dispatcherRebuildMesh->shutdown();
}

Terrain::Terrain(const std::shared_ptr<GraphicsDevice> &graphicsDevice,
                 const std::shared_ptr<TaskDispatcher> &dispatcher,
                 const std::shared_ptr<TaskDispatcher> &dispatcherRebuildMesh)
 : _graphicsDevice(graphicsDevice),
   _dispatcher(dispatcher),
   _dispatcherRebuildMesh(dispatcherRebuildMesh),
   _mesher(new MesherNaiveSurfaceNets),
   _voxelDataGenerator(new VoxelDataGenerator(/* random seed = */ 0)),
   _voxels(new VoxelDataStore(_voxelDataGenerator, TERRAIN_CHUNK_SIZE))
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
    auto meshesArray = std::make_unique<Array3D<MaybeTerrainMesh>>(box, res);
    _meshes = std::make_unique<TerrainMeshGrid>(std::move(meshesArray), 1);
    
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
    
    const glm::vec3 cameraPos = _cameraPosition;
    const float horizonDistance = _horizonDistance.get();
    const AABB horizonBox = {cameraPos, glm::vec3(horizonDistance, horizonDistance, horizonDistance)};
    const AABB activeRegion = _meshes->boundingBox().intersect(horizonBox);
    
    encoder->setShader(_defaultMesh->shader);
    encoder->setFragmentSampler(_defaultMesh->textureSampler, 0);
    encoder->setFragmentTexture(_defaultMesh->texture, 0);
    encoder->setVertexBuffer(_defaultMesh->uniforms, 1);
    
    // Draw meshes in the camera frustum.
    bool anyMeshesMissing = _drawList->draw(encoder, frustum, activeRegion);
    
    if (anyMeshesMissing) {
        // If any meshes are missing then kick off async tasks to fetch them.
        _dispatcher->async([=]{
            PROFILER(TerrainFetchMeshes);
            
            // For each mesh that is present, update the draw list to include that
            // mesh.
            auto onPresent = [&](const TerrainMesh &terrainMesh){
                _drawList->updateDrawList(terrainMesh);
            };
            
            // For each mesh that is missing, or for which we cannot take the lock
            // without blocking, kick off a task to fetch it asynchronously.
            auto onMissing = [&](const AABB &cell){
                _meshesToRebuild.push(cell);
                _dispatcherRebuildMesh->async([=]{ rebuildNextMesh(); });
            };
            
            _meshes->readerTransactionTry(activeRegion, onPresent, onMissing);
        });
    } else {
        // If no meshes were missing then increase the horizon distance so
        // we can fetch meshes further away next time.
        _horizonDistance.increment_clamp(ACTIVE_REGION_SIZE);
    }
}

float Terrain::getFogDensity() const
{
#if 0
    // Fog density decreases as the terrain horizon distance increases.
    // The density falls along an exponential decay curve. These particular
    // tuning values were picked because they look pretty good.
    constexpr float initialValue = 0.3f;
    constexpr float floorValue = 0.003f;
    constexpr float decayRate = -8.f;
    const float maxHorizon = glm::length(_voxels->boundingBox().extent);
    const float horizonDistance = _horizonDistance.get();
    const float t = std::min(horizonDistance / maxHorizon, 1.0f);
    const float fogDensity = initialValue * std::exp(decayRate * t) + floorValue;
    return fogDensity;
#else
    return 0.003f;
#endif
}

void Terrain::rebuildMeshInResponseToChanges(const ChangeLog &changeLog)
{
    PROFILER(TerrainRebuildMeshInResponseToChanges);
    
    // Kick off a task to rebuild each affected mesh.
    for (const auto &change : changeLog) {
        const AABB &region = change.affectedRegion;
        _meshes->readerTransaction(region, [&](const AABB &cell,
                                               Morton3 index,
                                               const MaybeTerrainMesh &value){
            if (_meshesToRebuild.push(cell)) {
                _dispatcherRebuildMesh->async([=]{
                    rebuildNextMesh();
                });
            }
        });
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
    do {
        auto maybeCell = _meshesToRebuild.pop();
        if (!maybeCell) {
            return; // Bail if there are no meshes to rebuild.
        }
        cell = *maybeCell;
    } while (!doBoxesIntersect(horizonBox, cell));
    
    _meshes->writerTransaction(cell, [&](const AABB &cell,
                                         Morton3 index,
                                         MaybeTerrainMesh &maybe){
        if (!maybe) {
            maybe.emplace(cell, _defaultMesh, _graphicsDevice, _mesher, _voxels);
        }
        maybe->rebuild();
    });
}
