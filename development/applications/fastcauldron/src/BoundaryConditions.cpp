//                                                                      
// Copyright (C) 2015 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 
#include "BoundaryConditions.h"

// utilities library
#include "ConstantsNumerical.h"
using Utilities::Numerical::CauldronNoDataValue;

void BoundaryConditions::reset () {

   for ( int i = 0; i < NumberOfElementNodes; ++i ) {
      m_bcs [ i ] = Interior_Boundary;
      m_bcValues [ i ] = CauldronNoDataValue;
   }

}

void BoundaryConditions::setBoundaryConditions ( const int                 node, 
                                                 const Boundary_Conditions bc,
                                                 const double              bcValue ) {
   m_bcs [ node ] = bc;
   m_bcValues [ node ] = bcValue;
}
