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
#include <condition_variable>
#include <list>

// Assists in locking a region of space when accessing a grid concurrently.
class RegionMutualExclusionArbitrator
{
public:
    using Token = std::list<AABB>::iterator;
    
    // Represents a region of space for which we need mutually exclusive access.
    // Satisfies BasicLockable and can be used with std::lock_guard.
    class RegionMutex
    {
    public:
        RegionMutex(RegionMutualExclusionArbitrator &arbitrator,
                    const AABB &region)
         : _arbitrator(arbitrator), _region(region)
        {}
        
        void lock()
        {
            _token = _arbitrator.lock(_region);
        }
        
        void unlock()
        {
            _arbitrator.unlock(_token);
        }
        
    private:
        RegionMutualExclusionArbitrator &_arbitrator;
        Token _token;
        const AABB _region;
    };
    
    ~RegionMutualExclusionArbitrator() = default;
    RegionMutualExclusionArbitrator() = default;
    
    // Returns a mutex-like object which protects the specified region of space.
    RegionMutex getMutex(const AABB &region)
    {
        return RegionMutex(*this, region);
    }
    
private:
    std::mutex _mutex;
    std::condition_variable _cvar;
    std::list<AABB> _transactions;
    
    // Lock a region of space. Returns a token which can be used to unlock it.
    Token lock(const AABB &region)
    {
        std::unique_lock<std::mutex> lock(_mutex);
        _cvar.wait(lock, [=]{
            return !regionIsInUse(region);
        });
        _transactions.push_front(region);
        return _transactions.begin();
    }
    
    // Unlock a preiously locked region.
    void unlock(const Token &token)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _transactions.erase(token);
        _cvar.notify_all();
    }
    
    // Test whether the specified region overlaps a region of any existing
    // transaction.
    bool regionIsInUse(const AABB &desiredRegion) const
    {
        for (const AABB &region : _transactions) {
            if (doBoxesIntersect(desiredRegion, region)) {
                return true;
            }
        }
        return false;
    }
};

#endif /* RegionMutualExclusionArbitrator_hpp */
