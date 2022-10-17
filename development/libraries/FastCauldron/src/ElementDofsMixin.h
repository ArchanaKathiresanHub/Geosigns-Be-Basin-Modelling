//                                                                      
// Copyright (C) 2015 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 
#ifndef FASTCAULDRON__ELEMENT_DOFS_MIXIN__H
#define FASTCAULDRON__ELEMENT_DOFS_MIXIN__H

// Access to STL
#ifdef _MSC_VER
#include <array>
#else
#include <tr1/array>
#endif

// Access to fastcauldron application code.
#include "ConstantsFastcauldron.h"


/// \brief Class used to add general dof numbering to an element.
class ElementDofsMixin {

public :

   /// \brief Fixed size 1d array of integers for holding dof numbers.
   typedef std::array<int, NumberOfElementNodes> DofArray;

   /// \brief Default constructor.
   ElementDofsMixin ();

   virtual ~ElementDofsMixin () {}

   /// \brief Set the global dof number for the node.
   ///
   /// \param [in] node The node whose dof number is to be set.
   /// \param [in] dof  The dof number of the node.
   /// \pre node is in the closed interval [ 0 .. 7 ].
   void setDof ( const int node,
                 const int dof );

   /// \brief Set the global dof number for the node.
   ///
   /// \param [in] node The node whose dof number is to be set.
   /// \pre node is in the closed interval [ 0 .. 7 ].
   int getDof ( const int node ) const;

   /// \brief Get access to the dofs number of the nodes.
   ///
   /// \return pointer to array containing the dof numbers for the nodes.
   /// The raw pointer is returned in order to simplyfy the call to the PETSc global matrix assembly.
   const DofArray& getDofs () const;

   /// \brief Set the boundary condition for the node
   /// 
   /// \param [in] node The node whose boundary condition is to be set.
   /// \param [in] bcs  The boundary condition value of the node.
   /// \pre node is in the closed interval [ 0 .. 7 ].
   void setBoundaryCondition ( const int                 node,
                               const Boundary_Conditions bcs );

   /// \brief Get the boundary condition for the node.
   /// 
   /// \param [in] node The node whose boundary condition is to be retrieved.
   /// \pre node is in the closed interval [ 0 .. 7 ].
   Boundary_Conditions getBoundaryConditions ( const int node ) const;


private :

   /// \brief Fixed size 1d array of boundary condition data for the nodes.
   typedef std::array<Boundary_Conditions, NumberOfElementNodes> BoundaryConditionArray;


   /// \brief The global dof numbers for each node.
   DofArray m_dofNumbers;

   /// \brief The boundary conditions values for each node.
   BoundaryConditionArray m_boundaryConditions;


};


//------------------------------------------------------------//
// Inline functions

inline const ElementDofsMixin::DofArray& ElementDofsMixin::getDofs () const {
   return m_dofNumbers;
}

inline int ElementDofsMixin::getDof ( const int node ) const {
   return m_dofNumbers [ node ];
}

inline Boundary_Conditions ElementDofsMixin::getBoundaryConditions ( const int node ) const {
   return m_boundaryConditions [ node ];
}


#endif // FASTCAULDRON__ELEMENT_DOFS_MIXIN__H

