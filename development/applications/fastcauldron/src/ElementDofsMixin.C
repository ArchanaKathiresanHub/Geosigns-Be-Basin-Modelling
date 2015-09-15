//                                                                      
// Copyright (C) 2015 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 
#include "ElementDofsMixin.h"

ElementDofsMixin::ElementDofsMixin () {

   for ( int i = 0; i < 8; ++i ) {
      m_dofNumbers [ i ] = -1;
   }

}

void ElementDofsMixin::setDof ( const int node,
                                const int dof ) {
   m_dofNumbers [ node ] = dof;
}


void ElementDofsMixin::setBoundaryCondition ( const int                 node,
                                              const Boundary_Conditions bcs ) {
   m_bcs [ node ] = bcs;
}

