//
// Copyright (C) 2017 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "TemperatureElementAssembly.h"

#include "Interface.h"
#include "GeoPhysicsFluidType.h"

#include "ConstantsMathematics.h"

#include "FastcauldronSimulator.h"
#include "property_manager.h"
#include "ElementContributions.h"
#include "Lithology.h"
#include "layer.h"
#include "Quadrature.h"

using namespace FiniteElementMethod;
using namespace DataAccess;
using namespace GeoPhysics;
using namespace Utilities;
using namespace Maths;

TemperatureElementAssembly::TemperatureElementAssembly ( const BasisFunctionCache& basisFunctions ) :
   m_basisFunctions ( basisFunctions ),
   m_previousProperties ( basisFunctions.getBasisFunctions ()),
   m_currentProperties ( basisFunctions.getBasisFunctions ()),
   m_evaluatedProperties ( basisFunctions.getNumberOfQuadraturePoints ()),
   m_brinePhaseState ( basisFunctions.getNumberOfQuadraturePoints (), 0.0 ),
   m_permeabilityWorkSpace ( basisFunctions.getNumberOfQuadraturePoints ()),
   m_jacobianStorage ( basisFunctions.getNumberOfQuadraturePoints ()),
   m_gradPo ( basisFunctions.getNumberOfQuadraturePoints ()),
   m_gradPh ( basisFunctions.getNumberOfQuadraturePoints ()),
   m_gradBasisMultipliers ( basisFunctions.getNumberOfQuadraturePoints ()),
   m_advectionMultipliers ( basisFunctions.getNumberOfQuadraturePoints ()),
   m_scaledBasis ( 8, basisFunctions.getNumberOfQuadraturePoints ()),
   m_scaledGradBasis ( 8, 3 * basisFunctions.getNumberOfQuadraturePoints ())
{
   NumericFunctions::Quadrature::QuadratureArray xQuadraturePoints;
   NumericFunctions::Quadrature::QuadratureArray xQuadratureWeights;

   NumericFunctions::Quadrature::QuadratureArray yQuadraturePoints;
   NumericFunctions::Quadrature::QuadratureArray yQuadratureWeights;

   NumericFunctions::Quadrature::QuadratureArray zQuadraturePoints;
   NumericFunctions::Quadrature::QuadratureArray zQuadratureWeights;

   const int numberOfXPoints = m_basisFunctions.getNumberOfPointsX ();
   const int numberOfYPoints = m_basisFunctions.getNumberOfPointsY ();
   const int numberOfZPoints = m_basisFunctions.getNumberOfPointsZ ();

   NumericFunctions::Quadrature::getInstance ().getGaussLegendreQuadrature ( numberOfXPoints, xQuadraturePoints, xQuadratureWeights );
   NumericFunctions::Quadrature::getInstance ().getGaussLegendreQuadrature ( numberOfYPoints, yQuadraturePoints, yQuadratureWeights );
   NumericFunctions::Quadrature::getInstance ().getGaussLegendreQuadrature ( numberOfZPoints, zQuadraturePoints, zQuadratureWeights );

   ArrayDefs::Real_ptr quadratureWeights = getScalarPropertyVector ( QuadratureWeights );

   // Only the quadrature weights are required.
   // They will be the same for every integral, so need be computed only once.
   for ( int i = 0, count = 0; i < numberOfXPoints; ++i ) {

      for ( int j = 0; j < numberOfYPoints; ++j ) {

         for ( int k = 0; k < numberOfZPoints; ++k, ++count ) {
            quadratureWeights [ count ] = xQuadratureWeights [ i ] * yQuadratureWeights [ j ] * zQuadratureWeights [ k ];
         }

      }

   }

}

