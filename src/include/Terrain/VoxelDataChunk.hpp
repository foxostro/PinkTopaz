//
//  VoxelDataChunk.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/22/18.
//
//

#ifndef VoxelDataChunk_hpp
#define VoxelDataChunk_hpp

#include "Grid/Array3D.hpp"
#include "Grid/GridIndexerRange.hpp"
#include "Terrain/Voxel.hpp"

static const Voxel SkyVoxel{
    /* .value = */ 0,
    /* .sunLight = */ MAX_LIGHT,
    /* .torchLight = */ 0
};

static const Voxel GroundVoxel{
    /* .value = */ 1,
    /* .sunLight = */ MAX_LIGHT,
    /* .torchLight = */ 0
};

class VoxelDataChunk : public GridIndexer
{
public:
    bool complete;
    
    enum Type {
        Array,
        Sky,
        Ground
    };
    
    VoxelDataChunk(const VoxelDataChunk &other)
     : GridIndexer(other.boundingBox(), other.gridResolution()),
       complete(other.complete),
       _type(other._type)
    {
        if (other._voxels) {
            _voxels = std::make_unique<Array3D<Voxel>>(*other._voxels);
        } else {
            _voxels = nullptr;
        }
        assert((_type != Array && !_voxels) || (_type == Array && _voxels));
    }
    
    VoxelDataChunk(VoxelDataChunk &&other)
     : GridIndexer(other.boundingBox(), other.gridResolution()),
       complete(other.complete),
       _type(other._type),
       _voxels(std::move(other._voxels))
    {
        assert((_type != Array && !_voxels) || (_type == Array && _voxels));
    }
    
    VoxelDataChunk& operator=(const VoxelDataChunk &other)
    {
        if (&other != this) {
            complete = other.complete;
            _type = other._type;
            if (other._voxels) {
                _voxels = std::make_unique<Array3D<Voxel>>(*other._voxels);
            } else {
                _voxels = nullptr;
            }
            assert((_type != Array && !_voxels) || (_type == Array && _voxels));
        }
        return *this;
    }
    
    inline auto getType() const
    {
        return _type;
    }
    
    template<typename Point>
    Voxel get(const Point &point) const
    {
        switch (_type) {
            case Array:
                assert(_voxels);
                return _voxels->reference(point);
                
            case Sky:
                return SkyVoxel;
                
            case Ground:
                return GroundVoxel;
                
            default:
                assert(!"unreachable");
        }
    }
    
    template<typename Point>
    void set(const Point &point, const Voxel &value)
    {
        switch (_type) {
            case Array:
                // nothing to do
                break;
                
            case Sky:
                if (value != SkyVoxel) {
                    convertToArray();
                }
                break;
                
            case Ground:
                if (value != GroundVoxel) {
                    convertToArray();
                }
                break;
                
            default:
                assert(!"unreachable");
        }
        
        if (_type == Array) {
            assert(_voxels);
            _voxels->mutableReference(point) = value;
        }
    }
    
    static VoxelDataChunk createArrayChunk(Array3D<Voxel> &&voxels)
    {
        VoxelDataChunk chunk(voxels.boundingBox(), voxels.gridResolution());
        chunk._type = Array;
        chunk._voxels = std::make_unique<Array3D<Voxel>>(std::move(voxels));
        assert(chunk._voxels);
        return chunk;
    }
    
    static VoxelDataChunk createSkyChunk(const AABB &boundingBox,
                                         const glm::ivec3 &gridResolution)
    {
        VoxelDataChunk chunk(boundingBox, gridResolution);
        chunk._type = Sky;
        return chunk;
    }
    
    static VoxelDataChunk createGroundChunk(const AABB &boundingBox,
                                            const glm::ivec3 &gridResolution)
    {
        VoxelDataChunk chunk(boundingBox, gridResolution);
        chunk._type = Ground;
        return chunk;
    }
    
    // Get the uncompressed voxel bytes from the chunk.
    std::vector<uint8_t> getUncompressedBytes() const
    {
        switch (_type) {
            case Array:
            {
                assert(_voxels);
                const glm::ivec3 res = gridResolution();
                const size_t numberOfVoxelBytes = res.x * res.y * res.z * sizeof(Voxel);
                std::vector<uint8_t> uncompressedBytes;
                uncompressedBytes.resize(numberOfVoxelBytes);
                memcpy((void *)uncompressedBytes.data(),
                       (const void *)_voxels->data(),
                       numberOfVoxelBytes);
                return uncompressedBytes;
            }
                
            case Sky:
                // fall through
                
            case Ground:
                return std::vector<uint8_t>();
                
            default:
                assert(!"unreachable");
        }
    }
    
    void convertToArray()
    {
        assert(_type != Array);
        
        _voxels = std::make_unique<Array3D<Voxel>>(boundingBox(), gridResolution());
        
        Voxel value;
        switch (_type) {
            case Sky:
                value = SkyVoxel;
                break;
                
            case Ground:
                value = GroundVoxel;
                break;
                
            case Array:
                // fall through
                
            default:
                assert(!"unreachable");
        }
        
        for (const glm::ivec3 coord : slice(*_voxels, _voxels->boundingBox())) {
            _voxels->mutableReference(coord) = value;
        }
        
        _type = Array;
    }
    
private:
    Type _type;
    std::unique_ptr<Array3D<Voxel>> _voxels;
    
    VoxelDataChunk(const AABB &boundingBox, const glm::ivec3 &gridResolution)
     : GridIndexer(boundingBox, gridResolution),
       complete(false),
       _type(Sky)
    {}
};

#endif /* VoxelDataChunk_hpp */
