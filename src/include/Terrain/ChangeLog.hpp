//
//  ChangeLog.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/17/17.
//
//

#ifndef ChangeLog_hpp
#define ChangeLog_hpp

#include <vector>
#include <string>
#include <glm/vec3.hpp>
#include <AABB.hpp>

// Ordered list of changes made to a VoxelDataStore during a transaction.
class ChangeLog
{
public:
    struct Change
    {
        std::string type;
        AABB affectedRegion;
    };
    
    static ChangeLog make(const std::string &type, const AABB &affectedRegion);
    
    // Default constructor.
    ChangeLog() = default;
    
    // Copy constructor is just the default.
    ChangeLog(const ChangeLog &changeLog) = default;
    
    // Move constructor is just the default.
    ChangeLog(ChangeLog &&changeLog) = default;
    
    // Destructor is just the default.
    ~ChangeLog() = default;
    
    // Copy assignment operator.
    ChangeLog& operator=(const ChangeLog &otherChangeLog);
    
    // Add a change to the log.
    void add(const Change &change);
    
    inline void add(const std::string &type, const AABB &affectedRegion)
    {
        Change change = {type, affectedRegion};
        add(change);
    }
    
    // Gets read-only access to the list of changes.
    inline const std::vector<Change>& getChanges() const { return _changes; }
    
private:
    std::vector<Change> _changes;
};

#endif /* ChangeLog_hpp */
