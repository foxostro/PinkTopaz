//
//  Terrain.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/16/17.
//
//

#include "FileUtilities.hpp"
#include "Terrain/VoxelDataLoader.hpp"
#include "SDL_image.h"
#include "Exception.hpp"
#include "Terrain/MesherNaiveSurfaceNets.hpp"
#include "Terrain/Terrain.hpp"
#include "Profiler.hpp"
#include <set>

Terrain::Terrain(const std::shared_ptr<GraphicsDevice> &graphicsDevice,
                 const std::shared_ptr<TaskDispatcher> &dispatcher)
 : _graphicsDevice(graphicsDevice),
   _dispatcher(dispatcher),
   _mesher(new MesherNaiveSurfaceNets())
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
    
    // Create a voxel data store. We want to fill this with voxel values we read
    // from file. Before we can do that, we need to initialize the data store to
    // the dimensions of the voxel field found in the file.
    const std::vector<uint8_t> bytes = binaryFileContents("0_0_0.voxels.dat");
    AABB box;
    glm::ivec3 res;
    VoxelDataLoader voxelDataLoader;
    voxelDataLoader.retrieveDimensions(bytes, box, res);
    
    const glm::vec3 chunkSize(MESH_CHUNK_SIZE, MESH_CHUNK_SIZE, MESH_CHUNK_SIZE);
    const AABB boxWithBorder = box.inset(-chunkSize);
    const glm::ivec3 resWithBorder = res + glm::ivec3(MESH_CHUNK_SIZE, MESH_CHUNK_SIZE, MESH_CHUNK_SIZE)*2;
    
    _voxels = std::make_unique<VoxelDataStore>(boxWithBorder, resWithBorder);
    _meshes = std::make_unique<Array3D<RenderableStaticMesh>>(box, res / MESH_CHUNK_SIZE);
    
    // When voxels change, we need to extract a polygonal mesh representation
    // of the isosurface. This mesh is what we actually draw.
    // For now, we extract the entire isosurface in one step.
    _voxels->voxelDataChanged.connect([&](const ChangeLog &changeLog){
        rebuildMesh(changeLog);
    });
    
    // Finally, actually load the voxel values from file.
    // For now, we load all voxels in one step.
    // AFOX_TODO: Use direct array access here.
    _voxels->writerTransaction(boxWithBorder, [&](GridMutable<Voxel> &voxels){
        assert(voxels.cellDimensions() == glm::vec3(1.0, 1.0, 1.0));
        
        voxels.mutableForEachCell(boxWithBorder, [&](const AABB &cell){
            return Voxel();
        });
        voxelDataLoader.load(bytes, voxels);
        return ChangeLog::make("load", box);
    });
}

Terrain::~Terrain()
{
    std::lock_guard<std::mutex> lock(_lockMeshes);
    _dispatcher->shutdown();
}

void Terrain::setTerrainUniforms(const TerrainUniforms &uniforms)
{
    std::lock_guard<std::mutex> lock(_lockMeshes);
    
    // The uniforms referenced in the default mesh are also referenced by other
    // meshes in the terrain. Setting it once here sets it for all of them.
    _defaultMesh.uniforms->replace(sizeof(uniforms), &uniforms);
}

void Terrain::draw(const std::shared_ptr<CommandEncoder> &encoder) const
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

void Terrain::rebuildMeshForChunkInner(const Array3D<Voxel> &voxels,
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

void Terrain::rebuildMeshForChunkOuter(const size_t index, const AABB &meshBox)
{
    // We need a border of voxels around the region of the mesh in order to
    // perform surface extraction.
    const AABB voxelBox = meshBox.inset(-glm::vec3(1, 1, 1));
    
    _voxels->readerTransaction(voxelBox, [&](const Array3D<Voxel> &voxels){
        rebuildMeshForChunkInner(voxels, index, meshBox);
    });
}

void Terrain::rebuildMesh(const ChangeLog &changeLog)
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