void TemperatureElementAssembly::loadProperties ( const LayerElement&  element,
                                                  const ElementVector& heatProduction,
                                                  const bool           includeChemicalCompaction,
                                                  const bool           includeAdvectionTerm ) {

   const CauldronGridDescription& grid = FastcauldronSimulator::getInstance ().getCauldronGridDescription ();
   double deltaX  = grid.deltaI;
   double deltaY  = grid.deltaJ;

   ElementVector currentPl;
   ElementVector currentPp;
   ElementVector currentPh;
   ElementVector currentPo;
   ElementVector currentElementVES;
   ElementVector currentElementMaxVES;
   ElementVector currentChemicalCompaction;
   ElementVector currentElementTemperature;
   ElementVector currentDepth;

   ElementVector previousChemicalCompaction;
   ElementVector previousPh;
   ElementVector previousPp;
   ElementVector previousPl;
   ElementVector previousElementVES;
   ElementVector previousElementMaxVES;
   ElementVector previousElementTemperature;

   getCoefficients ( element, Basin_Modelling::Depth,                currentDepth );
   getCoefficients ( element, Basin_Modelling::Overpressure,         currentPo );
   getCoefficients ( element, Basin_Modelling::Hydrostatic_Pressure, currentPh );
   getCoefficients ( element, Basin_Modelling::Lithostatic_Pressure, currentPl );
   getCoefficients ( element, Basin_Modelling::VES_FP,               currentElementVES );
   getCoefficients ( element, Basin_Modelling::Max_VES,              currentElementMaxVES );
   getCoefficients ( element, Basin_Modelling::Temperature,          currentElementTemperature );
   add ( currentPo, currentPh, currentPp );

   getPreviousCoefficients ( element, Basin_Modelling::Hydrostatic_Pressure, previousPh );
   getPreviousCoefficients ( element, Basin_Modelling::Pore_Pressure,        previousPp );
   getPreviousCoefficients ( element, Basin_Modelling::Lithostatic_Pressure, previousPl );
   getPreviousCoefficients ( element, Basin_Modelling::VES_FP,               previousElementVES );
   getPreviousCoefficients ( element, Basin_Modelling::Max_VES,              previousElementMaxVES );
   getPreviousCoefficients ( element, Basin_Modelling::Temperature,          previousElementTemperature );

   if ( includeChemicalCompaction ) {
      getCoefficients ( element, Basin_Modelling::Chemical_Compaction, currentChemicalCompaction );
      getPreviousCoefficients ( element, Basin_Modelling::Chemical_Compaction, previousChemicalCompaction );
   } else {
      previousChemicalCompaction.zero ();
      currentChemicalCompaction.zero ();
   }

   m_currentProperties.compute ( FiniteElementMethod::Ph,       currentPh,
                                 FiniteElementMethod::Po,       currentPo,
                                 FiniteElementMethod::Pp,       currentPp,
                                 FiniteElementMethod::Pl,       currentPl,
                                 FiniteElementMethod::Ves,      currentElementVES,
                                 FiniteElementMethod::MaxVes,   currentElementMaxVES,
                                 FiniteElementMethod::ChemComp, currentChemicalCompaction,
                                 FiniteElementMethod::Temp,     currentElementTemperature,
                                 FiniteElementMethod::HeatProd, heatProduction );

   m_previousProperties.compute ( FiniteElementMethod::Ph,       previousPh,
                                  FiniteElementMethod::Pp,       previousPp,
                                  FiniteElementMethod::Pl,       previousPl,
                                  FiniteElementMethod::Ves,      previousElementVES,
                                  FiniteElementMethod::MaxVes,   previousElementMaxVES,
                                  FiniteElementMethod::ChemComp, previousChemicalCompaction,
                                  FiniteElementMethod::Temp,     previousElementTemperature );

   m_jacobianStorage.set ( deltaX, deltaY, currentDepth, m_basisFunctions.getGradBasisFunctions ());

   if ( includeAdvectionTerm and element.getFluid () != nullptr ) {
      // Whilst the Po and Ph vectors are available, compute their gradients.
      // Later, when the Jacobian has been computed they can be transformed to real coordinates.
      FiniteElementMethod::ArrayOperations::computeGradProperty ( m_basisFunctions.getGradBasisFunctions (),
                                                                  currentPo, currentPh,
                                                                  m_gradPo, m_gradPh );

      // Now that the inverse jacobians have been computed the gradients can be transformed to real coordinates.
      FiniteElementMethod::ArrayOperations::productWithInverse ( m_jacobianStorage, m_gradPo );
      FiniteElementMethod::ArrayOperations::productWithInverse ( m_jacobianStorage, m_gradPh );

      for ( int i = 0; i < m_basisFunctions.getNumberOfQuadraturePoints (); ++i ) {
         m_gradPo [ i ]( 1 ) += m_gradPh [ i ]( 1 );
         m_gradPo [ i ]( 2 ) += m_gradPh [ i ]( 2 );
      }

   }

}

