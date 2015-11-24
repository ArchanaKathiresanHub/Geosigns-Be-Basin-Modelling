//                                                                      
// Copyright (C) 2015 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 
#ifndef FASTCAULDRON__SOLUTION_VECTOR_MAPPING__H
#define FASTCAULDRON__SOLUTION_VECTOR_MAPPING__H

// STL library.
#include <vector>
#include <string>

// Access to PETSc library
#include "petsc.h"

// Access to fastcauldron application code.
#include "ghost_array.h"
#include "property_manager.h"

/// \brief Forward declaration of the ComputationalDomain.
class ComputationalDomain;

/// \brief Map the solution for either the temperature or overpressure vectors.
class SolutionVectorMapping {

public :

   /// \brief Constructor.
   ///
   /// \param [in] domain The domain for which the solution vector mapping.
   /// \param [in] property The property to be stored. Can be only Temperature or Overpressure.
   SolutionVectorMapping ( ComputationalDomain&                        domain,
                           const Basin_Modelling::Fundamental_Property property );

   /// \brief Renumber the dof mappings based on the current computational domain state.
   void renumber ();

   /// \brief Store the solution vector into the 3d arrays contained in each layer.
   void putSolution ( const Vec vector ) const;

   /// \brief Get the solution vector from the 3d arrays contained in each layer.
   void getSolution ( Vec vector ) const;

   /// \brief Get the property for which this mapping applies.
   Basin_Modelling::Fundamental_Property getProperty () const;


private :

   typedef PETSc_Local_3D_Array<int> Integer3DArray;

   typedef std::vector<Integer3DArray> Integer3DArrayVector;

   /// \brief Allocate the layer number mapping if it has not already been or has the incorrect size.
   void allocateLayerMappings ( const size_t topLayerIndex,
                                size_t&      maxDepthDirection );


   /// \brief The compuational domain for which the mapping is intended.
   ComputationalDomain&                        m_computationalDomain;

   /// \brief The property that is to be mapped.
   ///
   /// Can only be 2 values, Temperature or Overpressure.
   const Basin_Modelling::Fundamental_Property m_property;

   /// \brief The scaling factor to be used for the property.
   ///
   /// Can only be 1 of 2 values, 1.0 for temperature or 1.0e6 for overpressure.
   const double                                m_scalingFactor;

   /// \brief Array of mappings for each layer.
   Integer3DArrayVector                         m_layerMappingNumbers;

};


#endif // FASTCAULDRON__SOLUTION_VECTOR_MAPPING__H
