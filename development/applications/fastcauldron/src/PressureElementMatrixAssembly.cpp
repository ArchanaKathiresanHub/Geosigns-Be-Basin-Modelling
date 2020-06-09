//
// Copyright (C) 2017 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "PressureElementMatrixAssembly.h"

#include "GeoPhysicsFluidType.h"

#include "ConstantsMathematics.h"

#include "FastcauldronSimulator.h"
#include "property_manager.h"
#include "ElementContributions.h"
#include "Lithology.h"
#include "Quadrature.h"

using namespace FiniteElementMethod;
using namespace DataAccess;
using namespace GeoPhysics;
using namespace Utilities;
using namespace Maths;

PressureElementMatrixAssembly::PressureElementMatrixAssembly ( const BasisFunctionCache& basisFunctions ) :
   m_basisFunctions ( basisFunctions ),
   m_previousProperties ( basisFunctions.getBasisFunctions ()),
   m_currentProperties ( basisFunctions.getBasisFunctions ()),
   m_evaluatedProperties ( basisFunctions.getNumberOfQuadraturePoints ()),
   m_evaluatedGradProperties ( 3 * basisFunctions.getNumberOfQuadraturePoints ()),
   m_brinePhaseState ( basisFunctions.getNumberOfQuadraturePoints (), 0.0 ),
   m_permeabilityWorkSpace ( basisFunctions.getNumberOfQuadraturePoints ()),
   m_jacobianStorage ( basisFunctions.getNumberOfQuadraturePoints ()),
   m_gradPo ( basisFunctions.getNumberOfQuadraturePoints ()),
   m_gradPh ( basisFunctions.getNumberOfQuadraturePoints ()),
   m_gradBasisMultipliers ( basisFunctions.getNumberOfQuadraturePoints ()),
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

