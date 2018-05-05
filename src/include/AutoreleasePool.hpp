//
//  AutoreleasePool.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/5/18.
//
//

#ifndef AutoreleasePool_hpp
#define AutoreleasePool_hpp

// On macOS, we need to wrap certain operations in an autorelease pool.
// On other platforms, this is a No-op.
class AutoreleasePool
{
public:
    ~AutoreleasePool();
    
    AutoreleasePool();
    
private:
    void *_context;
};

#endif /* AutoreleasePool_hpp */

