//
//  ConcurrentGridMutable.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 6/19/17.
//
//

#ifndef ConcurrentGridMutable_hpp
#define ConcurrentGridMutable_hpp

#include "Grid/GridAddressable.hpp"
#include "Grid/Array3D.hpp"
#include "Grid/ChangeLog.hpp"

#include <mutex>
#include <vector>
#include <functional>
#include <boost/signals2.hpp>
#include <glm/glm.hpp>

// Wraps an Array3D and provides safe concurrent access to the contents.
// Elements of the array are locked independently, allowing multiple readers
// and writers for non-overlapping regions of space.
// ElementType -- The type of the underlying elements of the grid.
template<typename ElementType>
class ConcurrentGridMutable : public GridIndexer
{
public:
    using Reader = std::function<void(const GridAddressable<ElementType> &data)>;
    using Writer = std::function<ChangeLog(GridMutable<ElementType> &data)>;
    
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
    
    // Constructor. The space is divided into a grid of cells where each cell
    // is associated with a data element. The space is also divided into a grid
    // of cells where each cell has a lock to protect the data grid from
    // concurrent access. Though, the two grids may have a different resolution
    // such that each lock protects a region of space instead of a single data
    // element.
    // array -- The array to wrap.
    // box -- The region of space for which this grid is valid.
    // lockGridResDivisor -- The resolution of the lock grid is determined by
    //                       dividing the array's grid resolution by this.
    ConcurrentGridMutable(std::unique_ptr<GridMutable<ElementType>> &&array,
                          unsigned lockGridResDivisor)
     : GridIndexer(array->boundingBox(), array->gridResolution()),
       _arrayLocks(array->boundingBox(),
                   array->gridResolution() / (int)lockGridResDivisor),
       _array(std::move(array))
    {}
    
    // No default constructor.
    ConcurrentGridMutable() = delete;
    
    // Destructor is just the default.
    virtual ~ConcurrentGridMutable() = default;
    
    // Perform an atomic transaction as a "reader" with read-only access to the
    // underlying data in the specified region.
    // region -- The region we will be reading from.
    // fn -- Closure which will be doing the reading.
    virtual void readerTransaction(const AABB &region, const Reader &fn) const
    {
        LockSet locks(locksForRegion(region));
        fn(*_array);
    }
    
    // Perform an atomic transaction as a "reader" with read-only access to the
    // underlying data in the specified region.
    // region -- The region we will be reading from.
    // fn -- Closure which will be doing the reading.
    virtual void readerTransaction(const Frustum &region, const Reader &fn) const
    {
        LockSet locks(locksForRegion(region));
        fn(*_array);
    }
    
    // Perform an atomic transaction as a "reader" with read-only access to the
    // underlying data in the specified region. Syntactic sugar for the common
    // use-case where the client immediately calls forEachCell inside the
    // transaction.
    template<typename RegionType>
    inline void readerTransaction(const RegionType &region,
                                  const std::function<void (const AABB &cell,
                                                            Morton3 index,
                                                            const ElementType &value)> &fn) const
    {
        readerTransaction(region, [&](const GridAddressable<ElementType> &data){
            data.forEachCell(region, fn);
        });
    }
    
    // Perform an atomic transaction as a "writer" with read-write access to
    // the underlying voxel data in the specified region. It is the
    // responsibility of the caller to provide a closure which will update the
    // change log accordingly.
    // region -- The region we will be writing to.
    // fn -- Closure which will be doing the writing.
    virtual void writerTransaction(const AABB &region, const Writer &fn)
    {
        ChangeLog changeLog;
        {
            LockSet locks(locksForRegion(region));
            changeLog = fn(*_array);
        }
        onWriterTransaction(changeLog);
    }
    
    // Perform an atomic transaction as a "writer" with read-write access to
    // the underlying voxel data in the specified region. It is the
    // responsibility of the caller to provide a closure which will update the
    // change log accordingly.
    // region -- The region we will be writing to.
    // fn -- Closure which will be doing the writing.
    virtual void writerTransaction(const Frustum &region, const Writer &fn)
    {
        ChangeLog changeLog;
        {
            LockSet locks(locksForRegion(region));
            changeLog = fn(*_array);
        }
        onWriterTransaction(changeLog);
    }
    
    // Perform an atomic transaction as a "writer" with read-write access to
    // the underlying voxel data in the specified region. Syntactic sugar for
    // the common use-case where the client immediately calls mutableForEachCell
    // inside the transaction.
    template<typename RegionType>
    inline void writerTransaction(const RegionType &region,
                                  const std::function<void (const AABB &cell,
                                                            Morton3 index,
                                                            ElementType &value)> &fn)
    {
        writerTransaction(region, [&](GridMutable<ElementType> &data){
            data.mutableForEachCell(region, fn);
            
            // Return an empty changelog. So, this call is not appropriate for
            // cases where a real changelog is necessary.
            return ChangeLog();
        });
    }
    
    // This signal fires when a "writer" transaction finishes. This provides the
    // opportunity to respond to changes to data. For example, by rebuilding
    // meshes associated with underlying voxel data.
    boost::signals2::signal<void (const ChangeLog &changeLog)> onWriterTransaction;
    
    // Gets the array for unprotected, raw access. Use carefully.
    inline const std::unique_ptr<GridMutable<ElementType>>& array()
    {
        return _array;
    }
    
protected:
    // Locks for the array contents.
    // We use a shared_ptr here because there is no copy-assignment operator for
    // std::mutex.
    mutable Array3D<std::mutex> _arrayLocks;
    
    // An array for which we intend to provide concurrent access.
    std::unique_ptr<GridMutable<ElementType>> _array;
    
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

#endif /* ConcurrentGridMutable_hpp */