void PressureElementMatrixAssembly::loadProperties ( const LayerElement& element,
                                                     const bool          includeChemicalCompaction,
                                                     ElementVector&      currentPo ) {

   const CauldronGridDescription& grid = FastcauldronSimulator::getInstance ().getCauldronGridDescription ();
   double deltaX  = grid.deltaI;
   double deltaY  = grid.deltaJ;

   ElementVector currentPh;
   ElementVector currentPl;
   ElementVector currentPp;
   ElementVector currentElementVES;
   ElementVector currentElementMaxVES;
   ElementVector currentChemicalCompaction;
   ElementVector currentElementTemperature;
   ElementVector currentDepth;

   ElementVector previousChemicalCompaction;
   ElementVector previousPh;
   ElementVector previousPo;
   ElementVector previousPp;
   ElementVector previousElementVES;
   ElementVector previousElementMaxVES;
   ElementVector previousElementTemperature;

   getCoefficients ( element, Basin_Modelling::Depth,                currentDepth );
   getCoefficients ( element, Basin_Modelling::Hydrostatic_Pressure, currentPh );
   getCoefficients ( element, Basin_Modelling::Overpressure,         currentPo );
   getCoefficients ( element, Basin_Modelling::VES_FP,               currentElementVES );
   getCoefficients ( element, Basin_Modelling::Max_VES,              currentElementMaxVES );
   getCoefficients ( element, Basin_Modelling::Temperature,          currentElementTemperature );
   add ( currentPo, currentPh, currentPp );

   getPreviousCoefficients ( element, Basin_Modelling::Hydrostatic_Pressure, previousPh );
   getPreviousCoefficients ( element, Basin_Modelling::Overpressure,         previousPo );
   getPreviousCoefficients ( element, Basin_Modelling::VES_FP,               previousElementVES );
   getPreviousCoefficients ( element, Basin_Modelling::Max_VES,              previousElementMaxVES );
   getPreviousCoefficients ( element, Basin_Modelling::Temperature,          previousElementTemperature );
   add ( previousPo, previousPh, previousPp );

   if ( includeChemicalCompaction ) {
      getCoefficients ( element, Basin_Modelling::Chemical_Compaction, currentChemicalCompaction );
      getPreviousCoefficients ( element, Basin_Modelling::Chemical_Compaction, previousChemicalCompaction );
   } else {
      previousChemicalCompaction.zero ();
      currentChemicalCompaction.zero ();
   }

   m_previousProperties.compute ( FiniteElementMethod::Ph,       previousPh,
                                  FiniteElementMethod::Po,       previousPo,
                                  FiniteElementMethod::Pp,       previousPp,
                                  FiniteElementMethod::Ves,      previousElementVES,
                                  FiniteElementMethod::MaxVes,   previousElementMaxVES,
                                  FiniteElementMethod::ChemComp, previousChemicalCompaction,
                                  FiniteElementMethod::Temp,     previousElementTemperature );

   m_currentProperties.compute ( FiniteElementMethod::Ph,       currentPh,
                                 FiniteElementMethod::Po,       currentPo,
                                 FiniteElementMethod::Pp,       currentPp,
                                 FiniteElementMethod::Ves,      currentElementVES,
                                 FiniteElementMethod::MaxVes,   currentElementMaxVES,
                                 FiniteElementMethod::ChemComp, currentChemicalCompaction,
                                 FiniteElementMethod::Temp,     currentElementTemperature );

   m_jacobianStorage.set ( deltaX, deltaY, currentDepth, m_basisFunctions.getGradBasisFunctions ());

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

void PressureElementMatrixAssembly::computeProperties ( const LayerElement& element,
                                                        const bool          includeChemicalCompaction ) {

   // Compute all brine properties.
   m_brinePhaseState.setSalinity ( element.getFluid ()->salinity ());

   m_brinePhaseState.set ( m_basisFunctions.getNumberOfQuadraturePoints (),
                           m_currentProperties.getProperty  ( FiniteElementMethod::Temp ),
                           m_currentProperties.getProperty  ( FiniteElementMethod::Pp ));

   element.getFluid ()->density ( m_brinePhaseState, getScalarPropertyVector ( CurrentFluidDensity ));
   element.getFluid ()->computeDensityDerivativeWRTPressure ( m_brinePhaseState, getScalarPropertyVector ( CurrentFluidDensityDerivativeWRTPressure ));
   element.getFluid ()->viscosity ( m_brinePhaseState, getScalarPropertyVector ( CurrentFluidViscosity ));
   m_brinePhaseState.set ( m_basisFunctions.getNumberOfQuadraturePoints (),
                           m_previousProperties.getProperty  ( FiniteElementMethod::Temp ),
                           m_previousProperties.getProperty  ( FiniteElementMethod::Pp ));
   element.getFluid ()->density ( m_brinePhaseState, getScalarPropertyVector ( PreviousFluidDensity ));

   // Compute all lithology properties.
   m_multiComponentProperty.resize ( element.getLithology ()->getNumberOfSimpleLithologies (), m_evaluatedProperties.getNumberOfValues ());
   element.getLithology ()->getPorosity ( m_basisFunctions.getNumberOfQuadraturePoints (),
                                          m_currentProperties.getProperty ( FiniteElementMethod::Ves ),
                                          m_currentProperties.getProperty ( FiniteElementMethod::MaxVes ),
                                          includeChemicalCompaction,
                                          m_currentProperties.getProperty ( FiniteElementMethod::ChemComp ),
                                          m_multiComponentProperty,
                                          getScalarPropertyVector ( CurrentPorosityDerivative ));

   element.getLithology ()->getPorosity ( m_basisFunctions.getNumberOfQuadraturePoints (),
                                          m_previousProperties.getProperty ( FiniteElementMethod::Ves ),
                                          m_previousProperties.getProperty ( FiniteElementMethod::MaxVes ),
                                          includeChemicalCompaction,
                                          m_previousProperties.getProperty ( FiniteElementMethod::ChemComp ),
                                          getScalarPropertyVector ( PreviousPorosity ));

   // permeability work space only needed here,
   element.getLithology ()->calcBulkPermeabilityNP ( m_basisFunctions.getNumberOfQuadraturePoints (),
                                                     m_currentProperties.getProperty ( FiniteElementMethod::Ves ),
                                                     m_currentProperties.getProperty ( FiniteElementMethod::MaxVes ),
                                                     m_multiComponentProperty,
                                                     getScalarPropertyVector ( CurrentPermeabilityNormal ),
                                                     getScalarPropertyVector ( CurrentPermeabilityPlane ),
                                                     m_permeabilityWorkSpace );

}

//------------------------------------------------------------//

inline void PressureElementMatrixAssembly::computeNormal ( const double jac11,
                                                           const double jac22,
                                                           const ThreeVector&   jacRow3,
                                                           ThreeVector& Normal ) {

   //
   // Normal = R1 x R2 -- vector cross product
   //
   // The Jacobian has a simplified structure, so the cross-product of
   // the two columns can be simplified.
   Normal ( 1 ) =  jac22 * jacRow3 ( 1 );
   Normal ( 2 ) =  jacRow3 ( 2 ) * jac11;
   Normal ( 3 ) = -jac22 * jac11;

}

//------------------------------------------------------------//

void PressureElementMatrixAssembly::setTensor ( const double       valueNormal,
                                                const double       valuePlane,
                                                const double       jac11,
                                                const double       jac22,
                                                const ThreeVector& jacRow3,
                                                Matrix3x3&         tensor ) {

  ThreeVector normal;
  double      normalLengthSq;
  double      valueDifference;

  computeNormal ( jac11, jac22, jacRow3, normal );
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

void PressureElementMatrixAssembly::computeFluidMobilityTerms ( const bool               hasFractured,
                                                                const double             fractureScaling,
                                                                const double             permeabilityNormal,
                                                                const double             permeabilityPlane,
                                                                const double             fluidDensity,
                                                                const double             fluidViscosity,
                                                                const double             relativePermeability,
                                                                const double             jac11,
                                                                const double             jac22,
                                                                const ThreeVector&       jacRow3,
                                                                const ThreeVector&       gradOverpressure,
                                                                const CompoundLithology* lithology,
                                                                ThreeVector&             fluidVelocity,
                                                                Matrix3x3&               fluidMobility ) {

   double permeabilityScaling;

   // Why square-root ( 10 )?
   static const double Sqrt10 = sqrt ( 10.0 );

   if ( hasFractured ) {
      permeabilityScaling = lithology->fracturedPermeabilityScaling () * pow ( Sqrt10, 0.25 * fractureScaling );
   } else {
      permeabilityScaling = 1.0;
   }

   setTensor ( permeabilityNormal * permeabilityScaling * relativePermeability * fluidDensity / fluidViscosity,
               permeabilityPlane * relativePermeability * fluidDensity / fluidViscosity,
               jac11, jac22, jacRow3, fluidMobility );
   matrixVectorProduct ( fluidMobility, gradOverpressure, fluidVelocity );

   // Need to scale the Fluid Velocity by MegaPaToPa here, because the overpressure value
   // has been scaled by PaToMegaPa for the fluid_density function (because it is a function
   // of pressure in MPa)
   fluidVelocity *= MegaPaToPa;
}


void PressureElementMatrixAssembly::applyDirichletBoundaryConditions ( const BoundaryConditions&  bcs,
                                                                       const double               dirichletBoundaryScaling,
                                                                       const double               conversionFactor,
                                                                       const ElementVector&       currentPropertyValues,
                                                                       ElementMatrix&             matrix,
                                                                       ElementVector&             vector ) {

  for ( int i = 1; i <= 8; ++i ) {

     if ( bcs.getBoundaryCondition ( i - 1 ) == Surface_Boundary or
          bcs.getBoundaryCondition ( i - 1 ) == Interior_Constrained_Temperature or
          bcs.getBoundaryCondition ( i - 1 ) == Interior_Constrained_Overpressure ) {
        vector ( i ) = dirichletBoundaryScaling * ( bcs.getBoundaryConditionValue ( i - 1 ) - currentPropertyValues ( i )) * conversionFactor;
        matrix ( i, i ) = dirichletBoundaryScaling;
     }

  }

}

void PressureElementMatrixAssembly::computeProduct ( const double       jac11,
                                                     const double       jac22,
                                                     const ThreeVector& jacRow3,
                                                     const double       integrationScaling,
                                                     const Matrix3x3&   fluidMobility,
                                                     Matrix3x3&         result ) {

   Matrix3x3 intermediateMatrix;

   intermediateMatrix ( 1, 1 ) = jac11 * fluidMobility ( 1, 1 );
   intermediateMatrix ( 1, 2 ) = jac11 * fluidMobility ( 1, 2 );
   intermediateMatrix ( 1, 3 ) = jac11 * fluidMobility ( 1, 3 );

   intermediateMatrix ( 2, 1 ) = jac22 * fluidMobility ( 2, 1 );
   intermediateMatrix ( 2, 2 ) = jac22 * fluidMobility ( 2, 2 );
   intermediateMatrix ( 2, 3 ) = jac22 * fluidMobility ( 2, 3 );

   intermediateMatrix ( 3, 1 ) = jacRow3 ( 1 ) * fluidMobility ( 1, 1 ) + jacRow3 ( 2 ) * fluidMobility ( 2, 1 ) + jacRow3 ( 3 ) * fluidMobility ( 3, 1 );
   intermediateMatrix ( 3, 2 ) = jacRow3 ( 1 ) * fluidMobility ( 1, 2 ) + jacRow3 ( 2 ) * fluidMobility ( 2, 2 ) + jacRow3 ( 3 ) * fluidMobility ( 3, 2 );
   intermediateMatrix ( 3, 3 ) = jacRow3 ( 1 ) * fluidMobility ( 1, 3 ) + jacRow3 ( 2 ) * fluidMobility ( 2, 3 ) + jacRow3 ( 3 ) * fluidMobility ( 3, 3 );

   result ( 1, 1 ) = integrationScaling * jac11 * intermediateMatrix ( 1, 1 );
   result ( 1, 2 ) = integrationScaling * jac11 * intermediateMatrix ( 2, 1 );
   result ( 1, 3 ) = integrationScaling * jac11 * intermediateMatrix ( 3, 1 );

   result ( 2, 1 ) = integrationScaling * jac22 * intermediateMatrix ( 1, 2 );
   result ( 2, 2 ) = integrationScaling * jac22 * intermediateMatrix ( 2, 2 );
   result ( 2, 3 ) = integrationScaling * jac22 * intermediateMatrix ( 3, 2 );

   result ( 3, 1 ) = integrationScaling * ( jacRow3 ( 1 ) * intermediateMatrix ( 1, 1 ) + jacRow3 ( 2 ) * intermediateMatrix ( 1, 2 ) + jacRow3 ( 3 ) * intermediateMatrix ( 1, 3 ));
   result ( 3, 2 ) = integrationScaling * ( jacRow3 ( 1 ) * intermediateMatrix ( 2, 1 ) + jacRow3 ( 2 ) * intermediateMatrix ( 2, 2 ) + jacRow3 ( 3 ) * intermediateMatrix ( 2, 3 ));
   result ( 3, 3 ) = integrationScaling * ( jacRow3 ( 1 ) * intermediateMatrix ( 3, 1 ) + jacRow3 ( 2 ) * intermediateMatrix ( 3, 2 ) + jacRow3 ( 3 ) * intermediateMatrix ( 3, 3 ));
}


void PressureElementMatrixAssembly::compute ( const LayerElement&                    layerElement,
                                              const double                           currentTime,
                                              const double                           timeStep,
                                              const BoundaryConditions&              bcs,
                                              const bool                             isIceSheetLayer,
                                              const bool                             includeChemicalCompaction,
                                              const Interface::FracturePressureModel fractureModel,
                                              const ElementVector&                   fracturePressureExceeded,
                                              const bool                             includeWaterSaturation,
                                              const Saturation&                      currentSaturation,
                                              const Saturation&                      previousSaturation,
                                              ElementMatrix&                         elementJacobian,
                                              ElementVector&                         elementResidual ) {

   const Lithology* lithology = layerElement.getLithology ();

   // Would it be better to multiply the equation by the delta t rather than divide by it?
   const double timeStepInv = 1.0 / ( timeStep * MillionYearToSecond );

   ElementVector      elementCurrentPo;
   Matrix3x3          fluidMobility;
   ThreeVector        fluidVelocity;

   loadProperties ( layerElement, includeChemicalCompaction, elementCurrentPo );
   computeProperties ( layerElement, includeChemicalCompaction );

   elementJacobian.zero ();
   elementResidual.zero ();

   double fractureScaling = maxValue ( fracturePressureExceeded );

   // If non-conservative fracturing has been switched-on then Has-Fractured constant will
   // be assigned false because the permeabilities are not to be scaled in this fracturing model.
   const bool   hasFractured = ( fractureModel == Interface::NON_CONSERVATIVE_TOTAL ? false : ( fractureScaling > 0.0 ) );

   double usedWaterSaturation         = includeWaterSaturation ? currentSaturation ( Saturation::WATER )  : 1.0;
   double usedPreviousWaterSaturation = includeWaterSaturation ? previousSaturation ( Saturation::WATER ) : 1.0;

   // How to remove the cast here? Should be dynamic_cast anyway.
   double relativePermeability = ( includeWaterSaturation ? lithology->relativePermeability ( Saturation::WATER, currentSaturation ) : 1.0 );
   double currentRelativePermeability = relativePermeability * layerElement.getFluid()->relativePermeability();

   // dVes / dP = d(pL - P) / dP = -1
   const double dVesDp = -1.0;

   ArrayDefs::ConstReal_ptr quadratureWeights = getScalarPropertyVector ( QuadratureWeights );
   ArrayDefs::Real_ptr currentFluidDensityVec = getScalarPropertyVector ( CurrentFluidDensity );
   ArrayDefs::Real_ptr currentFluidDensityDerivativeWRTPressureVec = getScalarPropertyVector ( CurrentFluidDensityDerivativeWRTPressure );
   ArrayDefs::Real_ptr previousFluidDensityVec = getScalarPropertyVector ( PreviousFluidDensity );
   ArrayDefs::Real_ptr currentFluidViscosityVec = getScalarPropertyVector ( CurrentFluidViscosity );
   ArrayDefs::Real_ptr permeabilityNormalVec = getScalarPropertyVector ( CurrentPermeabilityNormal );
   ArrayDefs::Real_ptr permeabilityPlaneVec  = getScalarPropertyVector ( CurrentPermeabilityPlane );
   ArrayDefs::Real_ptr currentPorosityDerivative = getScalarPropertyVector ( CurrentPorosityDerivative );
   ArrayDefs::Real_ptr previousPorosityVec = getScalarPropertyVector ( PreviousPorosity );
   ArrayDefs::Real_ptr residualScalarWorkSpace = getScalarPropertyVector ( ResidualScalarWorkSpaceVector );
   ArrayDefs::Real_ptr residualVectorWorkSpace = getVectorPropertyVector ( ResidualVectorWorkSpaceVector );

   // The count3 variable has a stride of 3. It indexes the array of 3-vectors.
   for ( int count = 0, count3 = 0; count < m_basisFunctions.getNumberOfQuadraturePoints (); ++count, count3 += 3 ) {

      double integrationWeight = quadratureWeights [ count ] * m_jacobianStorage.getDeterminant ( count );
      double previousPorosity = previousPorosityVec [ count ];
      double currentPorosity = m_multiComponentProperty.getMixedData ()[ count ];
      double OneOverOneMinusPhi = 1.0 / ( 1.0 - currentPorosity );
      double previousFluidDensity = previousFluidDensityVec [ count ];
      double currentFluidDensity  = currentFluidDensityVec [ count ];

      //
      // Now compute each of the terms in the PDE
      //

      //
      //        d\rho      \rho    d \phi
      //   \phi ----- +  -------- -------
      //          dt     1 - \phi    dt
      //
      //
      //           d\rho     Sw \rho     d (\phi)
      //   Sw \phi ----- +  ---------  -----------
      //             dt      1 - \phi      dt
      //
      //
      //
      //
      //           d\rho      \rho    d Sw \phi
      //   Sw \phi ----- +  -------- ----------
      //             dt     1 - \phi      dt
      //
      //
      //           d\rho     Sw \rho     d (\phi)    \rho \phi d Sw
      //   Sw \phi ----- +  ---------  ----------- + --------- ----
      //             dt      1 - \phi      dt         1 - \phi  dt
      //
      //
      // Term 1
      //
      double currentFluidDensityTerm = integrationWeight * timeStepInv * (( usedWaterSaturation * currentFluidDensity * currentPorosity ) +
                                                                          usedWaterSaturation * currentFluidDensity * currentPorosity * OneOverOneMinusPhi );
      //
      // Term 2
      //
      double previousFluidDensityTerm = integrationWeight * timeStepInv * ( usedWaterSaturation * previousFluidDensity * currentPorosity +
                                                                            usedWaterSaturation * currentFluidDensity * previousPorosity * OneOverOneMinusPhi );


      //
      // Term 3
      //
      computeFluidMobilityTerms ( hasFractured,
                                  fractureScaling,
                                  permeabilityNormalVec [ count ],
                                  permeabilityPlaneVec [ count ],
                                  currentFluidDensityVec [ count ],
                                  currentFluidViscosityVec [ count ],
                                  currentRelativePermeability,
                                  m_jacobianStorage.getJacobian11 (),
                                  m_jacobianStorage.getJacobian22 (),
                                  m_jacobianStorage.getJacobian3 ( count ),
                                  m_gradPo [ count ],
                                  lithology,
                                  fluidVelocity,
                                  fluidMobility );

      computeProduct ( m_jacobianStorage.getInverseJacobian11 (),
                       m_jacobianStorage.getInverseJacobian22 (),
                       m_jacobianStorage.getInverseJacobian3 ( count ),
                       integrationWeight,
                       fluidMobility,
                       m_gradBasisMultipliers [ count ]);

      double Scaling = -currentFluidDensityTerm + previousFluidDensityTerm;

      residualScalarWorkSpace [ count ] = Scaling;

      const ThreeVector& jacobianRow3 = m_jacobianStorage.getInverseJacobian3 ( count );

      residualVectorWorkSpace [ count3     ] = -integrationWeight * ( m_jacobianStorage.getInverseJacobian11 () * fluidVelocity ( 1 ));
      residualVectorWorkSpace [ count3 + 1 ] = -integrationWeight * ( m_jacobianStorage.getInverseJacobian22 () * fluidVelocity ( 2 ));
      residualVectorWorkSpace [ count3 + 2 ] = -integrationWeight * ( jacobianRow3 ( 1 ) * fluidVelocity ( 1 ) + jacobianRow3 ( 2 ) * fluidVelocity ( 2 ) + jacobianRow3 ( 3 ) * fluidVelocity ( 3 ));

      //----------------------------//

      //
      // The fluid density derivative is scaled by PaToMegaPa because the fluid density function
      // requires the pressure to be in MPa. The pressure that is computed here is in Pa.
      //

      const double dPhiDP( currentPorosityDerivative [ count ] * dVesDp );

      double dRhoDP = PaToMegaPa * currentFluidDensityDerivativeWRTPressureVec [ count ];

      double bulkFluidDensityDerivative = dRhoDP * usedWaterSaturation * currentPorosity * OneOverOneMinusPhi +
                                          currentFluidDensity * usedWaterSaturation * OneOverOneMinusPhi * dPhiDP +
                                          currentFluidDensity * usedWaterSaturation * currentPorosity * OneOverOneMinusPhi * OneOverOneMinusPhi * dPhiDP;

      getScalarPropertyVector ( JacobianScalarWorkSpaceVector )[ count ] = integrationWeight * bulkFluidDensityDerivative * timeStepInv;
   }

   // Now use the terms that have been collected to contruct the Jacobian and residual.
   mvp ( 1.0, m_basisFunctions.getBasisFunctions (), 1.0, residualScalarWorkSpace, elementResidual.data ());
   mvp ( 1.0, m_basisFunctions.getGradBasisFunctions (), 1.0, residualVectorWorkSpace, elementResidual.data ());

   FiniteElementMethod::ArrayOperations::scaleBases ( m_basisFunctions.getBasisFunctions (), getScalarPropertyVector ( JacobianScalarWorkSpaceVector ), m_scaledBasis );
   matmult ( Numerics::NO_TRANSPOSE, Numerics::TRANSPOSE, 1.0, m_basisFunctions.getBasisFunctions (), m_scaledBasis, 1.0, elementJacobian.C_Array ());

   FiniteElementMethod::ArrayOperations::scaleGradBases ( m_basisFunctions.getGradBasisFunctions (), m_gradBasisMultipliers, m_scaledGradBasis );
   matmult ( Numerics::NO_TRANSPOSE, Numerics::TRANSPOSE, 1.0, m_basisFunctions.getGradBasisFunctions (), m_scaledGradBasis, 1.0, elementJacobian.C_Array ());

   applyDirichletBoundaryConditions ( bcs, Dirichlet_Scaling_Value, MegaPaToPa, elementCurrentPo,
                                      elementJacobian, elementResidual );

}
