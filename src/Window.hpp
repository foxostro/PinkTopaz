//
//  Renderer.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 6/25/16.
//
//

#ifndef Window_hpp
#define Window_hpp

namespace PinkTopaz {
    
    // Owns the SDL window and handles associated window events.
    class Window
    {
    public:
        Window();
        ~Window();

        // Runs the window event loop on the current thread and returns when the window has closed.
        void run();
    };

} // namespace PinkTopaz

#endif /* Window_hpp */
