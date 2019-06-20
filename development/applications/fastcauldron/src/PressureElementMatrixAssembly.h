//
// Copyright (C) 2017 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#ifndef FASTCAULDRON__PRESSURE_ELEMENT_MATRIX_ASSEMBLY__H
#define FASTCAULDRON__PRESSURE_ELEMENT_MATRIX_ASSEMBLY__H

#include "Interface.h"

#include "FiniteElementTypes.h"
#include "FiniteElementArrayTypes.h"
#include "FiniteElementArrayOperations.h"
#include "AlignedDenseMatrix.h"
#include "AlignedWorkSpaceArrays.h"
#include "JacobianStorage.h"
#include "BasisFunctionCache.h"
#include "PropertiesToQuadPts.h"

#include "BrinePhases.h"
#include "CompoundLithology.h"
#include "MultiCompoundProperty.h"
#include "PermeabilityMixer.h"

#include "BoundaryConditions.h"
#include "LayerElement.h"
#include "Saturation.h"

/// \brief Assembles the element residual and Jacobian for the pressure equation.
class PressureElementMatrixAssembly {

public :

   /// \brief Construct with the basis function cache.
   ///
   /// All subsequent operations will use the number of quadrature points that have been used to
   /// construct the basis function cache.
   PressureElementMatrixAssembly ( const FiniteElementMethod::BasisFunctionCache& basisFunctions );

   /// \brief Compute the element residual and Jacobian for the pressure equation.
   void compute ( const LayerElement&                                layerElement,
                  const double                                       currentTime,
                  const double                                       timeStep,
                  const BoundaryConditions&                          bcs,
                  const bool                                         isIceSheetLayer,
                  const bool                                         includeChemicalCompaction,
                  const DataAccess::Interface::FracturePressureModel fractureModel,
                  const FiniteElementMethod::ElementVector&          fracturePressureExceeded,
                  const bool                                         includeWaterSaturation,
                  const Saturation&                                  currentSaturation,
                  const Saturation&                                  previousSaturation,
                  FiniteElementMethod::ElementMatrix&                elementJacobian,
                  FiniteElementMethod::ElementVector&                elementResidual );

private :

   /// \brief Remove the copy constructor.
   PressureElementMatrixAssembly ( const PressureElementMatrixAssembly& copy ) = delete;

   /// \brief Disallow copying of this class.
   PressureElementMatrixAssembly& operator=( const PressureElementMatrixAssembly& copy ) = delete;


   /// \brief Provide a named access to the arrays of scalar values.
   enum ScalarPropertyNames { QuadratureWeights = 0,
                              CurrentFluidDensity,
                              CurrentFluidDensityDerivativeWRTPressure,
                              PreviousFluidDensity,
                              CurrentFluidViscosity,
                              CurrentPorosityDerivative,
                              PreviousPorosity,
                              CurrentPermeabilityNormal,
                              CurrentPermeabilityPlane,
                              ResidualScalarWorkSpaceVector,
                              JacobianScalarWorkSpaceVector,
                              /// not to be used and needs to be last.
                              UnUsedScalarValue };

   /// \brief Provide a named access to the arrays of vector values.
   enum VectorPropertyNames { ResidualVectorWorkSpaceVector = 0,
                              /// not to be used and needs to be last.
                              UnUsedVectorValue };

   /// \brief The number of arrays of scalar values required.
   static const unsigned int NumberOfScalarArraysRequired = static_cast<unsigned int>( UnUsedScalarValue );

   /// \brief The number of arrays of vector values required.
   static const unsigned int NumberOfVectorArraysRequired = static_cast<unsigned int>( UnUsedVectorValue );

   typedef GeoPhysics::PermeabilityMixer::PermeabilityWorkSpaceArrays PermeabilityWorkSpaceArrays;

   /// \brief Return the vector for the scalar property.
   ArrayDefs::Real_ptr getScalarPropertyVector ( const ScalarPropertyNames prop );

   /// \brief Return the vector for the vector property.
   ArrayDefs::Real_ptr getVectorPropertyVector ( const VectorPropertyNames prop );

   /// \brief Load the required properties and set the Jacobians.
   ///
   /// The properties for the current and previous time steps are then
   /// interpolated at each of the quadrture points.
   void loadProperties ( const LayerElement&                 element,
                         const bool                          includeChemicalCompaction,
                         FiniteElementMethod::ElementVector& currentPo );

