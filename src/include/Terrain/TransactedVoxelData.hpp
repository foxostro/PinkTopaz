//
//  TransactedVoxelData.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/17/18.
//
//

#ifndef ConcurrentVoxelData_hpp
#define ConcurrentVoxelData_hpp

#include <boost/signals2.hpp>
#include <functional>
#include <memory>

#include "Grid/GridIndexer.hpp"
#include "Grid/Array3D.hpp"
#include "Grid/RegionMutualExclusionArbitrator.hpp"
#include "Terrain/Voxel.hpp"
#include "Terrain/TerrainOperation.hpp"
#include "Terrain/VoxelData.hpp"

// A block of voxels in space. Concurrent edits are protected by a lock.
class TransactedVoxelData : public GridIndexer
{
public:
    // Default Destructor.
    virtual ~TransactedVoxelData() = default;
    
    // No default constructor.
    TransactedVoxelData() = delete;
    
    // Constructor.
    // source -- The source provides initial voxel data.
    TransactedVoxelData(std::unique_ptr<VoxelData> &&source);
    
    // Perform an atomic transaction as a "reader" with read-only access to the
    // underlying data in the specified region.
    // region -- The region we will be reading from.
    // fn -- Closure which will be doing the reading.
    void readerTransaction(const AABB &region, std::function<void(Array3D<Voxel> &&data)> fn);
    
    // Perform an atomic transaction as a "reader" with read-only access to the
    // underlying data in the specified regions. This is a batch operation which
    // takes a lock, once, and then performs multiple operations.
    // regions -- Collection of regions to read.
    // fn -- Closure which will be doing the reading for each specified region.
    //       Parameters are the index of the region to process and the voxels
    //       associated with that region.
    void readerTransaction(const std::vector<AABB> regions,
                           std::function<void(size_t index, Array3D<Voxel> &&data)> fn);
    
    // Perform an atomic transaction as a "writer" with read-write access to
    // the underlying voxel data in the specified region.
    // operation -- Describes the edits to be made.
    void writerTransaction(TerrainOperation &operation);
    
    // This signal fires when a "writer" transaction finishes. This provides the
    // opportunity to respond to changes to data. For example, by rebuilding
    // meshes associated with underlying voxel data.
    boost::signals2::signal<void (const AABB &affectedRegion)> onWriterTransaction;
    
private:
    RegionMutualExclusionArbitrator _lockArbitrator;
    std::unique_ptr<VoxelData> _source;
};

#endif /* ConcurrentVoxelData_hpp */
