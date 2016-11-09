//                                                                      
// Copyright (C) 2015 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 
#ifndef FASTCAULDRON__BOUNDARY_CONDITIONS__H
#define FASTCAULDRON__BOUNDARY_CONDITIONS__H

#include "ConstantsFastcauldron.h"

/// \brief Class containing the boundary conditions and their values.
class BoundaryConditions {

public :

   /// \brief Set the boundary condition for all nodes to be an interior node and the value to the null value.
   void reset ();

   /// \brief Set the boundary condition and its value for a node.
   void setBoundaryConditions ( const int                 node,
                                const Boundary_Conditions bc,
                                const double              bcValue );

   /// \brief Get the boundary condition for a node.
   Boundary_Conditions getBoundaryCondition ( const int node ) const;

   /// \brief Get the boundary condition value for a node.
   double getBoundaryConditionValue ( const int node ) const;

public :

   /// \brief The boundary condition for each node of the element.
   Boundary_Conditions m_bcs [ NumberOfElementNodes ];

   /// \brief The boundary condition value for each node of the element.
   double              m_bcValues [ NumberOfElementNodes ];

};

inline Boundary_Conditions BoundaryConditions::getBoundaryCondition ( const int node ) const {
   return m_bcs [ node ];
}

inline double BoundaryConditions::getBoundaryConditionValue ( const int node ) const {
   return m_bcValues [ node ];
}


#endif // FASTCAULDRON__BOUNDARY_CONDITIONS__H
