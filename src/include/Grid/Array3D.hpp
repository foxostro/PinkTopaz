//
//  Array3D.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 9/26/17.
//
//

#ifndef Array3D_hpp
#define Array3D_hpp

#include "BaseArray3D.hpp"
#include <vector>

template <typename CellType>
using Array3D = BaseArray3D<CellType, std::vector<CellType>>;

#endif /* Array3D_hpp */
