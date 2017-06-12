//
//  UniqueName.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 3/10/17.
//
//

#ifndef UniqueName_hpp
#define UniqueName_hpp

// Preprocessor magic courtesy of StackOverflow: <http://stackoverflow.com/a/17624752/2403342>
#define PP_CAT(a, b) PP_CAT_I(a, b)
#define PP_CAT_I(a, b) PP_CAT_II(~, a ## b)
#define PP_CAT_II(p, res) res
#define UNIQUE_NAME(base) PP_CAT(base, __COUNTER__)

#endif /* UniqueName_hpp */