   /// \brief Compute all the derived properties.
   ///
   /// The derived properties are computed at each of the quadrature points.
   void computeProperties ( const LayerElement& element,
                            const bool          includeChemicalCompaction );

   /// \brief Compute the normal.
   void computeNormal ( const double                            jac11,
                        const double                            jac22,
                        const FiniteElementMethod::ThreeVector& jacRow3,
                        FiniteElementMethod::ThreeVector&       Normal );

   /// \brief Set the permeability tensor.
   void setTensor ( const double                            valueNormal,
                    const double                            valuePlane,
                    const double                            jac11,
                    const double                            jac22,
                    const FiniteElementMethod::ThreeVector& jacRow3,
                    FiniteElementMethod::Matrix3x3&         tensor );

   /// \brief Compute the product of J^-1 ( J^-1 FM )^t and scale by quadrature weight.
   void computeProduct ( const double                            jac11,
                         const double                            jac22,
                         const FiniteElementMethod::ThreeVector& jacRow3,
                         const double                            integrationScaling,
                         const FiniteElementMethod::Matrix3x3&   fluidMobility,
                         FiniteElementMethod::Matrix3x3&         result );

   /// \brief Apply the Dirichlet boundary conditions to the system.
   void applyDirichletBoundaryConditions ( const BoundaryConditions&                 bcs,
                                           const double                              dirichletBoundaryScaling,
                                           const double                              conversionFactor,
                                           const FiniteElementMethod::ElementVector& currentPropertyValues,
                                           FiniteElementMethod::ElementMatrix&       matrix,
                                           FiniteElementMethod::ElementVector&       vector );

   /// \brief Compute the fluid-mobilidy and fluid-velocity terms.
   void computeFluidMobilityTerms ( const bool                              hasFractured,
                                    const double                            fractureScaling,
                                    const double                            permeabilityNormal,
                                    const double                            permeabilityPlane,
                                    const double                            fluidDensity,
                                    const double                            fluidViscosity,
                                    const double                            relativePermeability,
                                    const double                            jac11,
                                    const double                            jac22,
                                    const FiniteElementMethod::ThreeVector& jacRow3,
                                    const FiniteElementMethod::ThreeVector& gradOverpressure,
                                    const GeoPhysics::CompoundLithology*    lithology,
                                    FiniteElementMethod::ThreeVector&       fluidVelocity,
                                    FiniteElementMethod::Matrix3x3&         fluidMobility );

   const FiniteElementMethod::BasisFunctionCache&   m_basisFunctions;
   FiniteElementMethod::PropertiesToQuadPts         m_previousProperties;
   FiniteElementMethod::PropertiesToQuadPts         m_currentProperties;

   AlignedWorkSpaceArrays<NumberOfScalarArraysRequired> m_evaluatedProperties;
   AlignedWorkSpaceArrays<NumberOfVectorArraysRequired> m_evaluatedGradProperties;

   GeoPhysics::Brine::PhaseStateVec                 m_brinePhaseState;
   GeoPhysics::MultiCompoundProperty                m_multiComponentProperty;
   PermeabilityWorkSpaceArrays                      m_permeabilityWorkSpace;
   FiniteElementMethod::JacobianStorage             m_jacobianStorage;
   FiniteElementMethod::ArrayOfVector3              m_gradPo;
   FiniteElementMethod::ArrayOfVector3              m_gradPh;
   FiniteElementMethod::ArrayOfMatrix3x3            m_gradBasisMultipliers;
   Numerics::AlignedDenseMatrix                     m_scaledBasis;
   Numerics::AlignedDenseMatrix                     m_scaledGradBasis;

};

inline ArrayDefs::Real_ptr PressureElementMatrixAssembly::getScalarPropertyVector ( const ScalarPropertyNames prop ) {
   return m_evaluatedProperties.getData ( static_cast<int>( prop ));
}

inline ArrayDefs::Real_ptr PressureElementMatrixAssembly::getVectorPropertyVector ( const VectorPropertyNames prop ) {
   return m_evaluatedGradProperties.getData ( static_cast<int>( prop ));
}


#endif // FASTCAULDRON__PRESSURE_ELEMENT_MATRIX_ASSEMBLY__H
