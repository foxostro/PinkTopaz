//
//  AutoreleasePool.mm
//  PinkTopaz
//
//  Created by Andrew Fox on 5/5/18.
//
//

#import <Foundation/Foundation.h>
#import "AutoreleasePool.hpp"

AutoreleasePool::AutoreleasePool()
{
    BOOL isMultiThreaded = [NSThread isMultiThreaded];
    if (!isMultiThreaded) {
        // NSAutoreleasePool documentation states that if we're creating our
        // own threads outside of Cocoa (which we are) then we need to put
        // Cocoa into multithreaded mode by detaching at least one thread using
        // NSThread.
        // See also: <https://developer.apple.com/documentation/foundation/nsautoreleasepool>
        [[NSThread new] start];
        assert([NSThread isMultiThreaded]);
    }
    
    NSAutoreleasePool *pool = [NSAutoreleasePool new];
    _context = (void *)pool;
}

AutoreleasePool::~AutoreleasePool()
{
    NSAutoreleasePool *pool = (NSAutoreleasePool *)_context;
    [pool release];
}
