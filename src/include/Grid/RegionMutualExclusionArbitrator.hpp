//
//  RegionMutualExclusionArbitrator.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 9/27/17.
//
//

#ifndef RegionMutualExclusionArbitrator_hpp
#define RegionMutualExclusionArbitrator_hpp

#include "Grid/Array3D.hpp"
#include <mutex>

// Assists in locking a region of space when accessing a grid concurrently.
class RegionMutualExclusionArbitrator
{
public:
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
    
    // Default Destructor
    ~RegionMutualExclusionArbitrator() = default;
    
    // No default constructor.
    RegionMutualExclusionArbitrator() = delete;
    
    // Constructor. The space is divided into a grid of cells where each cell
    // is associated with a data element. The space is also divided into a grid
    // of cells where each cell has a lock to protect the data grid from
    // concurrent access. Though, the two grids may have a different resolution
    // such that each lock protects a region of space instead of a single data
    // element.
    // boundingBox -- The region of space being managed.
    // gridResolution -- The resolution of the lock grid.
    RegionMutualExclusionArbitrator(const AABB &boundingBox,
                                    const glm::ivec3 &gridResolution)
     : _arrayLocks(boundingBox, gridResolution)
    {}
    
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
    
private:
    // Locks for the array contents.
    // We use a shared_ptr here because there is no copy-assignment operator for
    // std::mutex.
    mutable Array3D<std::mutex> _arrayLocks;
};

#endif /* RegionMutualExclusionArbitrator_hpp */
