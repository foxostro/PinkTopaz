//
//  VoxelDataStore.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/16/17.
//
//

#ifndef VoxelDataStore_hpp
#define VoxelDataStore_hpp

#include "VoxelData.hpp"
#include <shared_mutex>
#include <functional>

// A block of voxels in space with locking and expectation of concurrent access.
class VoxelDataStore
{
public:
    // Constructor. Accepts a bounding box decribing the region of space
    // this block of voxels represents. The space is divided into voxel
    // cells at a resolution described by `resolution.' That is, there are
    // `resolution.x' voxel cells along the X-axis, `resolution.y' voxel
    // cells along the Y-axis, and `resolution.z' cells along the Z-axis.
    VoxelDataStore(const AABB &box, const glm::ivec3 &resolution);
    
    // No default constructor.
    VoxelDataStore() = delete;
    
    // Copy constructor is just the default.
    VoxelDataStore(const VoxelDataStore &voxels) = default;
    
    // Move constructor is just the default.
    VoxelDataStore(VoxelDataStore &&voxels) = default;
    
    // Destructor is just the default.
    ~VoxelDataStore() = default;
    
    // Perform a transaction as a "reader" where we have read-only access to the
    // underlying voxel data.
    void readerTransaction(const std::function<void(const VoxelData &voxels)> &fn) const;
    
    // Perform a transaction as a "writer" where we have read-write access to
    // the underlying voxel data.
    void writerTransaction(const std::function<void(VoxelData &voxels)> &fn);
    
private:
    mutable std::shared_mutex _mutex;
    VoxelData _data;
};

#endif /* VoxelDataStore_hpp */
