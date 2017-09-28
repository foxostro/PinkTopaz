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
    using MutexVector = std::vector<std::reference_wrapper<std::mutex>>;
    
    // An ordered collection of locks which need to be acquired simultaneously.
    // Satisfies BasicLockable and can be used with std::lock_guard.
    class RegionMutex
    {
    public:
        RegionMutex(MutexVector &&theLocks) : _mutexes(std::move(theLocks)) {}
        
        void lock()
        {
            for (std::mutex &mutex : _mutexes) {
                mutex.lock();
            }
        }
        
        void unlock()
        {
            for (std::mutex &mutex : _mutexes) {
                mutex.unlock();
            }
        }
        
    private:
        const MutexVector _mutexes;
    };
    
    ~RegionMutualExclusionArbitrator() = default;
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
     : _arrayMutexes(boundingBox, gridResolution)
    {}
    
    // Gets a region lock to protect the specified region of space.
    template<typename RegionType>
    RegionMutex getMutex(const RegionType &region) const
    {
        MutexVector mutexes;
        
        _arrayMutexes.mutableForEachCell(region, [&](const AABB &cell,
                                                     Morton3 index,
                                                     std::mutex &mutex){
            mutexes.push_back(std::reference_wrapper<std::mutex>(mutex));
        });
        
        return RegionMutex(std::move(mutexes));
    }
    
private:
    // Locks for the array contents.
    // We use a shared_ptr here because there is no copy-assignment operator for
    // std::mutex.
    mutable Array3D<std::mutex> _arrayMutexes;
};

#endif /* RegionMutualExclusionArbitrator_hpp */
