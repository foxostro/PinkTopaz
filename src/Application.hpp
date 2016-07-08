//
//  Application.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 6/25/16.
//
//

#ifndef Application_hpp
#define Application_hpp

#include "SDL.h"
#include "opengl.hpp"
#include "Shader.hpp"
#include <memory>

namespace PinkTopaz {
    
    // The central game loop, basically.
    class Application
    {
    public:
        Application();
        ~Application();

        // Runs the game loop on the current thread and returns when the game has ended.
        void run();
        
    private:
        void windowSizeChanged(int windowWidth, int windowHeight, std::shared_ptr<Shader> &shader);
        void inner();
        
        SDL_Window *_window;
    };

} // namespace PinkTopaz

#endif /* Application_hpp */
