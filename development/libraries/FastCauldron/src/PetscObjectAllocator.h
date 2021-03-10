//                                                                      
// Copyright (C) 2015 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 
#ifndef FASTCAULDRON__PETSC_OBJECT_ALLOCATOR__H
#define FASTCAULDRON__PETSC_OBJECT_ALLOCATOR__H

// Access to PETSc library
#include "petsc.h"

// Access to fastcauldron application code.
#include "ComputationalDomain.h"

/// \brief Class to simplify allocation of PETSc matrices and vector.
class PetscObjectAllocator {

public :

   /// \brief Allocate a PETSc matrix using the computational domain.
   ///
   /// The matrix allocated will have the non-zeros per row allocated.
   static Mat allocateMatrix ( const ComputationalDomain& domain, 
                               const bool                 verbose = false );

   /// \brief Allocate a PETSc vector using the computational domain.
   static Vec allocateVector ( const ComputationalDomain& domain );

};


#endif // FASTCAULDRON__PETSC_OBJECT_ALLOCATOR__H
