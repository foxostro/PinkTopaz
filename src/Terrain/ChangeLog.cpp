//
//  ChangeLog.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/17/17.
//
//

#include "Terrain/ChangeLog.hpp"

ChangeLog ChangeLog::make(const std::string &type, const AABB &affectedRegion)
{
    ChangeLog changeLog;
    changeLog.add(type, affectedRegion);
    return changeLog;
}

ChangeLog& ChangeLog::operator=(const ChangeLog &otherChangeLog)
{
    _changes = otherChangeLog._changes;
    return *this;
}

void ChangeLog::add(const Change &change)
{
    _changes.push_back(change);
}
