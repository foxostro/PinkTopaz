//
//  SparseArray3D.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/16/17.
//
//

#ifndef SparseArray3D_hpp
#define SparseArray3D_hpp

#include "BaseArray3D.hpp"
#include "SparseVector.hpp"

template <typename CellType>
using SparseArray3D = BaseArray3D<CellType, SparseVector<CellType>>;

#endif /* SparseArray3D_hpp */