void TemperatureElementAssembly::computeDerivedProperties ( const LayerElement&      element,
                                                            const CompoundLithology* lithology,
                                                            const bool               includeChemicalCompaction,
                                                            const bool               isBasementFormation,
                                                            const bool               includeAdvectionTerm ) {

   // Compute all brine properties.

   bool elementHasFluid = ( element.getFluid () != nullptr );

   if ( elementHasFluid ) {
      m_brinePhaseState.setSalinity ( element.getFluid ()->salinity ());

      m_brinePhaseState.set ( m_basisFunctions.getNumberOfQuadraturePoints (),
                              m_currentProperties.getProperty  ( FiniteElementMethod::Temp ),
                              m_currentProperties.getProperty  ( FiniteElementMethod::Pp ));

      element.getFluid ()->density   ( m_brinePhaseState, getScalarPropertyVector ( CurrentFluidDensity ));
      element.getFluid ()->densXheatCapacity ( m_brinePhaseState,
                                               getScalarPropertyVector ( CurrentFluidDensity ),
                                               getScalarPropertyVector ( CurrentFluidDensityTimesHeatCap ));
      element.getFluid ()->thermalConductivity ( m_brinePhaseState, getScalarPropertyVector ( CurrentFluidThermalConductivity ));

      if ( includeAdvectionTerm ) {
         element.getFluid ()->viscosity ( m_brinePhaseState, getScalarPropertyVector ( CurrentFluidViscosity ));
      }

      m_brinePhaseState.set ( m_basisFunctions.getNumberOfQuadraturePoints (),
                              m_previousProperties.getProperty  ( FiniteElementMethod::Temp ),
                              m_previousProperties.getProperty  ( FiniteElementMethod::Pp ));

      element.getFluid ()->density ( m_brinePhaseState, getScalarPropertyVector ( PreviousFluidDensity ));
      element.getFluid ()->densXheatCapacity ( m_brinePhaseState,
                                               getScalarPropertyVector ( PreviousFluidDensity ),
                                               getScalarPropertyVector ( PreviousFluidDensityTimesHeatCap ));

   }

   // Compute all lithology properties.

   if ( includeAdvectionTerm and elementHasFluid ) {
      m_multiComponentProperty.resize ( lithology->getNumberOfSimpleLithologies (), m_evaluatedProperties.getNumberOfValues ());
      lithology->getPorosity ( m_basisFunctions.getNumberOfQuadraturePoints (),
                               m_currentProperties.getProperty ( FiniteElementMethod::Ves ),
                               m_currentProperties.getProperty ( FiniteElementMethod::MaxVes ),
                               includeChemicalCompaction,
                               m_currentProperties.getProperty ( FiniteElementMethod::ChemComp ),
                               m_multiComponentProperty );

      // permeability work space only needed here,
      lithology->calcBulkPermeabilityNP ( m_basisFunctions.getNumberOfQuadraturePoints (),
                                          m_currentProperties.getProperty ( FiniteElementMethod::Ves ),
                                          m_currentProperties.getProperty ( FiniteElementMethod::MaxVes ),
                                          m_multiComponentProperty,
                                          getScalarPropertyVector ( CurrentPermeabilityNormal ),
                                          getScalarPropertyVector ( CurrentPermeabilityPlane ),
                                          m_permeabilityWorkSpace );
   } else {
      lithology->getPorosity ( m_basisFunctions.getNumberOfQuadraturePoints (),
                               m_currentProperties.getProperty ( FiniteElementMethod::Ves ),
                               m_currentProperties.getProperty ( FiniteElementMethod::MaxVes ),
                               includeChemicalCompaction,
                               m_currentProperties.getProperty ( FiniteElementMethod::ChemComp ),
                               getScalarPropertyVector ( CurrentPorosity ));
   }

   const double* currentPorosityVec = ( includeAdvectionTerm and elementHasFluid ? m_multiComponentProperty.getMixedData () : getScalarPropertyVector ( CurrentPorosity ));

   lithology->getPorosity ( m_basisFunctions.getNumberOfQuadraturePoints (),
                            m_previousProperties.getProperty ( FiniteElementMethod::Ves ),
                            m_previousProperties.getProperty ( FiniteElementMethod::MaxVes ),
                            includeChemicalCompaction,
                            m_previousProperties.getProperty ( FiniteElementMethod::ChemComp ),
                            getScalarPropertyVector ( PreviousPorosity ));


   lithology->calcBulkDensXHeatCapacity ( m_basisFunctions.getNumberOfQuadraturePoints (),
                                          ( elementHasFluid ? getScalarPropertyVector ( CurrentFluidDensityTimesHeatCap ) : nullptr ),
                                          currentPorosityVec,
                                          m_currentProperties.getProperty ( FiniteElementMethod::Pp ),
                                          m_currentProperties.getProperty ( FiniteElementMethod::Temp ),
                                          m_currentProperties.getProperty ( FiniteElementMethod::Pl ),
                                          getScalarPropertyVector ( CurrentBulkDensityTimesHeatCap ),
                                          getScalarPropertyVector ( ScalarWorkSpace ));

   lithology->calcBulkDensXHeatCapacity ( m_basisFunctions.getNumberOfQuadraturePoints (),
                                          ( elementHasFluid ? getScalarPropertyVector ( PreviousFluidDensityTimesHeatCap ) : nullptr ),
                                          getScalarPropertyVector ( PreviousPorosity ),
                                          m_previousProperties.getProperty ( FiniteElementMethod::Pp ),
                                          m_previousProperties.getProperty ( FiniteElementMethod::Temp ),
                                          m_previousProperties.getProperty ( FiniteElementMethod::Pl ),
                                          getScalarPropertyVector ( PreviousBulkDensityTimesHeatCap ),
                                          getScalarPropertyVector ( ScalarWorkSpace ));

   if ( isBasementFormation ) {
      lithology->calcBulkThermCondNPBasement ( m_basisFunctions.getNumberOfQuadraturePoints (),
                                               m_currentProperties.getProperty ( FiniteElementMethod::Temp ),
                                               m_currentProperties.getProperty ( FiniteElementMethod::Pl ),
                                               getScalarPropertyVector ( CurrentBulkThermalConductivityNormal ),
                                               getScalarPropertyVector ( CurrentBulkThermalConductivityPlane ));
   } else {
      lithology->calcBulkThermCondNP ( m_basisFunctions.getNumberOfQuadraturePoints (),
                                       ( elementHasFluid ? getScalarPropertyVector ( CurrentFluidThermalConductivity ) : nullptr ),
                                       currentPorosityVec,
                                       m_currentProperties.getProperty ( FiniteElementMethod::Temp ),
                                       m_currentProperties.getProperty ( FiniteElementMethod::Pp ),
                                       getScalarPropertyVector ( CurrentBulkThermalConductivityNormal ),
                                       getScalarPropertyVector ( CurrentBulkThermalConductivityPlane ));
   }

}

