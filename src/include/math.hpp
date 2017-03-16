//
//  math.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 3/14/16.
//
//

#ifndef math_hpp
#define math_hpp

namespace PinkTopaz {

	template<typename T>
    static inline const T& clamp(const T &value, const T &min, const T &max)
    {
        return std::min(std::max(value, min), max);
    }

} // namespace PinkTopaz

#endif /* math_hpp */
