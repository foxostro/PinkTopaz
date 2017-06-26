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
    _meshes = std::make_unique<ConcurrentGridMutable<MaybeTerrainMesh>>(std::move(meshesArray), 1);
    
    // When voxels change, we need to extract a polygonal mesh representation
    // of the isosurface. This mesh is what we actually draw.
    // For now, we extract the entire isosurface in one step.
    _voxels->onWriterTransaction.connect([&](const ChangeLog &changeLog){
        rebuildMeshInResponseToChanges(changeLog);
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
    Frustum frustum(_modelViewProjection);
    
    // Update the draw list. If any meshes are missing then kick off tasks
    // to fetch them asynchronously.
    _dispatcher->async([=]{
        PROFILER(TerrainFetchMeshes);
        _meshes->readerTransaction(frustum, [&](const GridAddressable<MaybeTerrainMesh> &data){
            std::vector<AABB> missingMeshes;
            
            data.forEachCell(frustum, [&](const AABB &cell,
                                          Morton3 index,
                                          const MaybeTerrainMesh &maybe){
                if (maybe) {
                    _drawList->updateDrawList(*maybe, cell);
                } else {
                    missingMeshes.push_back(cell);
                }
            });
            
            // Kick off a task to rebuild each mesh that was missing.
            for (size_t i = 0, n = _meshesToRebuild.push(missingMeshes); i < n; ++i) {
                _dispatcherRebuildMesh->async([=]{ rebuildNextMesh(); });
            }
        });
    });
    
    encoder->setShader(_defaultMesh->shader);
    encoder->setFragmentSampler(_defaultMesh->textureSampler, 0);
    encoder->setFragmentTexture(_defaultMesh->texture, 0);
    encoder->setVertexBuffer(_defaultMesh->uniforms, 1);
    
    _drawList->draw(encoder, frustum);
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
    
    static glm::vec3 cameraPosition;
    {
        std::shared_lock<std::shared_mutex> lock(_lockCameraPosition);
        cameraPosition = _cameraPos;
    }
    
    auto maybeCell = _meshesToRebuild.pop(cameraPosition);
    
    if (maybeCell) {
        const AABB &cell = *maybeCell;        
        _meshes->writerTransaction(cell, [&](const AABB &cell,
                                             Morton3 index,
                                             MaybeTerrainMesh &maybe){
            if (!maybe) {
                maybe.emplace(cell, _defaultMesh, _graphicsDevice, _mesher, _voxels);
            }
            maybe->rebuild();
            _drawList->updateDrawList(*maybe, cell);
        });
    }
}
