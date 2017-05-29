//
//  UnitTestDetector.m
//  PinkTopaz
//
//  Created by Andrew Fox on 5/29/17.
//
//

#import <Foundation/Foundation.h>
#import "UnitTestDetector.h"

// Adapted from <https://stackoverflow.com/a/44023436/2403342>
bool areWeBeingUnitTested()
{
    Class testProbeClass;
    testProbeClass = NSClassFromString(@"XCTestProbe");
    return (testProbeClass != nil);
}
