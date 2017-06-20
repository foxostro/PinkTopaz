//
//  ConcurrentGridMutable.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 6/19/17.
//
//

#ifndef ConcurrentGridMutable_hpp
#define ConcurrentGridMutable_hpp

#include "GridAddressable.hpp"

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
class ConcurrentGridMutable
{
public:
    typedef std::function<void(const GridAddressable<ElementType> &data)> Reader;
    typedef std::function<ChangeLog(GridMutable<ElementType> &data)> Writer;
    
    // A vector which contains references to mutexes. This is used to pass
    // around references to locks in the locks array itself.
    typedef std::vector<std::shared_ptr<std::mutex>> LockVector;
    
    // An ordered collection of locks which need to be acquired simultaneously.
    // Acquires locks in the constructor and releases in the destructor.
    // This is used for exception-safe locking and unlocking of a region of the
    // grid.
    class LockSet
    {
    public:
        LockSet(LockVector &&theLocks) : locks(theLocks)
        {
            for (auto &lock : locks) {
                lock->lock();
            }
        }
        
        ~LockSet()
        {
            for (auto iter = locks.rbegin(); iter != locks.rend(); ++iter) {
                auto &lock = *iter;
                lock->unlock();
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
     : _cellDimensions(array->cellDimensions()),
       _boundingBox(array->boundingBox()),
       _gridResolution(array->gridResolution()),
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
    // r -- The region we will be reading from.
    // fn -- Closure which will be doing the reading.
    inline void readerTransaction(const AABB &region, const Reader &fn) const
    {
        LockSet locks(locksForRegion(region));
        fn(*_array); // AFOX_TODO: What if we want to copy the region to an Array3D?
    }
    
    // Perform an atomic transaction as a "writer" with read-write access to
    // the underlying voxel data in the specified region. It is the
    // responsibility of the caller to provide a closure which will update the
    // change log accordingly.
    // region -- The region we will be writing to.
    // fn -- Closure which will be doing the writing.
    inline void writerTransaction(const AABB &region, const Writer &fn)
    {
        LockSet locks(locksForRegion(region));
        ChangeLog changeLog = fn(*_array);
        onWriterTransaction(changeLog);
    }
    
    // This signal fires when a "writer" transaction finishes. This provides the
    // opportunity to respond to changes to data. For example, by rebuilding
    // meshes associated with underlying voxel data.
    boost::signals2::signal<void (const ChangeLog &changeLog)> onWriterTransaction;
    
    // Gets the dimensions of a single cell in the grid.
    // Note that cells in the grid are always the same size.
    inline glm::vec3 cellDimensions() const
    {
        return _cellDimensions;
    }
    
    // Gets the region for which the grid is defined.
    // Accesses to points outside this box is not permitted.
    inline AABB boundingBox() const
    {
        return _boundingBox;
    }
    
    // Gets the number of cells along each axis within the valid region.
    inline glm::ivec3 gridResolution() const
    {
        return _gridResolution;
    }
    
private:
    const glm::vec3 _cellDimensions;
    const AABB _boundingBox;
    const glm::ivec3 _gridResolution;
    
    // Locks for the array contents.
    // We use a shared_ptr here because there is no copy-assignment operator for
    // std::mutex.
    mutable Array3D<std::shared_ptr<std::mutex>> _arrayLocks;
    
    // An array for which we intend to provide concurrent access.
    std::unique_ptr<GridMutable<ElementType>> _array;
    
    // Returns an ordered list of the locks protecting the specified region.
    // Throws an exception if the region is not contained within the grid.
    LockVector locksForRegion(const AABB &region) const
    {
        if (!_array->inbounds(region)) {
            throw OutOfBoundsException();
        }
        
        if (!_arrayLocks.inbounds(region)) {
            throw OutOfBoundsException();
        }
        
        LockVector locks;
        
        _arrayLocks.mutableForEachCell(region, [&](const AABB &cell,
                                                   Morton3 index,
                                                   std::shared_ptr<std::mutex> &lock){
            if (!lock) {
                lock = std::make_shared<std::mutex>();
            }
            locks.push_back(lock);
        });
        
        return locks;
    }
};

#endif /* ConcurrentGridMutable_hpp */
