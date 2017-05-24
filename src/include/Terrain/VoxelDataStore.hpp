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
#include "ChangeLog.hpp"
#include <shared_mutex>
#include <functional>
#include <boost/signals2.hpp>

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
    // underlying voxel data in the specified region.
    void readerTransaction(const AABB &region, const std::function<void(const GridAddressable<Voxel> &voxels)> &fn) const;
    
    // Perform a transaction as a "writer" where we have read-write access to
    // the underlying voxel data in the specified region. It is the
    // responsibility of the caller to provide a closure which will update the
    // change log accordingly.
    void writerTransaction(const AABB &region, const std::function<ChangeLog(GridMutable<Voxel> &voxels)> &fn);
    
    // This signal fires when a voxel data "writer" transaction finishes and
    // provides the opportunity to respond to changes to voxel data. For
    // example, by rebuilding meshes.
    // TODO: Pass a change log object as a parameter here.
    boost::signals2::signal<void (const ChangeLog &changeLog)> voxelDataChanged;
    
private:
    mutable std::shared_mutex _mutex;
    ChangeLog _changeLog;
    VoxelData _data;
};

#endif /* VoxelDataStore_hpp */