void TemperatureElementAssembly::setTensor ( const double       valueNormal,
                                             const double       valuePlane,
                                             const double       jac11,
                                             const double       jac22,
                                             const ThreeVector& jacRow3,
                                             Matrix3x3&         tensor ) {

   ThreeVector normal;
   double      normalLengthSq;
   double      valueDifference;

   normal ( 1 ) =  jac22 * jacRow3 ( 1 );
   normal ( 2 ) =  jacRow3 ( 2 ) * jac11;
   normal ( 3 ) = -jac22 * jac11;

   normalLengthSq = normal ( 1 ) * normal ( 1 ) + normal ( 2 ) * normal ( 2 ) + normal ( 3 ) * normal ( 3 );

   valueDifference = valueNormal - valuePlane;
   valueDifference /= normalLengthSq;

   tensor ( 1, 1 ) = valueDifference * normal ( 1 ) * normal ( 1 ) + valuePlane;
   tensor ( 2, 1 ) = valueDifference * normal ( 2 ) * normal ( 1 );
   tensor ( 3, 1 ) = valueDifference * normal ( 3 ) * normal ( 1 );

   tensor ( 1, 2 ) = valueDifference * normal ( 1 ) * normal ( 2 );
   tensor ( 2, 2 ) = valueDifference * normal ( 2 ) * normal ( 2 ) + valuePlane;
   tensor ( 3, 2 ) = valueDifference * normal ( 3 ) * normal ( 2 );

   tensor ( 1, 3 ) = valueDifference * normal ( 1 ) * normal ( 3 );
   tensor ( 2, 3 ) = valueDifference * normal ( 2 ) * normal ( 3 );
   tensor ( 3, 3 ) = valueDifference * normal ( 3 ) * normal ( 3 ) + valuePlane;

}

