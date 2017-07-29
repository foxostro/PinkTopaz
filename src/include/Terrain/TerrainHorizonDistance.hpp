//
//  TerrainHorizonDistance.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 7/29/17.
//
//

#ifndef TerrainHorizonDistance_hpp
#define TerrainHorizonDistance_hpp

#include <entityx/entityx.h> // for TimeDelta
#include <shared_mutex>

// The terrain horizon scrolls away from the camera as chunks are loaded.
class TerrainHorizonDistance
{
public:
    TerrainHorizonDistance()
     : _targetHorizonDistance(STEP),
       _horizonDistance(STEP)
    {}
    
    // Gets the horizon distance. Takes the lock so this is thread-safe.
    inline float get() const
    {
        std::shared_lock<std::shared_mutex> lock(_lockHorizonDistance);
        return _horizonDistance;
    }
    
    // Increments the horizon distance by the step.
    // Takes the lock so this is thread-safe.
    inline float increment_clamp(float max)
    {
        std::unique_lock<std::shared_mutex> lock(_lockHorizonDistance);
        _targetHorizonDistance += STEP;
        _targetHorizonDistance = std::min(_targetHorizonDistance, max);
        return _targetHorizonDistance;
    }
    
    // The horizon distance moves away smoothly over time.
    inline void update(entityx::TimeDelta dt)
    {
        std::unique_lock<std::shared_mutex> lock(_lockHorizonDistance);
        _horizonDistance = std::min((float)(_horizonDistance + dt * STEP_PER_MS), _targetHorizonDistance);
    }
    
private:
    static constexpr float STEP = 16.0f;
    static constexpr float STEP_PER_MS = 2 * STEP / 1000.0f;
    mutable std::shared_mutex _lockHorizonDistance;
    float _targetHorizonDistance;
    float _horizonDistance;
};

#endif /* TerrainHorizonDistance_hpp */
