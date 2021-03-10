//
// Copyright (C) 2017 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#ifndef FASTCAULDRON__TEMPERATURE_ELEMENT_ASSEMBLY__H
#define FASTCAULDRON__TEMPERATURE_ELEMENT_ASSEMBLY__H

#include "Interface.h"

#include "FiniteElementTypes.h"
#include "FiniteElementArrayTypes.h"
#include "FiniteElementArrayOperations.h"
#include "AlignedDenseMatrix.h"
#include "AlignedWorkSpaceArrays.h"
#include "JacobianStorage.h"
#include "BasisFunctionCache.h"
#include "PropertiesToQuadPts.h"

#include "CompoundLithology.h"
#include "BrinePhases.h"
#include "MultiCompoundProperty.h"
#include "PermeabilityMixer.h"

#include "BoundaryConditions.h"
#include "LayerElement.h"
#include "Saturation.h"

/// \brief Assembles the element contributions for the unsteady temperature equation.
class TemperatureElementAssembly {

public :

   /// \brief Construct with the basis function cache.
   ///
   /// All subsequent operations will use the number of quadrature points that have been used to
   /// construct the basis function cache.
   TemperatureElementAssembly ( const FiniteElementMethod::BasisFunctionCache& basisFunctions );

   /// \brief Compute the element stiffness matrix and load vector for the unsteady temperature equation.
   void compute ( const LayerElement&                       element,
                  const GeoPhysics::CompoundLithology*      lithology,
                  const double                              currentTime,
                  const double                              timeStep,
                  const bool                                includeAdvectionTerm,
                  const BoundaryConditions&                 bcs,
                  const bool                                includeChemicalCompaction,
                  const FiniteElementMethod::ElementVector& elementHeatProduction,
                  FiniteElementMethod::ElementMatrix&       elementStiffnessMatrix,
                  FiniteElementMethod::ElementVector&       elementLoadVector );


private :

   /// \brief Remove the copy constructor.
   TemperatureElementAssembly ( const TemperatureElementAssembly& copy ) = delete;

   /// \brief Disallow copying of this class.
   TemperatureElementAssembly& operator=( const TemperatureElementAssembly& copy ) = delete;

   /// \brief Provide a named access to the arrays of scalar values.
   enum ScalarPropertyNames { QuadratureWeights = 0,
                              CurrentFluidDensity,
                              CurrentFluidViscosity,
                              CurrentPorosity,
                              CurrentBulkDensityTimesHeatCap,
                              CurrentFluidDensityTimesHeatCap,
                              CurrentPermeabilityNormal,
                              CurrentPermeabilityPlane,
                              CurrentFluidThermalConductivity,
                              CurrentBulkThermalConductivityNormal,
                              CurrentBulkThermalConductivityPlane,
                              PreviousPorosity,
                              PreviousBulkDensityTimesHeatCap,
                              PreviousFluidDensityTimesHeatCap,
                              PreviousFluidDensity,

                              // Used as a work space vector during some calculations.
                              LoadVectorWorkSpace,
                              StiffnessMatrixScalarWorkSpace,
                              ScalarWorkSpace,

                              /// not to be used and needs to be last.
                              UnUsedScalarValue };


   /// \brief The number of arrays of scalar values required.
   static const unsigned int NumberOfScalarArraysRequired = static_cast<unsigned int>( UnUsedScalarValue );

   typedef GeoPhysics::PermeabilityMixer::PermeabilityWorkSpaceArrays PermeabilityWorkSpaceArrays;


   /// \brief Return the vector for the scalar property.
   ArrayDefs::Real_ptr getScalarPropertyVector ( const ScalarPropertyNames prop );


   /// \brief Load the required properties and set the stiffness matrix and load vector.
   ///
   /// The properties for the current and previous time steps are then
   /// interpolated at each of the quadrture points.
   void loadProperties ( const LayerElement&  element,
                         const FiniteElementMethod::ElementVector& heatProduction,
                         const bool           includeChemicalCompaction,
                         const bool           includeAdvectionTerm );

