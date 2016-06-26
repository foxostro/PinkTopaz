//
//  Renderer.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 6/25/16.
//
//

#ifndef Renderer_hpp
#define Renderer_hpp

#include <SDL2/SDL.h>
#include <thread>
#include <atomic>
#include <queue>

namespace PinkTopaz::Renderer {
    
    // Generic actor object. Accepts messages and commands which are queued and processed in a separate thread.
    // The message queue must be implemented in the child classes. The API for this is very application-specific
    // so we cannot define this API in the base class. Instead, the Actor base class exclusively handles the creation
    // and management of the actor thread.
    class Actor
    {
    private:
        std::unique_ptr<std::thread> thr;
        std::atomic_bool shouldShutdown;
        
        // The thread runs this method.
        void run();
        
    protected:
        // Called on the actor thread before entering the loop.
        virtual void preLoop() = 0;
        
        // A single iteration of the actor message handling loop.
        virtual void pump() = 0;
        
        // Called on the actor thread after exiting the loop.
        virtual void postLoop() = 0;
        
    public:
        // Constructor. Starts the thread.
        Actor();
        
        // Destructor. Ensures clean shutdown of the thread.
        virtual ~Actor();
        
        // Signals that the thread should shutdown and terminate.
        void setShouldShutdown();
        
        void start();
        void finish();
    };
    
    // Actor which handles the task of Renderering.
    // This one does OpenGL renderering. This actor and associated objects are actually the only parts of the engine
    // that are permitted to access the OpenGL API.
    class Renderer : public Actor
    {
    private:
        SDL_Window *window;

    protected:
        // Called on the actor thread before entering the loop.
        virtual void preLoop();
        
        // A single iteration of the actor message handling loop.
        virtual void pump();
        
        // Called on the actor thread after exiting the loop.
        virtual void postLoop();
        
    public:
        Renderer(SDL_Window *window);
    };

} // namespace PinkTopaz::Renderer

#endif /* Renderer_hpp */
