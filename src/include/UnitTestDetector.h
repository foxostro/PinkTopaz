//
//  UnitTestDetector.h
//  PinkTopaz
//
//  Created by Andrew Fox on 5/29/17.
//
//

#ifndef UnitTestDetector_h
#define UnitTestDetector_h

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h> // We need to include this when building with Visual Studio.

bool areWeBeingUnitTested();

#ifdef __cplusplus
}
#endif

#endif /* UnitTestDetector_h */