void TemperatureElementAssembly::computeProduct ( const double       jac11,
                                                  const double       jac22,
                                                  const ThreeVector& jacRow3,
                                                  const double       integrationScaling,
                                                  const Matrix3x3&   mat,
                                                  Matrix3x3&         result ) {




#if USE_HIGHLY_OPTIMISED_MMP
   // Not quite correct.
   double t1 = jacRow3 ( 1 ) * mat ( 1, 1 ) + jacRow3 ( 2 ) * mat ( 1, 2 ) + jacRow3 ( 3 ) * mat ( 1, 3 );
   double t2 = jacRow3 ( 1 ) * mat ( 1, 2 ) + jacRow3 ( 2 ) * mat ( 2, 2 ) + jacRow3 ( 3 ) * mat ( 2, 3 );
   double t3 = jacRow3 ( 1 ) * mat ( 1, 3 ) + jacRow3 ( 2 ) * mat ( 2, 3 ) + jacRow3 ( 3 ) * mat ( 3, 3 );

   result ( 1, 1 ) = jac11 * jac11 * mat ( 1, 1 );
   result ( 2, 1 ) = result ( 1, 2 ) = jac11 * jac22 * mat ( 1, 2 );
   result ( 3, 1 ) = result ( 1, 3 ) = jac11 * jac11 * t1;

   result ( 2, 2 ) = jac22 * jac22 * mat ( 2, 2 );
   result ( 3, 2 ) = result ( 2, 3 ) = jac22 * t2;

   result ( 3, 2 ) = mat ( 2, 3 );
   result ( 3, 3 ) = jacRow3 ( 1 ) * t1 + jacRow3 ( 2 ) * t2 + jacRow3 ( 3 ) * t3;

#else
   Matrix3x3 intermediateMatrix;

   intermediateMatrix ( 1, 1 ) = jac11 * mat ( 1, 1 );
   intermediateMatrix ( 1, 2 ) = jac11 * mat ( 1, 2 );
   intermediateMatrix ( 1, 3 ) = jac11 * mat ( 1, 3 );

   intermediateMatrix ( 2, 1 ) = jac22 * mat ( 2, 1 );
   intermediateMatrix ( 2, 2 ) = jac22 * mat ( 2, 2 );
   intermediateMatrix ( 2, 3 ) = jac22 * mat ( 2, 3 );

   intermediateMatrix ( 3, 1 ) = jacRow3 ( 1 ) * mat ( 1, 1 ) + jacRow3 ( 2 ) * mat ( 2, 1 ) + jacRow3 ( 3 ) * mat ( 3, 1 );
   intermediateMatrix ( 3, 2 ) = jacRow3 ( 1 ) * mat ( 1, 2 ) + jacRow3 ( 2 ) * mat ( 2, 2 ) + jacRow3 ( 3 ) * mat ( 3, 2 );
   intermediateMatrix ( 3, 3 ) = jacRow3 ( 1 ) * mat ( 1, 3 ) + jacRow3 ( 2 ) * mat ( 2, 3 ) + jacRow3 ( 3 ) * mat ( 3, 3 );

   result ( 1, 1 ) = integrationScaling * jac11 * intermediateMatrix ( 1, 1 );
   result ( 1, 2 ) = integrationScaling * jac11 * intermediateMatrix ( 2, 1 );
   result ( 1, 3 ) = integrationScaling * jac11 * intermediateMatrix ( 3, 1 );

   result ( 2, 1 ) = integrationScaling * jac22 * intermediateMatrix ( 1, 2 );
   result ( 2, 2 ) = integrationScaling * jac22 * intermediateMatrix ( 2, 2 );
   result ( 2, 3 ) = integrationScaling * jac22 * intermediateMatrix ( 3, 2 );

   result ( 3, 1 ) = integrationScaling * ( jacRow3 ( 1 ) * intermediateMatrix ( 1, 1 ) + jacRow3 ( 2 ) * intermediateMatrix ( 1, 2 ) + jacRow3 ( 3 ) * intermediateMatrix ( 1, 3 ));
   result ( 3, 2 ) = integrationScaling * ( jacRow3 ( 1 ) * intermediateMatrix ( 2, 1 ) + jacRow3 ( 2 ) * intermediateMatrix ( 2, 2 ) + jacRow3 ( 3 ) * intermediateMatrix ( 2, 3 ));
   result ( 3, 3 ) = integrationScaling * ( jacRow3 ( 1 ) * intermediateMatrix ( 3, 1 ) + jacRow3 ( 2 ) * intermediateMatrix ( 3, 2 ) + jacRow3 ( 3 ) * intermediateMatrix ( 3, 3 ));

#endif

}

void TemperatureElementAssembly::computeFluidFlux ( const bool               imposeFluxLimit,
                                                    const bool               hasFractured,
                                                    const double             permeabilityNormalValue,
                                                    const double             permeabilityPlaneValue,
                                                    const double             fluidViscosity,
                                                    const double             jac11,
                                                    const double             jac22,
                                                    const ThreeVector&       jacRow3,
                                                    const ThreeVector&       gradOverpressure,
                                                    const double             relativePermeability,
                                                    const CompoundLithology* lithology,
                                                    ThreeVector&             fluidFlux ) {

   /// Limit the water flux to be within a reasonable range.
   /// This should be a depth related function, since it seems to be only
   /// the shallower parts where the high fluid fluxes cause a problem.
   const double MaximumFlux = 1.0e-9;

   /// This limits the permeability to be within a reasonable range. The soil mechanics sandstone
   /// has a particularly high permeability 10^8.6 milli-Darcy (almost 6 orders of magnitude higher
   /// than the standard exponential sandstone, for example).
   const double MaximumPermeability = 10.0 * MilliDarcyToM2;

   double permeabilityNormal = permeabilityNormalValue;
   double permeabilityPlane = permeabilityPlaneValue;
   double permeabilityScaling;

   /// Not quite the permeability tensor, since it is also divided by the fluid viscosity.
   Matrix3x3 permeabilityTensor;

   if ( hasFractured ) {
      permeabilityScaling = lithology->fracturedPermeabilityScaling ();
   } else {
      permeabilityScaling = 1.0;
   }

   // Limit the permeability to some maximum value.
   if ( imposeFluxLimit and permeabilityNormal > MaximumPermeability ) {
      permeabilityNormal = MaximumPermeability;
   }

   // Limit the permeability to some maximum value.
   if ( imposeFluxLimit and permeabilityPlane > MaximumPermeability ) {
      permeabilityPlane = MaximumPermeability;
   }

   permeabilityNormal = permeabilityScaling * relativePermeability * permeabilityNormal / fluidViscosity;
   permeabilityPlane  =                       relativePermeability * permeabilityPlane  / fluidViscosity;

   setTensor ( permeabilityNormal, permeabilityPlane, jac11, jac22, jacRow3, permeabilityTensor );
   matrixVectorProduct ( permeabilityTensor, gradOverpressure, fluidFlux );

   // Since pressure properties are stored in MPa units, we must convert to Pa to use in calculation.
   fluidFlux ( 1 ) = -fluidFlux ( 1 ) * MegaPaToPa;
   fluidFlux ( 2 ) = -fluidFlux ( 2 ) * MegaPaToPa;
   fluidFlux ( 3 ) = -fluidFlux ( 3 ) * MegaPaToPa;

   // Limit the fluid flux to some maximum value, if requested.
   if ( imposeFluxLimit ) {

      for ( int i = 1; i <= 3; ++i ) {

         if ( fluidFlux ( i ) > MaximumFlux ) {
            fluidFlux ( i ) = MaximumFlux;
         } else if ( fluidFlux ( i ) < -MaximumFlux ) {
            fluidFlux ( i ) = -MaximumFlux;
         }

      }

   }

}

