//
//  VoxelData.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 3/13/17.
//
//

#ifndef VoxelData_hpp
#define VoxelData_hpp

#include "Array3D.hpp"
#include "Voxel.hpp"
#include "VoxelDataGenerator.hpp"
#include <boost/optional.hpp>
#include <mutex>

// A block of voxels in space.
class VoxelData : public GridMutable<Voxel>
{
public:
    using GridMutable<Voxel>::get;
    
    using Chunk = Array3D<Voxel>;
    using MaybeChunk = boost::optional<Chunk>;
    
    // Constructor. Accepts `generator' which can generate voxel terrain as
    // needed to fill the chunks underlying VoxelData.
    VoxelData(const std::shared_ptr<VoxelDataGenerator> &generator,
              unsigned chunkSize);
    
    // No default constructor.
    VoxelData() = delete;
    
    // Destructor is just the default.
    ~VoxelData() = default;
    
    // Get the object corresponding to the specified point in space.
    // Note that each point in space corresponds to exactly one cell.
    // Throws an exception if the point is not within this grid.
    // Throws an exception if the chunk is not already present.
    const Voxel& get(const glm::vec3 &p) const override;
    
    // Get the cell associated with the given cell coordinates.
    // Each cell in the grid can be addressed by cell coordinates which uniquely
    // identify that cell.
    // See also gridResolution() and cellCoordsAtPoint().
    const Voxel& get(const glm::ivec3 &cellCoords) const override;
    
    // Each point in space corresponds to exactly one cell. Get the (mutable)
    // object. Note that this will retrieve the chunk contents from the
    // generator if the the chunk is not already present.
    Voxel& mutableReference(const glm::vec3 &p) override;
    
    // Get the (mutable) object associated with the given cell coordinates.
    // Each cell in the grid can be addressed by cell coordinates which uniquely
    // identify that cell.
    // See also gridResolution() and cellCoordsAtPoint().
    Voxel& mutableReference(const glm::ivec3 &cellCoords) override;
    
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
