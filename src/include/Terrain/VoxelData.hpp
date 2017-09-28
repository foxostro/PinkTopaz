//
//  VoxelData.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 3/13/17.
//
//

#ifndef VoxelData_hpp
#define VoxelData_hpp

#include "Grid/Array3D.hpp"
#include "Voxel.hpp"
#include "VoxelDataGenerator.hpp"
#include <boost/optional.hpp>

// A block of voxels in space.
class VoxelData : public GridIndexer
{
public:
    using Chunk = Array3D<Voxel>;
    using MaybeChunk = boost::optional<Chunk>;
    
    // Default Destructor.
    ~VoxelData() = default;
    
    // No default constructor.
    VoxelData() = delete;
    
    // Constructor. Accepts `generator' which can generate voxel terrain as
    // needed to fill the chunks underlying VoxelData.
    VoxelData(const std::shared_ptr<VoxelDataGenerator> &generator,
              unsigned chunkSize);
    
    // Returns an array which holds a copy of the contents of the subregion.
    Array3D<Voxel> copy(const AABB &region) const;
    
private:
    // Provides voxel data for regions of space where none is available yet.
    const std::shared_ptr<VoxelDataGenerator> &_generator;
    
    // The voxel grid is broken into chunks where each chunk is a fixed-size
    // grid of voxels.
    mutable Array3D<MaybeChunk> _chunks;
    
    // Fetches the chunk at the specified point in space `p'. This may create
    // the chunk. If so, it is filled using the generator.
    MaybeChunk& chunkAtPoint(const glm::vec3 &p) const;
    
    // If the chunk is not valid then emplace a new one filled using the voxel
    // data generator. This assumes the lock has already been taken.
    void emplaceChunkIfNecessary(const glm::vec3 &p,
                                 MaybeChunk &maybeChunk) const;
};

#endif /* VoxelData_hpp */
