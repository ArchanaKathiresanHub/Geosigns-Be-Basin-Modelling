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

#include "element_contributions.h"

//------------------------------------------------------------//

#include "BasisFunction.h"
#include "FiniteElement.h"
#include "ConstantsFastcauldron.h"
#include "utils.h"
#include "Lithology.h"
#include "PVTCalculator.h"

//------------------------------------------------------------//

#include "BasisFunction.h"
#include "NumericFunctions.h"
#include "Matrix8x8.h"
#include "MatrixSparse.h"

//------------------------------------------------------------//

#include "Lithology.h"

//------------------------------------------------------------//

#include "ConstantsMathematics.h"
using Utilities::Maths::MegaPaToPa;
using Utilities::Maths::PaToMegaPa;
using Utilities::Maths::MilliDarcyToM2;
using Utilities::Maths::YearToSecond;
using Utilities::Maths::MillionYearToSecond;
#include "ConstantsNumerical.h"
using Utilities::Numerical::CauldronNoDataValue;

using namespace FiniteElementMethod;

//------------------------------------------------------------//


void Basin_Modelling::Compute_Normal ( const Matrix3x3&   Jacobian,
                                             ThreeVector& Normal,
                                             ThreeVector& angles ) {

  ThreeVector Row_1;
  ThreeVector Row_2;
  ThreeVector Row_3;
  ThreeVector cross;

  //
  // These variables are called Row_Position this is not a mistake
  // we are taking the rows of the Jacobian, its just that the
  // Jaocbian is computed (and used here) as its transpose.
  //
  int Row_1_Position;
  int Row_2_Position;
  int Row_3_Position;

  Row_1_Position = 2;
  Row_2_Position = 1;
  Row_3_Position = 3;

  Row_1 ( 1 ) = Jacobian ( 1, Row_1_Position );
  Row_1 ( 2 ) = Jacobian ( 2, Row_1_Position );
  Row_1 ( 3 ) = Jacobian ( 3, Row_1_Position );

  Row_2 ( 1 ) = Jacobian ( 1, Row_2_Position );
  Row_2 ( 2 ) = Jacobian ( 2, Row_2_Position );
  Row_2 ( 3 ) = Jacobian ( 3, Row_2_Position );

  Row_3 ( 1 ) = Jacobian ( 1, Row_3_Position );
  Row_3 ( 2 ) = Jacobian ( 2, Row_3_Position );
  Row_3 ( 3 ) = Jacobian ( 3, Row_3_Position );

  // Normal = R1 x R2 -- vector cross product
  Normal ( 1 ) = Row_1 ( 2 ) * Row_2 ( 3 ) - Row_1 ( 3 ) * Row_2 ( 2 );
  Normal ( 2 ) = Row_1 ( 3 ) * Row_2 ( 1 ) - Row_1 ( 1 ) * Row_2 ( 3 );
  Normal ( 3 ) = Row_1 ( 1 ) * Row_2 ( 2 ) - Row_1 ( 2 ) * Row_2 ( 1 );

  double l1 = FiniteElementMethod::length ( Row_1 );
  double l2 = FiniteElementMethod::length ( Row_2 );
  double l3 = FiniteElementMethod::length ( Row_3 );



  cross ( 1 ) = Row_1 ( 2 ) * Row_2 ( 3 ) - Row_1 ( 3 ) * Row_2 ( 2 );
  cross ( 2 ) = -Row_1 ( 1 ) * Row_2 ( 3 ) + Row_1 ( 3 ) * Row_2 ( 1 );
  cross ( 3 ) = Row_1 ( 1 ) * Row_2 ( 2 ) - Row_1 ( 2 ) * Row_2 ( 1 );

  angles ( 1 ) = NumericFunctions::Minimum ( FiniteElementMethod::length ( cross ) / ( l1 * l2 ), 1.0 );
  angles ( 1 ) = asin ( angles ( 1 )) * 180 / M_PI;

  cross ( 1 ) = Row_1 ( 2 ) * Row_3 ( 3 ) - Row_1 ( 3 ) * Row_3 ( 2 );
  cross ( 2 ) = -Row_1 ( 1 ) * Row_3 ( 3 ) + Row_1 ( 3 ) * Row_3 ( 1 );
  cross ( 3 ) = Row_1 ( 1 ) * Row_3 ( 2 ) - Row_1 ( 2 ) * Row_3 ( 1 );

  angles ( 2 ) = NumericFunctions::Minimum ( FiniteElementMethod::length ( cross ) / ( l1 * l3 ), 1.0 );
  angles ( 2 ) = asin ( angles ( 2 )) * 180 / M_PI;

  cross ( 1 ) = Row_2 ( 2 ) * Row_3 ( 3 ) - Row_2 ( 3 ) * Row_3 ( 2 );
  cross ( 2 ) = -Row_2 ( 1 ) * Row_3 ( 3 ) + Row_2 ( 3 ) * Row_3 ( 1 );
  cross ( 3 ) = Row_2 ( 1 ) * Row_3 ( 2 ) - Row_2 ( 2 ) * Row_3 ( 1 );

  angles ( 3 ) = NumericFunctions::Minimum ( FiniteElementMethod::length ( cross ) / ( l2 * l3 ), 1.0 );
  angles ( 3 ) = asin ( angles ( 3 )) * 180 / M_PI;


}

