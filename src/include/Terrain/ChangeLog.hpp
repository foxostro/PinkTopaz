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

// Ordered list of changes made to a TransactedVoxelData during a transaction.
class ChangeLog
{
public:
    struct Change
    {
        std::string type;
        AABB affectedRegion;
    };
    
    using container_type = std::vector<Change>;
    using iterator = container_type::iterator;
    using const_iterator = container_type::const_iterator;
    
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
    
    inline iterator begin() { return _changes.begin(); }
    inline const_iterator begin() const { return _changes.begin(); }
    inline iterator end() { return _changes.end(); }
    inline const_iterator end() const { return _changes.end(); }
    
private:
    container_type _changes;
};

#endif /* ChangeLog_hpp */