void TemperatureElementAssembly::applyHeatFlowBoundaryConditions ( const LayerElement&       element,
                                                                   const BoundaryConditions& bcs,
                                                                   ElementVector&            elementVectorContributions ) {

   double xQuadraturePoints [ 4 ] = { -1.0,  1.0, 1.0, -1.0 };
   double yQuadraturePoints [ 4 ] = { -1.0, -1.0, 1.0,  1.0 };
   double zQuadraturePoint = 1.0;

   ElementGeometryMatrix geometryMatrix;
   double            normalLength;
   ElementVector     Basis;
   GradElementVector gradBasis;
   Matrix3x3         Jacobian;
   ThreeVector       Normal;
   FiniteElementMethod::BasisFunction basisFunction;

   getGeometryMatrix ( element, geometryMatrix );

   //
   // Integrate the Neumann boundary conditions using a Gauss-Lobatto quadrature.
   // This greatly simplifies the computation here, and is safe since the elements
   // here, in the basement, will never have degenerate segments.
   //
   for ( int i = 1; i <= 4; ++i ) {
      basisFunction ( xQuadraturePoints [ i - 1 ],
                      yQuadraturePoints [ i - 1 ],
                      zQuadraturePoint,
                      Basis, gradBasis );

      matrixMatrixProduct ( geometryMatrix, gradBasis, Jacobian );

      Normal ( 1 ) =  Jacobian ( 2, 2 ) * Jacobian ( 3, 1 );
      Normal ( 2 ) =  Jacobian ( 3, 2 ) * Jacobian ( 1, 1 );
      Normal ( 3 ) = -Jacobian ( 2, 2 ) * Jacobian ( 1, 1 );
      normalLength = FiniteElementMethod::length ( Normal );

      //
      // Since we are using a Gauss-Lobatto quadrature and the only non-zero entry will be
      // at the node then the calculation can be simplified to only adding in the contribution
      // from the boundary condition value and the normal vector in the z-direction.
      //
      elementVectorContributions ( i + 4 ) -= normalLength * bcs.getBoundaryConditionValue ( i - 1 + 4 );
   }

}

void TemperatureElementAssembly::applyDirichletBoundaryConditionsLinear ( const BoundaryConditions& bcs,
                                                                          const double              Dirichlet_Boundary_Scaling,
                                                                          ElementMatrix&            Stiffness_Matrix,
                                                                          ElementVector&            Load_Vector ) {

   for ( int i = 1; i <= 8; ++i ) {

      if ( bcs.getBoundaryCondition ( i - 1 ) == Surface_Boundary or
           bcs.getBoundaryCondition ( i - 1 ) == Interior_Constrained_Temperature ) {
         Load_Vector ( i ) = Dirichlet_Boundary_Scaling * bcs.getBoundaryConditionValue ( i - 1 );
         Stiffness_Matrix ( i, i ) = Dirichlet_Boundary_Scaling;
      }

   }

}


