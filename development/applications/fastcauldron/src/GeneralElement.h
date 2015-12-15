//                                                                      
// Copyright (C) 2015 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 
#ifndef FASTCAULDRON__GENERAL_ELEMENT__H
#define FASTCAULDRON__GENERAL_ELEMENT__H


// Access to PETSc library
#include "petsc.h"

// Access to fastcauldron application code.
#include "BaseElement.h"
#include "BoundaryId.h"
#include "ElementDofsMixin.h"
#include "globaldefs.h"


/// \brief An element that is a part of a ComputationalDomain.
///
/// This element adds general dof counting to the base element.
typedef BaseElement<ElementDofsMixin> GeneralElement;


#endif // FASTCAULDRON__GENERAL_ELEMENT__H