   /// \brief Compute all the derived properties.
   ///
   /// The derived properties are computed at each of the quadrature points.
   void computeDerivedProperties ( const LayerElement&                  element,
                                   const GeoPhysics::CompoundLithology* lithology,
                                   const bool                           includeChemicalCompaction,
                                   const bool                           isBasementFormation,
                                   const bool                           includeAdvectionTerm );

   /// \brief Set the thermal conductivity tensor.
   void setTensor ( const double       valueNormal,
                    const double       valuePlane,
                    const double       jac11,
                    const double       jac22,
                    const FiniteElementMethod::ThreeVector& jacRow3,
                    FiniteElementMethod::Matrix3x3&         tensor );

   void computeProduct ( const double       jac11,
                         const double       jac22,
                         const FiniteElementMethod::ThreeVector& jacRow3,
                         const double       integrationScaling,
                         const FiniteElementMethod::Matrix3x3&   mat,
                         FiniteElementMethod::Matrix3x3&         result );

   /// \brief Compute the product of J^-1 ( J^-1 TC )^t and scale by quadrature weight.
   void computeFluidFlux ( const bool                               imposeFluxLimit,
                           const bool                               hasFractured,
                           const double                             permeabilityNormalValue,
                           const double                             permeabilityPlaneValue,
                           const double                             fluidViscosity,
                           const double                             jac11,
                           const double                             jac22,
                           const FiniteElementMethod::ThreeVector&  jacRow3,
                           const FiniteElementMethod::ThreeVector&  gradOverpressure,
                           const double                             relativePermeability,
                           const GeoPhysics::CompoundLithology*     lithology,
                           FiniteElementMethod::ThreeVector&        fluidFlux );

   /// \brief Apply the heat-flow, Neumann type, boundary conditions to the system.
   void applyHeatFlowBoundaryConditions ( const LayerElement&                 element,
                                          const BoundaryConditions&           bcs,
                                          FiniteElementMethod::ElementVector& ElementVector_Contributions );

   /// \brief Apply the Dirichlet boundary conditions to the system.
   void applyDirichletBoundaryConditionsLinear ( const BoundaryConditions&           bcs,
                                                 const double                        Dirichlet_Boundary_Scaling,
                                                 FiniteElementMethod::ElementMatrix& Stiffness_Matrix,
                                                 FiniteElementMethod::ElementVector& Load_Vector );


   const FiniteElementMethod::BasisFunctionCache&   m_basisFunctions;
   FiniteElementMethod::PropertiesToQuadPts         m_previousProperties;
   FiniteElementMethod::PropertiesToQuadPts         m_currentProperties;

   AlignedWorkSpaceArrays<NumberOfScalarArraysRequired> m_evaluatedProperties;
   GeoPhysics::Brine::PhaseStateVec                 m_brinePhaseState;
   GeoPhysics::MultiCompoundProperty                m_multiComponentProperty;
   PermeabilityWorkSpaceArrays                      m_permeabilityWorkSpace;
   FiniteElementMethod::JacobianStorage             m_jacobianStorage;
   FiniteElementMethod::ArrayOfVector3              m_gradPo;
   FiniteElementMethod::ArrayOfVector3              m_gradPh;
   FiniteElementMethod::ArrayOfMatrix3x3            m_gradBasisMultipliers;
   FiniteElementMethod::ArrayOfVector3              m_advectionMultipliers;
   Numerics::AlignedDenseMatrix                     m_scaledBasis;
   Numerics::AlignedDenseMatrix                     m_scaledGradBasis;

};

inline ArrayDefs::Real_ptr TemperatureElementAssembly::getScalarPropertyVector ( const ScalarPropertyNames prop ) {
   return m_evaluatedProperties.getData ( static_cast<int>( prop ));
}

#endif // FASTCAULDRON__TEMPERATURE_ELEMENT_ASSEMBLY__H
