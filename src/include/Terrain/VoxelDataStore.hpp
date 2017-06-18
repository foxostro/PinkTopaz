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
#include "VoxelDataGenerator.hpp"
#include <shared_mutex>
#include <functional>
#include <boost/signals2.hpp>

// A block of voxels in space with locking and expectation of concurrent access.
class VoxelDataStore : public GridTransactional<Voxel>
{
public:
    typedef std::function<void(const GridAddressable<Voxel> &voxels)> Reader;
    typedef std::function<ChangeLog(GridMutable<Voxel> &voxels)> Writer;
    
    // Constructor.
    VoxelDataStore();
    
    // Destructor is just the default.
    ~VoxelDataStore() = default;
    
    // Perform a transaction as a "reader" where we have read-only access to the
    // underlying voxel data in the specified region.
    void readerTransaction(const AABB &region, const Reader &fn) const override;
    
    // Perform a transaction as a "writer" where we have read-write access to
    // the underlying voxel data in the specified region. It is the
    // responsibility of the caller to provide a closure which will update the
    // change log accordingly.
    void writerTransaction(const AABB &region, const Writer &fn) override;
    
    // Gets the dimensions of a single cell in the grid.
    // Note that cells in the grid are always the same size.
    glm::vec3 cellDimensions() const override;
    
    // Gets the region for which the grid is defined.
    // Accesses to points outside this box is not permitted.
    AABB boundingBox() const override;
    
    // Gets the number of cells along each axis within the valid region.
    glm::ivec3 gridResolution() const override;
    
    // This signal fires when a voxel data "writer" transaction finishes and
    // provides the opportunity to respond to changes to voxel data. For
    // example, by rebuilding meshes.
    boost::signals2::signal<void (const ChangeLog &changeLog)> voxelDataChanged;
    
private:
    VoxelDataGenerator _generator;
    VoxelData _data;
    ChangeLog _changeLog;
    mutable std::shared_mutex _mutex;
};

#endif /* VoxelDataStore_hpp */
