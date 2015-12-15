//------------------------------------------------------------//

#ifndef __Auxiliary_Operations_HH__
#define __Auxiliary_Operations_HH__

//------------------------------------------------------------//

#include "PetscVectors.h"

//------------------------------------------------------------//

namespace Basin_Modelling {

  void Copy_Array_To_Top ( const double**        Source,
                           const int             Top_Index,
                           const PETSC_2D_Array& Valid_Node,
                                 PETSC_3D_Array& Destination );

} // end namespace Basin_Modelling

//------------------------------------------------------------//

#endif // __Auxiliary_Operations_HH__

//------------------------------------------------------------//
