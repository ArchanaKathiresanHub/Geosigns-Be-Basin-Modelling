//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
//------------------------------------------------------------//

#ifndef __Element_Contributions_HH__
#define __Element_Contributions_HH__

//------------------------------------------------------------//

#include "FiniteElementTypes.h"
#include "BasisFunctionCache.h"
#include "ConstantsFastcauldron.h"
#include "Quadrature.h"
#include "HydraulicFracturingManager.h"
#include "BoundaryConditions.h"

//------------------------------------------------------------//

#include "CompoundLithology.h"
#include "GeoPhysicsFluidType.h"

//------------------------------------------------------------//

#include "CompoundProperty.h"

//------------------------------------------------------------//

#include "Saturation.h"

//------------------------------------------------------------//

#include "Matrix8x8.h"

//------------------------------------------------------------//

using namespace GeoPhysics;
using namespace FiniteElementMethod;

namespace Basin_Modelling {

  void Compute_Normal
     ( const Matrix3x3&              Jacobian,
             ThreeVector&            Normal );

  void Compute_Normal
     ( const Matrix3x3&              Jacobian,
             ThreeVector&            Normal,
             ThreeVector&            angles );

   void setTensor ( const PetscScalar valueNormal,
                    const PetscScalar valuePlane,
                    const Matrix3x3&  jacobian,
                    Matrix3x3&        tensor );

  void computeFluidFlux
     ( const bool                imposeFluxLimit,
       const bool                hasFractured,
       const double              VES,
       const double              maxVES,
       const CompoundProperty&   porosity,
       const double              fluidViscosity,
       const Matrix3x3&          jacobian,
       const ThreeVector&        gradOverpressure,
       const double              relativePermeability,
       const CompoundLithology*  lithology,
             ThreeVector&        fluidFlux );

  bool Degenerate_Element ( const ElementGeometryMatrix& geometryMatrix );


  void Compute_Heat_Flow
     ( const bool                   isBasementFormation,
      const CompoundLithology*     Lithology,
       const GeoPhysics::FluidType* Fluid,
       const ElementVector&         Temperature_Vector,
       const double                 Temperature_Value,
       const double                 Porosity,
       const double                 PorePressure,
       const double                 LithostaticPressure,
       const Matrix3x3&          Jacobian,
       const GradElementVector& Grad_Basis,
             ThreeVector&        Heat_Flow,
             Matrix3x3&          Conductivity_Tensor );


  /// \brief Applies the Neumann type boundar conditions to the temperature equations.
  void applyHeatFlowBoundaryConditions ( const ElementGeometryMatrix& geometryMatrix,
                                         const BoundaryConditions&    bcs,
                                         ElementVector&               ElementVector_Contributions );

   /// \brief Apply Dirichlet type boundary conditions for non-linear pressure and temperature equations.
   void applyDirichletBoundaryConditionsNewton ( const BoundaryConditions&  bcs,
                                                 const double               dirichletBoundaryScaling,
                                                 const double               conversionFactor,
                                                 const ElementVector&       currentPropertyValues,
                                                 ElementMatrix&             matrix,
                                                 ElementVector&             vector );

   /// \brief Apply Dirichlet type boundary conditions for non-linear pressure and temperature equations.
   void applyDirichletBoundaryConditionsNewton ( const BoundaryConditions&  bcs,
                                                 const double               dirichletBoundaryScaling,
                                                 const double               conversionFactor,
                                                 const ElementVector&       currentPropertyValues,
                                                 ElementVector&             vector );

   /// \brief Assemble the element Jacobian and residual for the temperature equation.
  void assembleElementTemperatureSystem ( const bool                   isBasementFormation,
                                          const int                    planeQuadratureDegree,
                                          const int                    depthQuadratureDegree,
                                          const double                 timeStep,
                                          const bool                   Include_Advection_Term,
                                          const BoundaryConditions&    bcs,
                                          const CompoundLithology*     lithology,
                                          const GeoPhysics::FluidType* fluid,
                                          const bool                   includeChemicalCompaction,
                                          const ElementGeometryMatrix& geometryMatrix,
                                          const ElementVector&         Element_Heat_Production,
                                          const ElementVector&         Current_Ph,
                                          const ElementVector&         Current_Po,
                                          const ElementVector&         Current_Lp,
                                          const ElementVector&         Current_Element_VES,
                                          const ElementVector&         Current_Element_Max_VES,
                                          const ElementVector&         Previous_Element_Temperature,
                                          const ElementVector&         Current_Element_Temperature,
                                          const ElementVector&         Current_Element_Chemical_Compaction,
                                          ElementMatrix&               Element_Jacobian,
                                          ElementVector&               Element_Residual );


   /// \brief Assemble the element residual for the temperature equation.
  void assembleElementTemperatureResidual ( const bool                   isBasementFormation,
                                            const int                    planeQuadratureDegree,
                                            const int                    depthQuadratureDegree,
                                            const double                 timeStep,
                                            const bool                   Include_Advection_Term,
                                            const BoundaryConditions&    bcs,
                                            const CompoundLithology*     lithology,
                                            const GeoPhysics::FluidType* fluid,
                                            const bool                   includeChemicalCompaction,
                                            const ElementGeometryMatrix& geometryMatrix,
                                            const ElementVector&         Element_Heat_Production,
                                            const ElementVector&         Current_Ph,
                                            const ElementVector&         Current_Po,
                                            const ElementVector&         Current_Lp,
                                            const ElementVector&         Current_Element_VES,
                                            const ElementVector&         Current_Element_Max_VES,
                                            const ElementVector&         Previous_Element_Temperature,
                                            const ElementVector&         Current_Element_Temperature,
                                            const ElementVector&         Current_Element_Chemical_Compaction,
                                            ElementVector&               Element_Residual );


} // end namespace Basin_Modelling


//------------------------------------------------------------//

#endif // __Element_Contributions_HH__

//------------------------------------------------------------//
