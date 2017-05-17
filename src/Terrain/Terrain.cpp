//
//  Terrain.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/16/17.
//
//

#include "FileUtilities.hpp"
#include "Terrain/VoxelDataLoader.hpp"
#include "Terrain/Terrain.hpp"
#include "RenderableStaticMesh.hpp"
#include "SDL_image.h"
#include "Exception.hpp"

Terrain::Terrain(const std::shared_ptr<GraphicsDevice> &graphicsDevice)
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
    
    // Create a voxel data store and fill it with voxel data we load from file.
    // We need the voxel data store to use the dimensions and resolution of the
    // data contained in the file.
    std::vector<uint8_t> bytes = binaryFileContents("0_0_0.voxels.dat");
    AABB box;
    glm::ivec3 res;
    VoxelDataLoader voxelDataLoader;
    voxelDataLoader.retrieveDimensions(bytes, box, res);
    VoxelDataStore voxelDataStore(box, res);
    voxelDataStore.writerTransaction([&](VoxelData &voxels){
        voxelDataLoader.load(bytes, voxels);
    });
    
    // The voxel file uses a binary SOLID/EMPTY flag for voxels. So, we get
    // values that are either 0.0 or 1.0.
    std::shared_ptr<Mesher> mesher(new MesherMarchingCubes());
    StaticMesh mesh;
    voxelDataStore.readerTransaction([&](const VoxelData &voxels){
        mesh = mesher->extract(voxels, 0.5f);
    });
    
    auto vertexBufferData = mesh.getBufferData();
    auto vertexBuffer = graphicsDevice->makeBuffer(vertexBufferData,
                                                   StaticDraw,
                                                   ArrayBuffer);
    vertexBuffer->addDebugMarker("Terrain Vertices", 0, vertexBufferData.size());
    
    TerrainUniforms uniforms;
    auto uniformBuffer = graphicsDevice->makeBuffer(sizeof(uniforms),
                                                    &uniforms,
                                                    DynamicDraw,
                                                    UniformBuffer);
    vertexBuffer->addDebugMarker("Terrain Uniforms", 0, sizeof(uniforms));
    
    auto shader = graphicsDevice->makeShader(mesh.getVertexFormat(),
                                             "vert", "frag",
                                             false);
    
    _mesh = (RenderableStaticMesh) {
        mesh.getVertexCount(),
        vertexBuffer,
        uniformBuffer,
        shader,
        texture,
        sampler
    };
}