void TemperatureElementAssembly::compute ( const LayerElement&                       element,
                                           const GeoPhysics::CompoundLithology*      lithology,
                                           const double                              currentTime,
                                           const double                              timeStep,
                                           const bool                                includeAdvectionTerm,
                                           const BoundaryConditions&                 bcs,
                                           const bool                                includeChemicalCompaction,
                                           const FiniteElementMethod::ElementVector& elementHeatProduction,
                                           FiniteElementMethod::ElementMatrix&       elementStiffnessMatrix,
                                           FiniteElementMethod::ElementVector&       elementLoadVector ) {

   double timeStepInv;

   if ( timeStep == 0.0 ) {
      timeStepInv  = 0.0;
   } else {
      timeStepInv  = 1.0 / ( timeStep * MillionYearToSecond );
   }

   bool isBasementFormation = element.getFormation()->isBasement();
   bool elementHasFluid = ( element.getFluid () != nullptr );

   loadProperties ( element, elementHeatProduction, includeChemicalCompaction, includeAdvectionTerm );
   computeDerivedProperties ( element, lithology, includeChemicalCompaction, isBasementFormation, includeAdvectionTerm );

   Matrix3x3    conductivityTensor;
   ThreeVector  fluidFlux;
   ThreeVector  jacobianInverseFluidVelocity;
   double       integrationWeight;

   double previousTemperature;
   double previousPorosity;
   double currentPorosity;
   double previousBulkDensityTimesCapacity;
   double currentBulkDensityTimesCapacity;
   double heatSourceTerm;
   double loadTerms;

   elementLoadVector.zero ();
   elementStiffnessMatrix.zero ();

   ArrayDefs::ConstReal_ptr quadratureWeights = getScalarPropertyVector ( QuadratureWeights );
   ArrayDefs::ConstReal_ptr heatProductionVec = m_currentProperties.getProperty ( FiniteElementMethod::HeatProd );

   ArrayDefs::ConstReal_ptr previousTemperatureVec = m_previousProperties.getProperty ( FiniteElementMethod::Temp );

   ArrayDefs::ConstReal_ptr currentPorosityVec = ( includeAdvectionTerm and elementHasFluid ? m_multiComponentProperty.getMixedData () : getScalarPropertyVector ( CurrentPorosity ));
   ArrayDefs::ConstReal_ptr previousPorosityVec = getScalarPropertyVector ( PreviousPorosity );

   ArrayDefs::ConstReal_ptr currentBulkDensityTimesCapacityVec  = getScalarPropertyVector ( CurrentBulkDensityTimesHeatCap );
   ArrayDefs::ConstReal_ptr previousBulkDensityTimesCapacityVec = getScalarPropertyVector ( PreviousBulkDensityTimesHeatCap );

   ArrayDefs::ConstReal_ptr currentBulkThermalConductivityNormalVec = getScalarPropertyVector ( CurrentBulkThermalConductivityNormal );
   ArrayDefs::ConstReal_ptr currentBulkThermalConductivityPlaneVec  = getScalarPropertyVector ( CurrentBulkThermalConductivityPlane );
   ArrayDefs::ConstReal_ptr currentFluidDensityTimesHeatCapacityVec = ( elementHasFluid ? getScalarPropertyVector ( CurrentFluidDensityTimesHeatCap ) : nullptr );

   ArrayDefs::Real_ptr loadVectorWorkSpace = getScalarPropertyVector ( LoadVectorWorkSpace );
   ArrayDefs::Real_ptr stiffnessMatrixScalarWorkSpace = getScalarPropertyVector ( StiffnessMatrixScalarWorkSpace );

   const double* currentFluidViscosity = ( elementHasFluid ? getScalarPropertyVector ( CurrentFluidViscosity ) : nullptr );
   const double* currentPermeabilityNormal = ( includeAdvectionTerm and elementHasFluid ? getScalarPropertyVector ( CurrentPermeabilityNormal ) : nullptr );
   const double* currentPermeabilityPlane = ( includeAdvectionTerm and elementHasFluid ? getScalarPropertyVector ( CurrentPermeabilityPlane ) : nullptr );

   for ( int count = 0; count < m_basisFunctions.getNumberOfQuadraturePoints (); ++count ) {

      integrationWeight = quadratureWeights [ count ] * m_jacobianStorage.getDeterminant ( count );

      //
      // Term 1
      //
      currentPorosity = currentPorosityVec [ count ];
      previousPorosity = previousPorosityVec [ count ];
      previousTemperature = previousTemperatureVec [ count ];
      currentBulkDensityTimesCapacity  = currentBulkDensityTimesCapacityVec [ count ];
      previousBulkDensityTimesCapacity = previousBulkDensityTimesCapacityVec [ count ];

      double t1 = currentBulkDensityTimesCapacity / ( 1.0 - currentPorosity ) * ( currentPorosity - previousPorosity );
      double t2 = ( 2.0 * currentBulkDensityTimesCapacity - previousBulkDensityTimesCapacity );
      double t3 = previousBulkDensityTimesCapacity * previousTemperature;

      //
      // Term 4
      //
      heatSourceTerm = heatProductionVec [ count ];
      loadTerms = heatSourceTerm + t3 * timeStepInv;

      // Update the load vector with Source terms and previous time step term.
      loadVectorWorkSpace [ count ] = integrationWeight * loadTerms;

      //----------------------------//
      //
      // Stiffness Matrix
      //

      //
      // Term 1
      //
      stiffnessMatrixScalarWorkSpace [ count ] = integrationWeight * ( t1 + t2 ) * timeStepInv;

      //
      // Term 3
      //
      setTensor ( currentBulkThermalConductivityNormalVec [ count ],
                  currentBulkThermalConductivityPlaneVec [ count ],
                  m_jacobianStorage.getJacobian11 (),
                  m_jacobianStorage.getJacobian22 (),
                  m_jacobianStorage.getJacobian3 ( count ),
                  conductivityTensor );

      computeProduct ( m_jacobianStorage.getInverseJacobian11 (),
                       m_jacobianStorage.getInverseJacobian22 (),
                       m_jacobianStorage.getInverseJacobian3 ( count ),
                       integrationWeight,
                       conductivityTensor,
                       m_gradBasisMultipliers [ count ]);

      // Add in the Advection term if required.
      if ( includeAdvectionTerm and elementHasFluid ) {
         double relativePermeability = element.getFluid ()->relativePermeability();

         bool hasFractured = false;

         computeFluidFlux ( true,
                            hasFractured,
                            currentPermeabilityNormal [ count ],
                            currentPermeabilityPlane [ count ],
                            currentFluidViscosity [ count ],
                            m_jacobianStorage.getJacobian11 (),
                            m_jacobianStorage.getJacobian22 (),
                            m_jacobianStorage.getJacobian3 ( count ),
                            m_gradPo [ count ],
                            relativePermeability,
                            lithology,
                            fluidFlux );

         fluidFlux ( 1 ) = currentFluidDensityTimesHeatCapacityVec [ count ] * integrationWeight * fluidFlux ( 1 );
         fluidFlux ( 2 ) = currentFluidDensityTimesHeatCapacityVec [ count ] * integrationWeight * fluidFlux ( 2 );
         fluidFlux ( 3 ) = currentFluidDensityTimesHeatCapacityVec [ count ] * integrationWeight * fluidFlux ( 3 );

         // Make use of the fact that the Jacobian has a particular form: [[a,0,0], [0,b,0], [c,d,e]]
         jacobianInverseFluidVelocity ( 1 ) = m_jacobianStorage.getInverseJacobian11 () * fluidFlux ( 1 );
         jacobianInverseFluidVelocity ( 2 ) = m_jacobianStorage.getInverseJacobian22 () * fluidFlux ( 2 );
         jacobianInverseFluidVelocity ( 3 ) = innerProduct ( m_jacobianStorage.getInverseJacobian3 ( count ), fluidFlux );

         m_advectionMultipliers [ count ] = jacobianInverseFluidVelocity;

      }

   }

   mvp ( 1.0, m_basisFunctions.getBasisFunctions (), 1.0, loadVectorWorkSpace, elementLoadVector.data ());

   FiniteElementMethod::ArrayOperations::scaleBases ( m_basisFunctions.getBasisFunctions (), getScalarPropertyVector ( StiffnessMatrixScalarWorkSpace ), m_scaledBasis );
   matmult ( Numerics::NO_TRANSPOSE, Numerics::TRANSPOSE, 1.0, m_basisFunctions.getBasisFunctions (), m_scaledBasis, 1.0, elementStiffnessMatrix.C_Array ());

   if ( includeAdvectionTerm and elementHasFluid ) {
      FiniteElementMethod::ArrayOperations::scaleGradBases ( m_basisFunctions.getGradBasisFunctions (), m_advectionMultipliers, m_scaledBasis );
      matmult ( Numerics::NO_TRANSPOSE, Numerics::TRANSPOSE, 1.0, m_basisFunctions.getBasisFunctions (), m_scaledBasis, 1.0, elementStiffnessMatrix.C_Array ());
   }

   FiniteElementMethod::ArrayOperations::scaleGradBases ( m_basisFunctions.getGradBasisFunctions (), m_gradBasisMultipliers, m_scaledGradBasis );
   matmult ( Numerics::NO_TRANSPOSE, Numerics::TRANSPOSE, 1.0, m_basisFunctions.getGradBasisFunctions (), m_scaledGradBasis, 1.0, elementStiffnessMatrix.C_Array ());

   // Need the Neumann BCs for the heat flow.
   if ( bcs.getBoundaryCondition ( 4 ) == Bottom_Boundary_Flux ) {

      // If one bottom node is Neumann then all nodes will be.
      applyHeatFlowBoundaryConditions ( element, bcs, elementLoadVector );
   }

   applyDirichletBoundaryConditionsLinear ( bcs, Dirichlet_Scaling_Value, elementStiffnessMatrix, elementLoadVector );
}
