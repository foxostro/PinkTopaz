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
    _defaultMesh = (RenderableStaticMesh) {
        0,
        nullptr,
        uniformBuffer,
        shader,
        texture,
        sampler
    };
    
    const AABB box = _voxels->boundingBox();
    const glm::ivec3 res = _voxels->gridResolution() / MESH_CHUNK_SIZE;
    _meshes = std::make_unique<Array3D<RenderableStaticMesh>>(box, res);
    
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
    _defaultMesh.uniforms->replace(sizeof(uniforms), &uniforms);
}

void TerrainMeshes::draw(const std::shared_ptr<CommandEncoder> &encoder) const
{
    std::lock_guard<std::mutex> lock(_lockMeshes);
    
    // The following resources are referenced by and used by all meshes in the
    // terrain. We only need to set them once.
    encoder->setShader(_defaultMesh.shader);
    encoder->setFragmentSampler(_defaultMesh.textureSampler, 0);
    encoder->setFragmentTexture(_defaultMesh.texture, 0);
    encoder->setVertexBuffer(_defaultMesh.uniforms, 1);
    
    for (const RenderableStaticMesh &mesh : *_meshes) {
        if (mesh.vertexCount > 0) {
            encoder->setVertexBuffer(mesh.buffer, 0);
            encoder->drawPrimitives(Triangles, 0, mesh.vertexCount, 1);
        }
    }
}

void TerrainMeshes::rebuildMeshForChunkInner(const Array3D<Voxel> &voxels,
                                             const size_t index,
                                             const AABB &meshBox)
{
    // The voxel file uses a binary SOLID/EMPTY flag for voxels.
    // So, we get values that are either 0.0 or 1.0.
    constexpr float isosurface = 0.5f;
    
    StaticMesh mesh = _mesher->extract(voxels, meshBox, isosurface);
    
    std::shared_ptr<Buffer> vertexBuffer = nullptr;
    
    if (mesh.getVertexCount() > 0) {
        // AFOX_TODO: We may need to create graphics resources on the main thread, e.g., when using OpenGL.
        auto vertexBufferData = mesh.getBufferData();
        vertexBuffer = _graphicsDevice->makeBuffer(vertexBufferData,
                                                   StaticDraw,
                                                   ArrayBuffer);
        vertexBuffer->addDebugMarker("Terrain Vertices", 0, vertexBufferData.size());
    }
    
    RenderableStaticMesh renderableStaticMesh = _defaultMesh;
    renderableStaticMesh.vertexCount = mesh.getVertexCount();
    renderableStaticMesh.buffer = vertexBuffer;
    
    {
        std::lock_guard<std::mutex> lock(_lockMeshes);
        _meshes->set(index, renderableStaticMesh);
    }
}

void TerrainMeshes::rebuildMeshForChunkOuter(const size_t index, const AABB &meshBox)
{
    // We need a border of voxels around the region of the mesh in order to
    // perform surface extraction.
    const AABB voxelBox = meshBox.inset(-glm::vec3(1, 1, 1));
    
    _voxels->readerTransaction(voxelBox, [&](const Array3D<Voxel> &voxels){
        rebuildMeshForChunkInner(voxels, index, meshBox);
    });
}

void TerrainMeshes::rebuildMesh(const ChangeLog &changeLog)
{
    // Get the set of meshes (by index) which are affected by the changes.
    // Only these meshes will need to be rebuilt.
    std::set<std::pair<size_t, AABB>> affectedMeshes;
    for (const auto &change : changeLog) {
        const auto &region = change.affectedRegion;
        const auto indices = _meshes->indicesOverRegion(region);
        affectedMeshes.insert(indices.begin(), indices.end());
    }
    
    for (const std::pair<size_t, AABB> pair : affectedMeshes) {
        _dispatcher->async([=]{
            rebuildMeshForChunkOuter(pair.first, pair.second);
        });
    }
}
