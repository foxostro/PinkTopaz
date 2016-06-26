//
//  Actor.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 6/25/16.
//
//

#include "Actor.hpp"

namespace PinkTopaz {
    
    Actor::Actor() : shouldShutdown(false)
    {
        // Nothing to do
    }
    
    Actor::~Actor()
    {
        // Nothing to do
    }
    
    void Actor::start()
    {
        thr = std::unique_ptr<std::thread>(new std::thread([this]{ this->run(); }));
    }
    
    void Actor::join()
    {
        thr->join();
    }
    
    void Actor::setShouldShutdown()
    {
        shouldShutdown = true;
    }
    
    void Actor::run()
    {
        preLoop();
        
        while(!shouldShutdown)
        {
            pump();
        }
        
        postLoop();
    }
    
} // namespace PinkTopaz
