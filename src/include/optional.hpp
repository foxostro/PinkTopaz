//
//  optional.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 9/3/17.
//
//

#ifndef Optional_hpp
#define Optional_hpp

#ifdef _MSC_VER
#include <optional>
using std::optional;
using std::make_optional;
#else
#include <experimental/optional>
using std::experimental::optional;
using std::experimental::make_optional;
#endif

#endif /* Optional_hpp */
