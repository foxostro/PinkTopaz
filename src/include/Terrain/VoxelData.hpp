//
//  VoxelData.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 3/13/17.
//
//

#ifndef VoxelData_hpp
#define VoxelData_hpp

#include "Grid/SparseGrid.hpp"
#include "Voxel.hpp"
#include "VoxelDataGenerator.hpp"
#include <memory>

// A block of voxels in space.
class VoxelData : public GridIndexer
{
public:
    using GeneratorPtr = std::shared_ptr<VoxelDataGenerator>;
    
    // Default Destructor.
    ~VoxelData() = default;
    
    // No default constructor.
    VoxelData() = delete;
    
    // Constructor.
    // generator -- The generator provides initial voxel data.
    // chunkSize -- The size of chunk VoxelData should use internally.
    VoxelData(const GeneratorPtr &generator, unsigned chunkSize);
    
    // VoxelData may evict chunks to keep the total chunk count under this
    // limit. Pass in at least the expected number of chunks in the working set.
    void setChunkCountLimit(unsigned chunkCountLimit);
    
    // Loads a copy of the contents of the specified sub-region of the grid to
    // an Array3D and returns that. May fault in missing voxels to satisfy the
    // request.
    Array3D<Voxel> load(const AABB &region);
    
    // Stores the contents of the specified array of voxels to the grid.
    void store(const Array3D<Voxel> &voxels);
    
private:
    using Chunk = Array3D<Voxel>;
    using ChunkPtr = std::shared_ptr<Chunk>;
    
    const GeneratorPtr &_generator;
    SparseGrid<ChunkPtr> _chunks;
    
    // Gets the chunk, creating it if necessary.
    ChunkPtr get(const AABB &bbox, Morton3 index);
};

#endif /* VoxelData_hpp */
