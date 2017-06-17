//
//  VoxelData.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 3/13/16.
//
//

#ifndef VoxelData_hpp
#define VoxelData_hpp

#include "Array3D.hpp"
#include "Voxel.hpp"
#include "VoxelDataGenerator.hpp"
#include <experimental/optional>
#include <mutex>

// A block of voxels in space.
class VoxelData : public GridMutable<Voxel>
{
public:
    typedef Array3D<Voxel> Chunk;
    typedef typename std::experimental::optional<Chunk> MaybeChunk;
    
    // Constructor. Accepts `generator' which can generate voxel terrain as
    // needed to fill the chunks underlying VoxelData.
    VoxelData(const VoxelDataGenerator &generator);
    
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
    
    // Each point in space corresponds to exactly one cell. Set the object.
    void set(const glm::vec3 &p, const Voxel &object) override;
    
    // Sets the cell associated with the given cell coordinates.
    // Each cell in the grid can be addressed by cell coordinates which uniquely
    // identify that cell.
    // See also gridResolution() and cellCoordsAtPoint().
    void set(const glm::ivec3 &cellCoords, const Voxel &object) override;
    
    // Gets the dimensions of a single cell. (All cells are the same size.)
    glm::vec3 cellDimensions() const override;
    
    // Gets the region for which the grid is defined.
    AABB boundingBox() const override;
    
    // Gets the number of cells along each axis within the valid region.
    glm::ivec3 gridResolution() const override;
    
private:
    static constexpr int CHUNK_SIZE = 32;
    
    // Provides voxel data for regions of space where none is available yet.
    const VoxelDataGenerator &_generator;
    
    // The voxel grid is broken into chunks where each chunk is a fixed-size
    // grid of voxels.
    mutable std::mutex _lockChunks;
    mutable Array3D<MaybeChunk> _chunks;
    
    // Fetches the chunk at the specified point in space `p'. This may create
    // the chunk. If so, it is filled using the generator.
    MaybeChunk& chunkAtPoint(const glm::vec3 &p) const;
};

#endif /* VoxelData_hpp */