//------------------------------------------------------------//

inline void Basin_Modelling::Compute_Normal ( const Matrix3x3&   Jacobian,
                                              ThreeVector& Normal ) {
   
   //
   // Normal = R1 x R2 -- vector cross product
   //
   Normal ( 1 ) =  Jacobian ( 2, 2 ) * Jacobian ( 3, 1 );
   Normal ( 2 ) =  Jacobian ( 3, 2 ) * Jacobian ( 1, 1 );
   Normal ( 3 ) = -Jacobian ( 2, 2 ) * Jacobian ( 1, 1 );

}
//------------------------------------------------------------//

void Basin_Modelling::setTensor ( const PetscScalar valueNormal,
                                  const PetscScalar valuePlane,
                                  const Matrix3x3&  jacobian,
                                  Matrix3x3&        tensor ) {

  ThreeVector normal;
  PetscScalar normalLength;
  PetscScalar valueDifference;

  Compute_Normal ( jacobian, normal );
  normalLength = FiniteElementMethod::length ( normal );

  valueDifference = valueNormal - valuePlane;
  valueDifference /= normalLength * normalLength;

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

//------------------------------------------------------------//


bool Basin_Modelling::Degenerate_Element ( const ElementGeometryMatrix& geometryMatrix ) {

  bool Is_Degenerate = true;
  int  I;

  for ( I = 1; I <= 4; I++ ) {
    Is_Degenerate = Is_Degenerate && ( fabs ( geometryMatrix ( 3, I ) - geometryMatrix ( 3, I + 4 )) < 0.001 );
  }

  return Is_Degenerate;
}

//------------------------------------------------------------//


void Basin_Modelling::computeFluidFlux ( const bool               imposeFluxLimit,
                                         const bool               hasFractured,
                                         const double             VES,
                                         const double             maxVES,
                                         const CompoundProperty&  porosity,
                                         const double             fluidViscosity,
                                         const Matrix3x3&         jacobian,
                                         const ThreeVector&       gradOverpressure,
                                         const double             relativePermeability,
                                         const CompoundLithology* lithology,
                                               ThreeVector&       fluidFlux ) {

  /// Limit the water flux to be within a reasonable range.
  /// This should be a depth related function, since it seems to be only
  /// the shallower parts where the high fluid fluxes cause a problem.
  const double MaximumFlux     = 1.0e-9;

  /// This limits the permeability to be within a reasonable range. The soil mechanics sandstone
  /// has a particularly high permeability 10^8.6 milli-Darcy (almost 6 orders of magnitude higher
  /// than the standard exponential sandstone, for example).
  const double MaximumPermeability = 10.0 * MilliDarcyToM2;

  double permeabilityNormal;
  double permeabilityPlane;
  double permeabilityScaling;

  /// Not quite the permeability tensor, since it is also divided by the fluid viscosity.
  Matrix3x3 permeabilityTensor;
  int I;

  if ( hasFractured ) {
    permeabilityScaling = lithology->fracturedPermeabilityScaling ();
  } else {
    permeabilityScaling = 1.0;
  }

  lithology->calcBulkPermeabilityNP ( VES, maxVES, porosity, permeabilityNormal, permeabilityPlane );

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

  setTensor ( permeabilityNormal, permeabilityPlane, jacobian, permeabilityTensor );

  matrixVectorProduct ( permeabilityTensor, gradOverpressure, fluidFlux );

  // Since pressure properties are stored in MPa units, we must convert to Pa to use in calculation.
  fluidFlux ( 1 ) = -fluidFlux ( 1 ) * MegaPaToPa;
  fluidFlux ( 2 ) = -fluidFlux ( 2 ) * MegaPaToPa;
  fluidFlux ( 3 ) = -fluidFlux ( 3 ) * MegaPaToPa;

  // Limit the fluid flux to some maximum value, if requested.
  if ( imposeFluxLimit ) {

    for ( I = 1; I <= 3; I++ ) {

      if ( fluidFlux ( I ) > MaximumFlux ) {
        fluidFlux ( I ) = MaximumFlux;
      } else if ( fluidFlux ( I ) < -MaximumFlux ) {
        fluidFlux ( I ) = -MaximumFlux;
      }

    }

  }

}


//------------------------------------------------------------//


void Basin_Modelling::Compute_Heat_Flow ( const bool                   isBasementFormation,
                                          const CompoundLithology*     lithology,
                                          const GeoPhysics::FluidType* Fluid,
                                          const ElementVector&         Temperature_Vector,
                                          const double                 Temperature_Value,
                                          const double                 Porosity,
                                          const double                 PorePressure,
                                          const double                 LithostaticPressure,
                                          const Matrix3x3&             Jacobian,
                                          const GradElementVector&     Grad_Basis,
                                                ThreeVector&           Heat_Flow,
                                                Matrix3x3&             Conductivity_Tensor ) {

  double Conductivity_Normal;
  double Conductivity_Tangential;

  if(isBasementFormation) {
     lithology -> calcBulkThermCondNPBasement ( Fluid, Porosity, Temperature_Value, LithostaticPressure, Conductivity_Normal, Conductivity_Tangential );
  } else {
     lithology -> calcBulkThermCondNP ( Fluid, Porosity, Temperature_Value, PorePressure, Conductivity_Normal, Conductivity_Tangential );
  }
  GradElementVector Grad_Basis2;

  setTensor ( Conductivity_Normal, Conductivity_Tangential, Jacobian, Conductivity_Tensor );
  matrixMatrixProduct ( Grad_Basis, Conductivity_Tensor, Grad_Basis2 );
  matrixTransposeVectorProduct ( Grad_Basis2, Temperature_Vector, Heat_Flow );
}


//------------------------------------------------------------//

void Basin_Modelling::assembleElementTemperatureResidual ( const bool                   isBasementFormation,
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
                                                           ElementVector&               elementResidual ) {

  double timeStepInv;

  if ( timeStep == 0.0 ) {
    timeStepInv  = 0.0;
  } else {
    timeStepInv  = 1.0 / ( timeStep * MillionYearToSecond );
  }

  NumericFunctions::Quadrature::QuadratureArray X_Quadrature_Points;
  NumericFunctions::Quadrature::QuadratureArray X_Quadrature_Weights;

  NumericFunctions::Quadrature::QuadratureArray Y_Quadrature_Points;
  NumericFunctions::Quadrature::QuadratureArray Y_Quadrature_Weights;

  NumericFunctions::Quadrature::QuadratureArray Z_Quadrature_Points;
  NumericFunctions::Quadrature::QuadratureArray Z_Quadrature_Weights;

  const int Number_Of_X_Points = planeQuadratureDegree;
  const int Number_Of_Y_Points = planeQuadratureDegree;
  const int Number_Of_Z_Points = depthQuadratureDegree;

  NumericFunctions::Quadrature::getInstance ().getGaussLegendreQuadrature ( Number_Of_X_Points, X_Quadrature_Points, X_Quadrature_Weights );
  NumericFunctions::Quadrature::getInstance ().getGaussLegendreQuadrature ( Number_Of_Y_Points, Y_Quadrature_Points, Y_Quadrature_Weights );
  NumericFunctions::Quadrature::getInstance ().getGaussLegendreQuadrature ( Number_Of_Z_Points, Z_Quadrature_Points, Z_Quadrature_Weights );

  int I, J, K;

  double integrationWeight;

  ThreeVector        Grad_Temperature;
  ThreeVector        Reference_Grad_Temperature;
  ThreeVector        Grad_Overpressure;
  ThreeVector        Reference_Grad_Overpressure;

  Matrix3x3          Fluid_Mobility;
  ThreeVector        fluidFlux;
  BasisFunction basisFunction;

  ThreeVector        gradHydrostaticPressure;
  ThreeVector        referenceGradHydrostaticPressure;

  double Fluid_Density_Heat_Capacity;
  double Fluid_Viscosity;

  ElementVector      Basis;
  ElementVector      Term_3;
  GradElementVector Grad_Basis;
  GradElementVector Scaled_Grad_Basis;

  Matrix3x3          Jacobian;
  Matrix3x3          Jacobian_Inverse;

  ThreeVector        Heat_Flow;
  Matrix3x3          Conductivity_Tensor;

  double Current_VES;
  double Current_Max_VES;
  double Current_Hydrostatic_Pressure;
  double Current_Overpressure;
  double Current_LithostaticPressure;
  double Current_Pore_Pressure;

  double Current_Temperature;
  double Previous_Temperature;

  double Current_Porosity;
  CompoundProperty Current_Compound_Porosity;

  double Current_Chemical_Compaction_Term;


  double Current_Bulk_Density_X_Capacity;
  double Heat_Source_Term;

  double Scaling;

  elementResidual.zero ();


  if ( Degenerate_Element ( geometryMatrix )) {
     //
     // This is only temporary until I sort out the whole degenerate segment thing!
     //
     applyDirichletBoundaryConditionsNewton ( bcs, Dirichlet_Scaling_Value, 1.0,
                                              Current_Element_Temperature, elementResidual );
    return;
  }


  for ( I = 0; I < Number_Of_X_Points; I++ ) {

    for ( J = 0; J < Number_Of_Y_Points; J++ ) {

      for ( K = 0; K < Number_Of_Z_Points; K++ ) {

        basisFunction ( X_Quadrature_Points [ I ],
                        Y_Quadrature_Points [ J ],
                        Z_Quadrature_Points [ K ],
                        Basis,
                        Grad_Basis );

        matrixMatrixProduct ( geometryMatrix, Grad_Basis, Jacobian );

        invert ( Jacobian, Jacobian_Inverse );

        matrixMatrixProduct ( Grad_Basis, Jacobian_Inverse, Scaled_Grad_Basis );

        integrationWeight = X_Quadrature_Weights [ I ] *
                             Y_Quadrature_Weights [ J ] *
                             Z_Quadrature_Weights [ K ] *
                             determinant ( Jacobian );


        //----------------------------//

        Current_VES  = FiniteElementMethod::innerProduct ( Basis, Current_Element_VES );

        Current_Max_VES  = FiniteElementMethod::innerProduct ( Basis, Current_Element_Max_VES );

        Current_Hydrostatic_Pressure  = FiniteElementMethod::innerProduct ( Basis, Current_Ph );

        Current_Overpressure  = FiniteElementMethod::innerProduct ( Basis, Current_Po );

        Current_LithostaticPressure  = FiniteElementMethod::innerProduct ( Basis, Current_Lp );

        Current_Pore_Pressure  = Current_Hydrostatic_Pressure  + Current_Overpressure;

        Previous_Temperature = FiniteElementMethod::innerProduct ( Basis, Previous_Element_Temperature );
        Current_Temperature  = FiniteElementMethod::innerProduct ( Basis, Current_Element_Temperature );

        Current_Chemical_Compaction_Term  = FiniteElementMethod::innerProduct ( Basis, Current_Element_Chemical_Compaction );

        lithology->getPorosity ( Current_VES, Current_Max_VES, includeChemicalCompaction, Current_Chemical_Compaction_Term, Current_Compound_Porosity );
        Current_Porosity = Current_Compound_Porosity.mixedProperty ();

        //----------------------------//
        //
        // Term 1
        //
        lithology -> calcBulkDensXHeatCapacity ( fluid,
                                                 Current_Porosity,
                                                 Current_Pore_Pressure,
                                                 Current_Temperature,
                                            Current_LithostaticPressure,
                                                 Current_Bulk_Density_X_Capacity );

        Scaling = Current_Bulk_Density_X_Capacity * Current_Temperature * integrationWeight * timeStepInv;
        Increment ( -Scaling, Basis, elementResidual );

        //
        // Term 2
        //
        Scaling = Current_Bulk_Density_X_Capacity * Previous_Temperature * integrationWeight * timeStepInv;
        Increment ( Scaling, Basis, elementResidual );

        //
        // Term 3
        //
        Compute_Heat_Flow ( isBasementFormation,
                            lithology,
                            fluid,
                            Current_Element_Temperature,
                            Current_Temperature,
                            Current_Porosity,
                            Current_Pore_Pressure,
                            Current_LithostaticPressure,
                            Jacobian,
                            Scaled_Grad_Basis,
                            Heat_Flow,
                            Conductivity_Tensor );

        matrixVectorProduct ( Scaled_Grad_Basis, Heat_Flow, Term_3 );
        Increment ( -integrationWeight, Term_3, elementResidual );

        //
        // Term 4
        //
        Heat_Source_Term = FiniteElementMethod::innerProduct ( Element_Heat_Production, Basis );
        Increment ( integrationWeight * Heat_Source_Term, Basis, elementResidual );


        //
        // Convection term, if required
        //
        if ( Include_Advection_Term && fluid != 0 ) {

          Fluid_Density_Heat_Capacity = fluid->densXheatCapacity ( Current_Temperature, Current_Pore_Pressure );
          Fluid_Viscosity = fluid->viscosity ( Current_Temperature, Current_Pore_Pressure  );

          matrixTransposeVectorProduct ( Grad_Basis, Current_Po, Reference_Grad_Overpressure );
          matrixTransposeVectorProduct ( Jacobian_Inverse, Reference_Grad_Overpressure, Grad_Overpressure );

          matrixTransposeVectorProduct ( Grad_Basis, Current_Element_Temperature, Reference_Grad_Temperature );
          matrixTransposeVectorProduct ( Jacobian_Inverse, Reference_Grad_Temperature, Grad_Temperature );

          matrixTransposeVectorProduct ( Grad_Basis, Current_Ph, referenceGradHydrostaticPressure );
          matrixTransposeVectorProduct ( Jacobian_Inverse, referenceGradHydrostaticPressure, gradHydrostaticPressure );

          Grad_Overpressure ( 1 ) += gradHydrostaticPressure ( 1 );
          Grad_Overpressure ( 2 ) += gradHydrostaticPressure ( 2 );

          //
          // DONT Know what to do about this!
          // Since the fluid is removed from the system if fracturing occurs.
          //
          bool Has_Fractured = false;

          computeFluidFlux ( true,
                             Has_Fractured,
                             Current_VES,
                             Current_Max_VES,
                             Current_Compound_Porosity,
                             Fluid_Viscosity,
                             Jacobian,
                             Grad_Overpressure,
                             1.0,
                             lithology,
                             fluidFlux );

          Increment ( -integrationWeight * Fluid_Density_Heat_Capacity * FiniteElementMethod::innerProduct ( Grad_Temperature, fluidFlux ),
                      Basis,
                      elementResidual );
        }

      }

    }

  }


  //
  // Need the Neumann BCs for the heat flow.
  //
  if ( bcs.getBoundaryCondition ( 4 ) == Bottom_Boundary_Flux ) {
    //
    //
    // If one bottom node is then all nodes will be.
    //
    applyHeatFlowBoundaryConditions ( geometryMatrix, bcs, elementResidual );
  }

  applyDirichletBoundaryConditionsNewton ( bcs, Dirichlet_Scaling_Value, 1.0,
                                           Current_Element_Temperature, elementResidual );

}

//------------------------------------------------------------//

void Basin_Modelling::assembleElementTemperatureSystem ( const bool                   isBasementFormation,
                                                         const int                    planeQuadratureDegree,
                                                         const int                    depthQuadratureDegree,
                                                         const double                 timeStep,
                                                         const bool                   includeAdvectionTerm,
                                                         const BoundaryConditions&    bcs,
                                                         const CompoundLithology*     lithology,
                                                         const GeoPhysics::FluidType* fluid,
                                                         const bool                   includeChemicalCompaction,
                                                         const ElementGeometryMatrix& geometryMatrix,
                                                         const ElementVector&         elementHeatProduction,
                                                         const ElementVector&         currentPh,
                                                         const ElementVector&         currentPo,
                                                         const ElementVector&         currentLp,
                                                         const ElementVector&         currentElementVes,
                                                         const ElementVector&         currentElementMaxVes,
                                                         const ElementVector&         previousElementTemperature,
                                                         const ElementVector&         currentElementTemperature,
                                                         const ElementVector&         currentElementChemicalCompaction,
                                                         ElementMatrix&               elementJacobian,
                                                         ElementVector&               elementResidual ) {


  double timeStepInv;


  if ( timeStep == 0.0 ) {
    timeStepInv  = 0.0;
  } else {
    timeStepInv  = 1.0 / ( timeStep * MillionYearToSecond );
  }

  NumericFunctions::Quadrature::QuadratureArray xQuadraturePoints;
  NumericFunctions::Quadrature::QuadratureArray xQuadratureWeights;

  NumericFunctions::Quadrature::QuadratureArray yQuadraturePoints;
  NumericFunctions::Quadrature::QuadratureArray yQuadratureWeights;

  NumericFunctions::Quadrature::QuadratureArray zQuadraturePoints;
  NumericFunctions::Quadrature::QuadratureArray zQuadratureWeights;

  const int numberOfXPoints = planeQuadratureDegree;
  const int numberOfYPoints = planeQuadratureDegree;
  const int numberOfZPoints = depthQuadratureDegree;

  NumericFunctions::Quadrature::getInstance ().getGaussLegendreQuadrature ( numberOfXPoints, xQuadraturePoints, xQuadratureWeights );
  NumericFunctions::Quadrature::getInstance ().getGaussLegendreQuadrature ( numberOfYPoints, yQuadraturePoints, yQuadratureWeights );
  NumericFunctions::Quadrature::getInstance ().getGaussLegendreQuadrature ( numberOfZPoints, zQuadraturePoints, zQuadratureWeights );

  int I, J, K;

  double integrationWeight;

  ThreeVector        gradTemperature;
  ThreeVector        referenceGradTemperature;
  ThreeVector        gradOverpressure;
  ThreeVector        referenceGradOverpressure;
  ThreeVector        gradHydrostaticPressure;
  ThreeVector        referenceGradHydrostaticPressure;

  ThreeVector        fluidFlux;
  ThreeVector        fluidFluxIntermediate;
  BasisFunction basisFunction;

  double fluidDensityHeatCapacity;
  double fluidViscosity;

  ElementVector      Basis;
  ElementVector      workSpace;
  ElementVector      term3;
  GradElementVector gradBasis;
  GradElementVector scaledGradBasis;
  GradElementVector scaledGradBasis2;

  Matrix3x3          jacobian;
  Matrix3x3          jacobianInverse;


  ThreeVector        heatFlow;
  Matrix3x3          conductivityTensor;

  double currentVes;
  double currentMaxVes;
  double currentHydrostaticPressure;
  double currentOverpressure;
  double currentLithostaticPressure;
  double currentPorePressure;
  double previousTemperature;
  double currentTemperature;
  double currentPorosity;
  CompoundProperty currentCompoundPorosity;

  double currentChemicalCompactionTerm;
  double currentBulkDensityXCapacity;
  double heatSourceTerm;
  double Scaling;

  elementResidual.zero ();
  elementJacobian.zero ();

  if ( Degenerate_Element ( geometryMatrix )) {
     //
     // This is only temporary until I sort out the whole degenerate segment thing!
     //
     applyDirichletBoundaryConditionsNewton ( bcs, Dirichlet_Scaling_Value, 1.0,
                                              currentElementTemperature, elementJacobian, elementResidual );
    return;
  }

  for ( I = 0; I < numberOfXPoints; I++ ) {

    for ( J = 0; J < numberOfYPoints; J++ ) {

      for ( K = 0; K < numberOfZPoints; K++ ) {

        basisFunction ( xQuadraturePoints [ I ],
                        yQuadraturePoints [ J ],
                        zQuadraturePoints [ K ],
                        Basis,
                        gradBasis );


        matrixMatrixProduct ( geometryMatrix, gradBasis, jacobian );

        invert ( jacobian, jacobianInverse );

        matrixMatrixProduct ( gradBasis, jacobianInverse, scaledGradBasis );

        integrationWeight = xQuadratureWeights [ I ] *
                            yQuadratureWeights [ J ] *
                            zQuadratureWeights [ K ] *
                            determinant ( jacobian );

        //----------------------------//

        currentVes                  = FiniteElementMethod::innerProduct ( Basis, currentElementVes );
        currentMaxVes               = FiniteElementMethod::innerProduct ( Basis, currentElementMaxVes );
        currentHydrostaticPressure  = FiniteElementMethod::innerProduct ( Basis, currentPh );
        currentOverpressure         = FiniteElementMethod::innerProduct ( Basis, currentPo );
        currentLithostaticPressure  = FiniteElementMethod::innerProduct ( Basis, currentLp );
        currentTemperature          = FiniteElementMethod::innerProduct ( Basis, currentElementTemperature );
        previousTemperature         = FiniteElementMethod::innerProduct ( Basis, previousElementTemperature );
        currentPorePressure        = currentHydrostaticPressure  + currentOverpressure;

        currentChemicalCompactionTerm  = FiniteElementMethod::innerProduct ( Basis, currentElementChemicalCompaction );

        lithology->getPorosity ( currentVes, currentMaxVes, includeChemicalCompaction, currentChemicalCompactionTerm, currentCompoundPorosity );
        currentPorosity = currentCompoundPorosity.mixedProperty ();

        //----------------------------//

        //
        // Term 1
        //
        lithology -> calcBulkDensXHeatCapacity ( fluid,
                                                 currentPorosity,
                                                 currentPorePressure,
                                                 currentTemperature,
                                                 currentLithostaticPressure,
                                                 currentBulkDensityXCapacity );

        Scaling = currentBulkDensityXCapacity * currentTemperature * integrationWeight * timeStepInv;
        Increment ( -Scaling, Basis, elementResidual );

        //
        // Term 2
        //
        Scaling = currentBulkDensityXCapacity * previousTemperature * integrationWeight * timeStepInv;
        Increment ( Scaling, Basis, elementResidual );

        //
        // Term 3
        //
        Compute_Heat_Flow ( isBasementFormation,
                            lithology,
                            fluid,
                            currentElementTemperature,
                            currentTemperature,
                            currentPorosity,
                            currentPorePressure,
                            currentLithostaticPressure,
                            jacobian,
                            scaledGradBasis,
                            heatFlow,
                            conductivityTensor );

        matrixVectorProduct ( scaledGradBasis, heatFlow, term3 );
        Increment ( -integrationWeight, term3, elementResidual );

        //
        // Term 4
        //
        heatSourceTerm = FiniteElementMethod::innerProduct ( elementHeatProduction, Basis );
        Increment ( integrationWeight * heatSourceTerm, Basis, elementResidual );

        //----------------------------//
        //
        // Jacobian
        //
        //

        //
        // Term 1
        //
        addOuterProduct ( integrationWeight * currentBulkDensityXCapacity * timeStepInv, Basis, Basis, elementJacobian );

        //
        // Term 3
        //
        matrixMatrixProduct ( scaledGradBasis, conductivityTensor, scaledGradBasis2 );
        addOuterProduct ( integrationWeight, scaledGradBasis, scaledGradBasis2, elementJacobian );

        //
        // Convection term, if required
        //
        if ( includeAdvectionTerm && fluid != 0 ) {

          fluidDensityHeatCapacity = fluid->densXheatCapacity ( currentTemperature, currentPorePressure );
          fluidViscosity = fluid->viscosity ( currentTemperature, currentPorePressure );

          matrixTransposeVectorProduct ( gradBasis, currentPo, referenceGradOverpressure );
          matrixTransposeVectorProduct ( jacobianInverse, referenceGradOverpressure, gradOverpressure );

          matrixTransposeVectorProduct ( gradBasis, currentElementTemperature, referenceGradTemperature );
          matrixTransposeVectorProduct ( jacobianInverse, referenceGradTemperature, gradTemperature );

          matrixTransposeVectorProduct ( gradBasis, currentPh, referenceGradHydrostaticPressure );
          matrixTransposeVectorProduct ( jacobianInverse, referenceGradHydrostaticPressure, gradHydrostaticPressure );

          gradOverpressure ( 1 ) += gradHydrostaticPressure ( 1 );
          gradOverpressure ( 2 ) += gradHydrostaticPressure ( 2 );

          //
          // DONT Know what to do about this!
          // Since the fluid is removed from the system if fracturing occurs.
          //
          bool Has_Fractured = false;

          computeFluidFlux ( true,
                             Has_Fractured,
                             currentVes,
                             currentMaxVes,
                             currentCompoundPorosity,
                             fluidViscosity,
                             jacobian,
                             gradOverpressure,
                             1.0,
                             lithology,
                             fluidFlux );

          Increment ( -integrationWeight * fluidDensityHeatCapacity * FiniteElementMethod::innerProduct ( gradTemperature, fluidFlux ),
                      Basis,
                      elementResidual );


          matrixVectorProduct ( jacobianInverse, fluidFlux, fluidFluxIntermediate );
          matrixVectorProduct ( gradBasis, fluidFluxIntermediate, workSpace );

          scale ( workSpace, -integrationWeight * currentPorosity * fluidDensityHeatCapacity );
          addOuterProduct ( workSpace, Basis, elementJacobian );
        }


      }

    }

  }


  //
  // Need the Neumann BCs for the heat flow.
  //
  if ( bcs.getBoundaryCondition ( 4 ) == Bottom_Boundary_Flux ) {
    //
    //
    // If one bottom node is then all nodes will be.
    //
    applyHeatFlowBoundaryConditions ( geometryMatrix, bcs, elementResidual );
  }

  applyDirichletBoundaryConditionsNewton ( bcs, Dirichlet_Scaling_Value, 1.0,
                                           currentElementTemperature, elementJacobian, elementResidual );

}

//------------------------------------------------------------//

void Basin_Modelling::applyDirichletBoundaryConditionsNewton ( const BoundaryConditions&  bcs,
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

//------------------------------------------------------------//

void Basin_Modelling::applyDirichletBoundaryConditionsNewton ( const BoundaryConditions&  bcs,
                                                               const double               dirichletBoundaryScaling,
                                                               const double               conversionFactor,
                                                               const ElementVector&       currentPropertyValues,
                                                               ElementVector&             vector ) {

  for ( int i = 1; i <= 8; ++i ) {

     if ( bcs.getBoundaryCondition ( i - 1 ) == Surface_Boundary or
          bcs.getBoundaryCondition ( i - 1 ) == Interior_Constrained_Temperature or
          bcs.getBoundaryCondition ( i - 1 ) == Interior_Constrained_Overpressure ) {
        vector ( i ) = dirichletBoundaryScaling * ( bcs.getBoundaryConditionValue ( i - 1 ) - currentPropertyValues ( i )) * conversionFactor;
     }

  }

}

//------------------------------------------------------------//


void Basin_Modelling::applyHeatFlowBoundaryConditions ( const ElementGeometryMatrix& geometryMatrix,
                                                        const BoundaryConditions&    bcs,
                                                        ElementVector&               ElementVector_Contributions ) {

  double xQuadraturePoints [ 4 ] = { -1.0,  1.0, 1.0, -1.0 };
  double yQuadraturePoints [ 4 ] = { -1.0, -1.0, 1.0,  1.0 };
  double zQuadraturePoint = 1.0;

  double            normalLength;
  ElementVector     Basis;
  GradElementVector gradBasis;
  Matrix3x3         Jacobian;
  ThreeVector       Normal;
  FiniteElementMethod::BasisFunction basisFunction;

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

    Compute_Normal ( Jacobian, Normal );
    normalLength = FiniteElementMethod::length ( Normal );

    //
    // Since we are using a Gauss-Lobatto quadrature and the only non-zero entry will be
    // at the node then the calculation can be simplified to only adding in the contribution
    // from the boundary condition value and the normal vector in the z-direction.
    //
    ElementVector_Contributions ( i + 4 ) -= normalLength * bcs.getBoundaryConditionValue ( i - 1 + 4 );
  }

}

//------------------------------------------------------------//
