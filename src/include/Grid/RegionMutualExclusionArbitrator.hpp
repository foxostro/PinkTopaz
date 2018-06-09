//
//  RegionMutualExclusionArbitrator.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 9/27/17.
//
//

#ifndef RegionMutualExclusionArbitrator_hpp
#define RegionMutualExclusionArbitrator_hpp

#include "AABB.hpp"
#include "GridIndexer.hpp" // for doBoxesIntersect()
#include <mutex>
#include <condition_variable>
#include <list>

// Assists in locking a region of space when accessing a grid concurrently.
class RegionMutualExclusionArbitrator
{
public:
    struct ReaderToken{ std::list<AABB>::iterator iter; };
    struct WriterToken{ std::list<AABB>::iterator iter; };
    
    // Represents a region of space for which we need mutually exclusive access.
    // Supports either a reader or a writer depending on TokenType.
    // Satisfies BasicLockable and can be used with std::scoped_lock.
    template<typename TokenType>
    class RegionMutex
    {
    public:
        RegionMutex(RegionMutualExclusionArbitrator &arbitrator,
                    const AABB &region)
         : _arbitrator(arbitrator), _region(region)
        {}
        
        void lock()
        {
            _arbitrator.lock(_token, _region);
        }
        
        void unlock()
        {
            _arbitrator.unlock(_token);
        }
        
    private:
        RegionMutualExclusionArbitrator &_arbitrator;
        TokenType _token;
        const AABB _region;
    };
    
    ~RegionMutualExclusionArbitrator() = default;
    RegionMutualExclusionArbitrator() = default;
    
    // Returns a mutex-like object which protects the specified region of space
    // for reading.
    auto readerMutex(const AABB &region)
    {
        return RegionMutex<ReaderToken>(*this, region);
    }
    
    // Returns a mutex-like object which protects the specified region of space
    // for writing.
    auto writerMutex(const AABB &region)
    {
        return RegionMutex<WriterToken>(*this, region);
    }
    
private:
    std::mutex _mutex;
    std::condition_variable _cvar;
    std::list<AABB> _readerTransactions;
    std::list<AABB> _writerTransactions;
    
    // Lock a region of space for reading.
    // Returns a token which can be used to unlock it.
    void lock(ReaderToken &token, const AABB &region)
    {
        std::unique_lock<std::mutex> lock(_mutex);
        _cvar.wait(lock, [=]{
            return readerCanEnter(region);
        });
        _readerTransactions.push_front(region);
        token.iter = _readerTransactions.begin();
    }
    
    // Lock a region of space for reading.
    // Returns a token which can be used to unlock it.
    void lock(WriterToken &token, const AABB &region)
    {
        std::unique_lock<std::mutex> lock(_mutex);
        _cvar.wait(lock, [=]{
            return writerCanEnter(region);
        });
        _writerTransactions.push_front(region);
        token.iter = _writerTransactions.begin();
    }
    
    // Unlock a region preiously locked for reading.
    void unlock(ReaderToken &token)
    {
        std::scoped_lock lock(_mutex);
        _readerTransactions.erase(token.iter);
        token.iter = _readerTransactions.end();
        _cvar.notify_all();
    }
    
    // Unlock a region preiously locked for writing.
    void unlock(WriterToken &token)
    {
        std::scoped_lock lock(_mutex);
        _writerTransactions.erase(token.iter);
        token.iter = _writerTransactions.end();
        _cvar.notify_all();
    }
    
    // Test whether the specified region overlaps a region of any existing
    // writer transaction.
    bool readerCanEnter(const AABB &desiredRegion) const
    {
        // A reader can enter if it will not collide with any current writer.
        for (const AABB &region : _writerTransactions) {
            if (doBoxesIntersect(desiredRegion, region)) {
                return false;
            }
        }
        return true;
    }
    
    // Test whether the specified region overlaps a region of any existing
    // reader or writer transaction.
    bool writerCanEnter(const AABB &desiredRegion) const
    {
        // A writer can enter if it will not collide with any current
        // writer or reader transaction.
        for (const AABB &region : _writerTransactions) {
            if (doBoxesIntersect(desiredRegion, region)) {
                return false;
            }
        }
        for (const AABB &region : _readerTransactions) {
            if (doBoxesIntersect(desiredRegion, region)) {
                return false;
            }
        }
        return true;
    }
};

#endif /* RegionMutualExclusionArbitrator_hpp */
