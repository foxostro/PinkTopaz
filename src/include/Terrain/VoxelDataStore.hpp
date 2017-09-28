//
//  VoxelDataStore.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/16/17.
//
//

#ifndef VoxelDataStore_hpp
#define VoxelDataStore_hpp

#include <boost/signals2.hpp>
#include <functional>
#include <vector>
#include <mutex>

#include "Grid/GridIndexer.hpp"
#include "Grid/Array3D.hpp"
#include "Grid/ChangeLog.hpp"
#include "VoxelData.hpp"

// Wraps a VoxelData object. Transactions on VoxelDataStore protect this voxel
// data to allow concurrent readers and writers.
class VoxelDataStore : public GridIndexer
{
public:
    using Reader = std::function<void(const Array3D<Voxel> &data)>;
    using Writer = std::function<ChangeLog(VoxelData &data)>;
    
    // A vector which contains references to mutexes. This is used to pass
    // around references to locks in the locks array itself.
    using LockVector = std::vector<std::reference_wrapper<std::mutex>>;
    
    // An ordered collection of locks which need to be acquired simultaneously.
    // Acquires locks in the constructor and releases in the destructor.
    // This is used for exception-safe locking and unlocking of a region of the
    // grid.
    class LockSet
    {
    public:
        LockSet(LockVector &&theLocks) : locks(theLocks)
        {
            for (auto iter = locks.begin(); iter != locks.end(); ++iter) {
                std::mutex &lock = *iter;
                lock.lock();
            }
        }
        
        ~LockSet()
        {
            for (auto iter = locks.rbegin(); iter != locks.rend(); ++iter) {
                std::mutex &lock = *iter;
                lock.unlock();
            }
        }
        
    private:
        const LockVector locks;
    };
    
    // Destructor is just the default.
    virtual ~VoxelDataStore() = default;
    
    // No default constructor.
    VoxelDataStore() = delete;
    
    // Constructor. Accepts a VoxelData object which contains the actual voxels
    // Transactions on VoxelDataStore protect this voxel data during concurrent
    // access.
    VoxelDataStore(std::unique_ptr<VoxelData> &&voxelData, unsigned chunkSize)
     : GridIndexer(voxelData->boundingBox(), voxelData->gridResolution()),
       _arrayLocks(voxelData->boundingBox(), voxelData->gridResolution() / (int)chunkSize),
       _array(std::move(voxelData))
    {}
    
    // Perform an atomic transaction as a "reader" with read-only access to the
    // underlying data in the specified region.
    // region -- The region we will be reading from.
    // fn -- Closure which will be doing the reading.
    void readerTransaction(const AABB &region, const Reader &fn) const
    {
        // TODO: The call to copy() will serially fetch the underling voxels if they
        // were not present. This will lead to other reader transactions being
        // blocked waiting for those voxels even if they might be able to make
        // progress if one or two of those voxel chunks became ready.
        // Instead, we should fire off a group of asynchronous tasks right here
        // where each task will take the lock on a single voxel chunk and fetch that
        // chunk. Once all tasks in the group has completed then we grab the entire
        // lockset and proceed with the copy as before.
        
        LockSet locks(locksForRegion(region));
        auto rawPointer = (VoxelData *)_array.get();
        assert(rawPointer != nullptr);
        const Array3D<Voxel> data = rawPointer->copy(region);
        fn(data);
    }
    
    // Perform an atomic transaction as a "writer" with read-write access to
    // the underlying voxel data in the specified region. It is the
    // responsibility of the caller to provide a closure which will update the
    // change log accordingly.
    // region -- The region we will be writing to.
    // fn -- Closure which will be doing the writing.
    void writerTransaction(const AABB &region, const Writer &fn)
    {
        ChangeLog changeLog;
        {
            LockSet locks(locksForRegion(region));
            VoxelData &voxels = *_array;
            changeLog = fn(voxels);
        }
        onWriterTransaction(changeLog);
    }
    
    // This signal fires when a "writer" transaction finishes. This provides the
    // opportunity to respond to changes to data. For example, by rebuilding
    // meshes associated with underlying voxel data.
    boost::signals2::signal<void (const ChangeLog &changeLog)> onWriterTransaction;
    
protected:
    // Locks for the array contents.
    // We use a shared_ptr here because there is no copy-assignment operator for
    // std::mutex.
    mutable Array3D<std::mutex> _arrayLocks;
    
    // An array for which we intend to provide concurrent access.
    std::unique_ptr<VoxelData> _array;
    
    // Returns an ordered list of the locks protecting the specified region.
    template<typename RegionType>
    LockVector locksForRegion(const RegionType &region) const
    {
        LockVector locks;
        
        _arrayLocks.mutableForEachCell(region, [&](const AABB &cell,
                                                   Morton3 index,
                                                   std::mutex &lock){
            locks.push_back(std::reference_wrapper<std::mutex>(lock));
        });
        
        return locks;
    }
};

#endif /* VoxelDataStore_hpp */
