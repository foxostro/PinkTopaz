//
//  TerrainMeshes.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 6/12/17.
//
//

#include "Terrain/TerrainMeshes.hpp"
#include "Terrain/MesherNaiveSurfaceNets.hpp"
#include "SDL_image.h"

TerrainMeshes::~TerrainMeshes()
{
    std::lock_guard<std::mutex> lock(_lockMeshes);
    _dispatcher->shutdown();
}

TerrainMeshes::TerrainMeshes(const std::shared_ptr<GraphicsDevice> &graphicsDevice,
                             const std::shared_ptr<TaskDispatcher> &dispatcher,
                             const std::shared_ptr<VoxelDataStore> &voxels)
 : _graphicsDevice(graphicsDevice),
   _dispatcher(dispatcher),
   _mesher(new MesherNaiveSurfaceNets),
   _voxels(voxels)
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
    const glm::ivec3 res = _voxels->gridResolution() / MESH_CHUNK_SIZE;
    _meshes = std::make_unique<Array3D<MaybeTerrainMesh>>(box, res);
    
    // When voxels change, we need to extract a polygonal mesh representation
    // of the isosurface. This mesh is what we actually draw.
    // For now, we extract the entire isosurface in one step.
    _voxels->voxelDataChanged.connect([&](const ChangeLog &changeLog){
        rebuildMesh(changeLog);
    });
}

void TerrainMeshes::setTerrainUniforms(const TerrainUniforms &uniforms)
{
    std::lock_guard<std::mutex> lock(_lockMeshes);
    
    // The uniforms referenced in the default mesh are also referenced by other
    // meshes in the terrain. Setting it once here sets it for all of them.
    _defaultMesh->uniforms->replace(sizeof(uniforms), &uniforms);
}

void TerrainMeshes::draw(const std::shared_ptr<CommandEncoder> &encoder) const
{
    std::lock_guard<std::mutex> lock(_lockMeshes);
    
    // The following resources are referenced by and used by all meshes in the
    // terrain. We only need to set them once.
    encoder->setShader(_defaultMesh->shader);
    encoder->setFragmentSampler(_defaultMesh->textureSampler, 0);
    encoder->setFragmentTexture(_defaultMesh->texture, 0);
    encoder->setVertexBuffer(_defaultMesh->uniforms, 1);
    
    // Draw each mesh that is available.
    // Request that each missing mesh be generated asynchronously.
    _meshes->forEachCell(_meshes->boundingBox(), [&](const AABB &cell){
        const MaybeTerrainMesh &maybeMesh = _meshes->get(cell.center);
        if (maybeMesh) {
            maybeMesh->draw(encoder);
        } else {
            _dispatcher->async([=]{
                std::lock_guard<std::mutex> lock(_lockMeshes);
                MaybeTerrainMesh &maybeMesh = _meshes->mutableReference(cell.center);
                if (!maybeMesh) {
                    maybeMesh.emplace(cell,
                                      _defaultMesh,
                                      _graphicsDevice,
                                      _mesher,
                                      _voxels);
                    maybeMesh->rebuild();
                }
            });
        }
    });
}

void TerrainMeshes::rebuildMesh(const ChangeLog &changeLog)
{
    std::lock_guard<std::mutex> lock(_lockMeshes);
    
    // Get the set of meshes which are affected by the changes.
    std::set<AABB> affectedMeshes;
    for (const auto &change : changeLog) {
        _meshes->forEachCell(change.affectedRegion, [&](const AABB &cell){
            affectedMeshes.insert(cell);
        });
    }
    
    // Kick off a task to rebuild each affected mesh.
    for (const AABB &cell : affectedMeshes) {
        _dispatcher->async([=]{
            std::lock_guard<std::mutex> lock(_lockMeshes);
            MaybeTerrainMesh &maybeMesh = _meshes->mutableReference(cell.center);
            if (!maybeMesh) {
                maybeMesh.emplace(cell,
                                  _defaultMesh,
                                  _graphicsDevice,
                                  _mesher,
                                  _voxels);
            }
            maybeMesh->rebuild();
        });
    }
}
