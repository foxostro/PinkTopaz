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
#include "Terrain/Terrain.hpp"

Terrain::Terrain(const std::shared_ptr<GraphicsDevice> &graphicsDevice)
 : _graphicsDevice(graphicsDevice),
   _mesher(new MesherMarchingCubes())
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
    _mesh = (RenderableStaticMesh) {
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
    _voxels = std::make_shared<VoxelDataStore>(box, res);
    
    // When voxels change, we need to extract a polygonal mesh representation
    // of the isosurface. This mesh is what we actually draw.
    // For now, we extract the entire isosurface in one step.
    _voxels->voxelDataChanged.connect([&](){
        rebuildMesh();
    });
    
    // Finally, actually load the voxel values from file.
    // For now, we load all voxels in one step.
    _voxels->writerTransaction([&](VoxelData &voxels){
        voxelDataLoader.load(bytes, voxels);
    });
}

void Terrain::setTerrainUniforms(const TerrainUniforms &uniforms)
{
    std::lock_guard<std::mutex> lock(_lockMesh);
    _mesh.uniforms->replace(sizeof(uniforms), &uniforms);
}

void Terrain::draw(const std::shared_ptr<CommandEncoder> &encoder) const
{
    std::lock_guard<std::mutex> lock(_lockMesh);
    encoder->setShader(_mesh.shader);
    encoder->setFragmentSampler(_mesh.textureSampler, 0);
    encoder->setFragmentTexture(_mesh.texture, 0);
    encoder->setVertexBuffer(_mesh.buffer, 0);
    encoder->setVertexBuffer(_mesh.uniforms, 1);
    encoder->drawPrimitives(Triangles, 0, _mesh.vertexCount, 1);
}

void Terrain::rebuildMesh()
{
    _voxels->readerTransaction([&](const VoxelData &voxels){
        std::lock_guard<std::mutex> lock(_lockMesh);
 
        // The voxel file uses a binary SOLID/EMPTY flag for voxels. So, we get
        // values that are either 0.0 or 1.0.
        StaticMesh mesh = _mesher->extract(voxels, 0.5f);
        
        auto vertexBufferData = mesh.getBufferData();
        auto vertexBuffer = _graphicsDevice->makeBuffer(vertexBufferData,
                                                        StaticDraw,
                                                        ArrayBuffer);
        vertexBuffer->addDebugMarker("Terrain Vertices", 0, vertexBufferData.size());
        
        _mesh.vertexCount = mesh.getVertexCount();
        _mesh.buffer = vertexBuffer;
    });
}
