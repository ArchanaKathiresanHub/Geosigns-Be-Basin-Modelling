//
// Copyright (C) 2017 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "FiniteElementArrayTypes.h"

FiniteElementMethod::ArrayOfMatrix3x3::ArrayOfMatrix3x3 ( const unsigned int numberOfMats ) :
   m_allMatrices ( numberOfMats ) {
   // Nothing else to do.
}

FiniteElementMethod::ArrayOfVector3::ArrayOfVector3 ( const unsigned int numberOfVecs ) :
   m_allVectors ( numberOfVecs ) {
   // Nothing else to do.
}
