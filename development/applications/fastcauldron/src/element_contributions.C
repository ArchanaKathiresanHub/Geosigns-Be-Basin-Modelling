//------------------------------------------------------------//

#include "element_contributions.h"

//------------------------------------------------------------//

#include "BasisFunction.h"
#include "FiniteElement.h"
#include "globaldefs.h"
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

using namespace FiniteElementMethod;

//------------------------------------------------------------//

void Basin_Modelling::Compute_Normal ( const Matrix3x3&   Jacobian,
                                             ThreeVector& Normal ) {

  ThreeVector column1;
  ThreeVector column2;

  int column1Position;
  int column2Position;

  column1Position = 2;
  column2Position = 1;

  column1 ( 1 ) = Jacobian ( 1, column1Position );
  column1 ( 2 ) = Jacobian ( 2, column1Position );
  column1 ( 3 ) = Jacobian ( 3, column1Position );

  column2 ( 1 ) = Jacobian ( 1, column2Position );
  column2 ( 2 ) = Jacobian ( 2, column2Position );
  column2 ( 3 ) = Jacobian ( 3, column2Position );

  //
  // Normal = R1 x R2 -- vector cross product
  //
  Normal ( 1 ) = column1 ( 2 ) * column2 ( 3 ) - column1 ( 3 ) * column2 ( 2 );
  Normal ( 2 ) = column1 ( 3 ) * column2 ( 1 ) - column1 ( 1 ) * column2 ( 3 );
  Normal ( 3 ) = column1 ( 1 ) * column2 ( 2 ) - column1 ( 2 ) * column2 ( 1 );

}

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

void Basin_Modelling::setTensor ( const PetscScalar valueNormal,
                                  const PetscScalar valuePlane,
                                  const Matrix3x3&  jacobian,
                                  Matrix3x3&        tensor ) {

  ThreeVector normal;
  PetscScalar normalLength;
  PetscScalar valueDifference;

  Compute_Normal ( jacobian, normal );
  normalLength = FiniteElementMethod::length ( normal );
  scale ( normal, 1.0 / normalLength );

  valueDifference = valueNormal - valuePlane;

  for ( int i = 1; i <= 3; ++i ) {

    for ( int j = 1; j <= 3; ++j ) {
       tensor ( i, j ) = valueDifference * normal ( i ) * normal ( j );
    }

  }

  for ( int i = 1; i <= 3; ++i ) {
     tensor ( i, i ) += valuePlane;
  }

}

//------------------------------------------------------------//

void Basin_Modelling::computeFluidMobilityTerms ( const bool                Has_Fractured,
                                                  const double              fractureScaling,
                                                  const double              VES,
                                                  const double              Max_VES,
                                                  const CompoundProperty&   Porosity,
                                                  const double              fluidDensity,
                                                  const double              fluidViscosity,
                                                  const double              relativePermeability,
                                                  const Matrix3x3&          Jacobian,
                                                  const ThreeVector&        gradOverpressure,
                                                  const CompoundLithology*  lithology,
                                                        ThreeVector&        Fluid_Velocity,
                                                        Matrix3x3&          Fluid_Mobility ) {

   double Permeability_Normal;
   double Permeability_Plane;
   double Permeability_Scaling;
   
   // Why square-root ( 10 )?
   static const double Sqrt10 = sqrt ( 10.0 );
   
   if ( Has_Fractured ) {
      Permeability_Scaling = lithology->fracturedPermeabilityScaling () * pow ( Sqrt10, 0.25 * fractureScaling );
   } else {
      Permeability_Scaling = 1.0;
   }
   
   lithology->calcBulkPermeabilityNP ( VES, Max_VES, Porosity, Permeability_Normal, Permeability_Plane );

   Permeability_Normal *=  Permeability_Scaling * relativePermeability * fluidDensity / fluidViscosity;
   Permeability_Plane  *=                         relativePermeability * fluidDensity / fluidViscosity;

   setTensor ( Permeability_Normal, Permeability_Plane, Jacobian, Fluid_Mobility );
   matrixVectorProduct ( Fluid_Mobility, gradOverpressure, Fluid_Velocity );

   // Need to scale the Fluid Velocity by MPa_To_Pa here, because the overpressure value
   // has been scaled by Pa_To_MPa for the fluid_density function (because it is a function
   // of pressure in MPa)
   Fluid_Velocity *= MPa_To_Pa;
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
  const double MaximumPermeability = 10.0 * MILLIDARCYTOM2;

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
  fluidFlux ( 1 ) = -fluidFlux ( 1 ) * MPa_To_Pa;
  fluidFlux ( 2 ) = -fluidFlux ( 2 ) * MPa_To_Pa;
  fluidFlux ( 3 ) = -fluidFlux ( 3 ) * MPa_To_Pa;

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

void Basin_Modelling::computeFluidVelocity
   ( const CompoundLithology*       lithology,
     const GeoPhysics::FluidType*   fluid,
     const bool                     includeChemicalCompaction,
     const ElementGeometryMatrix& geometryMatrix,
     const ElementVector&          currentElementVES,
     const ElementVector&          currentElementMaxVES,
     const ElementVector&          currentElementPh,
     const ElementVector&          currentElementPo,
     const ElementVector&          currentElementTemperature,
     const ElementVector&          currentElementChemicalCompaction,
     const double                  currentElementRelativePermeability,
           ThreeVector&            fluidVelocity ) {

   computeFluidVelocity ( 0.0, 0.0, 0.0,
                          lithology,
                          fluid,
                          includeChemicalCompaction,
                          geometryMatrix,
                          currentElementVES,
                          currentElementMaxVES,
                          currentElementPh,
                          currentElementPo,
                          currentElementTemperature,
                          currentElementChemicalCompaction,
                          currentElementRelativePermeability,
                          fluidVelocity );
}

//------------------------------------------------------------//

void Basin_Modelling::computeFluidVelocity
   ( const double                   x,
     const double                   y,
     const double                   z,
     const CompoundLithology*       lithology,
     const GeoPhysics::FluidType*   fluid,
     const bool                     includeChemicalCompaction,
     const ElementGeometryMatrix& geometryMatrix,
     const ElementVector&          currentElementVES,
     const ElementVector&          currentElementMaxVES,
     const ElementVector&          currentElementPh,
     const ElementVector&          currentElementPo,
     const ElementVector&          currentElementTemperature,
     const ElementVector&          currentElementChemicalCompaction,
     const double                  currentElementRelativePermeability,
           ThreeVector&            fluidVelocity ) {

  double temperature;
  double VES;
  double maxVES;
  double porosity;
  double fluidViscosity;
  double chemicalCompactionTerm;
  double relativePermeability;
  double porePressure;
  double overpressure;
  double hydrostaticPressure;

  CompoundProperty currentCompoundPorosity;

  ElementVector      basis;
  GradElementVector gradBasis;

  ThreeVector        gradOverpressure;
  ThreeVector        referenceGradOverpressure;
  ThreeVector        fluidFlux;

  ThreeVector        gradHydrostaticPressure;
  ThreeVector        referenceGradHydrostaticPressure;

  Matrix3x3 jacobian;
  Matrix3x3 jacobianInverse;
  FiniteElementMethod::BasisFunction basisFunction;

  bool hasFractured = false;

  //
  // Centre of the element
  //

  if ( Degenerate_Element ( geometryMatrix )) {
    //
    // This is only temporary until I sort out the whole degenerate segment thing!
    //
    fluidVelocity ( 1 ) = CAULDRONIBSNULLVALUE;
    fluidVelocity ( 2 ) = CAULDRONIBSNULLVALUE;
    fluidVelocity ( 3 ) = CAULDRONIBSNULLVALUE;
    return;
  } else if ( fluid == 0 or lithology->surfacePorosity () == 0.0 ) {
    fluidVelocity ( 1 ) = 0.0;
    fluidVelocity ( 2 ) = 0.0;
    fluidVelocity ( 3 ) = 0.0;
    return;
  }

  /// Evaluated at some point in the reference element.
  basisFunction ( x, y, z, basis, gradBasis );

  matrixMatrixProduct ( geometryMatrix, gradBasis, jacobian );
  invert ( jacobian, jacobianInverse );

  temperature = FiniteElementMethod::innerProduct ( currentElementTemperature, basis );
  VES         = FiniteElementMethod::innerProduct ( currentElementVES, basis );
  maxVES      = FiniteElementMethod::innerProduct ( currentElementMaxVES, basis );
  chemicalCompactionTerm = FiniteElementMethod::innerProduct ( currentElementChemicalCompaction, basis );
  hydrostaticPressure    = FiniteElementMethod::innerProduct ( currentElementPh,  basis ) * MPa_To_Pa;;
  overpressure           = FiniteElementMethod::innerProduct ( currentElementPo, basis  ) * MPa_To_Pa;

  lithology->getPorosity ( VES, maxVES, includeChemicalCompaction, chemicalCompactionTerm, currentCompoundPorosity );
  porosity = currentCompoundPorosity.mixedProperty ();
 

  porePressure = ( overpressure + hydrostaticPressure ) * Pa_To_MPa;
  fluidViscosity = fluid->viscosity ( temperature, porePressure );
  relativePermeability = fluid->relativePermeability ( temperature, porePressure ) * currentElementRelativePermeability;

  matrixTransposeVectorProduct ( gradBasis, currentElementPo, referenceGradOverpressure );
  matrixTransposeVectorProduct ( jacobianInverse, referenceGradOverpressure, gradOverpressure );

  matrixTransposeVectorProduct ( gradBasis, currentElementPh, referenceGradHydrostaticPressure );
  matrixTransposeVectorProduct ( jacobianInverse, referenceGradHydrostaticPressure, gradHydrostaticPressure );

  gradOverpressure ( 1 ) += gradHydrostaticPressure ( 1 );
  gradOverpressure ( 2 ) += gradHydrostaticPressure ( 2 );

  computeFluidFlux ( false,
                     hasFractured,
                     VES,
                     maxVES,
                     currentCompoundPorosity,
                     fluidViscosity,
                     jacobian,
                     gradOverpressure,
                     relativePermeability,
                     lithology,
                     fluidFlux );

  // Convert to mm/year.
  fluidVelocity ( 1 ) =  1000.0 * SecondsPerYear * fluidFlux ( 1 ) / porosity;
  fluidVelocity ( 2 ) =  1000.0 * SecondsPerYear * fluidFlux ( 2 ) / porosity;
  // +ve to represent upwards, so scale by -1
  fluidVelocity ( 3 ) = -1000.0 * SecondsPerYear * fluidFlux ( 3 ) / porosity;

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


void Basin_Modelling::computeHeatFlow
   ( const bool                     isBasementFormation,
     const double                   x,
     const double                   y,
     const double                   z,
     const CompoundLithology*       lithology,
     const GeoPhysics::FluidType*   fluid,
     const bool                     includeChemicalCompaction,
     const bool                     includeAdvectionTerm,
     const ElementGeometryMatrix& geometryMatrix,
     const ElementVector&          currentElementVES,
     const ElementVector&          currentElementMaxVES,
     const ElementVector&          currentElementPo,
     const ElementVector&          currentElementPp,
     const ElementVector&          currentElementLp,
     const ElementVector&          currentElementTemperature,
     const ElementVector&          currentElementChemicalCompaction,
           ThreeVector&            heatFlow ) {

  double conductivityNormal;
  double conductivityTangential;
  double temperature;
  double VES;
  double maxVES;
  double porosity;
  double chemicalCompactionTerm;
  double lithostaticPressure;
  double porePressure;
  CompoundProperty currentCompoundPorosity;

  ElementVector      basis;
  GradElementVector gradBasis;
  GradElementVector scaledGradBasis;
  GradElementVector gradBasis2;

  Matrix3x3 jacobian;
  Matrix3x3 jacobianInverse;
  Matrix3x3 conductivityTensor;
  FiniteElementMethod::BasisFunction basisFunction;
  //
  //
  // Centre of the element
  //

  if ( Degenerate_Element ( geometryMatrix )) {
    //
    // This is only temporary until I sort out the whole degenerate segment thing!
    //
    heatFlow ( 1 ) = CAULDRONIBSNULLVALUE;
    heatFlow ( 2 ) = CAULDRONIBSNULLVALUE;
    heatFlow ( 3 ) = CAULDRONIBSNULLVALUE;
    return;
  }

  ///
  /// Evaluated at centre of element
  ///
  basisFunction ( x, y, z, basis, gradBasis );

  matrixMatrixProduct ( geometryMatrix, gradBasis, jacobian );
  invert ( jacobian, jacobianInverse );
  matrixMatrixProduct ( gradBasis, jacobianInverse, scaledGradBasis );

  temperature = FiniteElementMethod::innerProduct ( currentElementTemperature, basis );
  VES         = FiniteElementMethod::innerProduct ( currentElementVES, basis );
  maxVES      = FiniteElementMethod::innerProduct ( currentElementMaxVES, basis );
  chemicalCompactionTerm = FiniteElementMethod::innerProduct ( currentElementChemicalCompaction, basis );
  lithostaticPressure = FiniteElementMethod::innerProduct ( currentElementLp, basis );
  porePressure = FiniteElementMethod::innerProduct ( currentElementPp, basis );

  lithology->getPorosity ( VES, maxVES, includeChemicalCompaction, chemicalCompactionTerm, currentCompoundPorosity );
  porosity = currentCompoundPorosity.mixedProperty ();
  if(isBasementFormation) {
     lithology -> calcBulkThermCondNPBasement ( fluid, porosity, temperature, lithostaticPressure, conductivityNormal, conductivityTangential );
  } else {
     lithology -> calcBulkThermCondNP ( fluid, porosity, temperature, porePressure, conductivityNormal, conductivityTangential );
  }     
  setTensor ( conductivityNormal, conductivityTangential, jacobian, conductivityTensor );
  matrixMatrixProduct ( scaledGradBasis, conductivityTensor, gradBasis2 );
  matrixTransposeVectorProduct ( gradBasis2, currentElementTemperature, heatFlow );

  //
  // Fouriers law states:  q = -k \grad T
  //
  scale ( heatFlow, -1.0 );

  if ( includeAdvectionTerm && fluid != 0 ) {

    double fluidDensity;
    double fluidViscosity;
    double heatCapacity;
    double advectionScaling;

    ThreeVector referenceGradOverpressure;
    ThreeVector gradOverpressure;
    ThreeVector fluidFlux;

    matrixTransposeVectorProduct ( gradBasis, currentElementPo, referenceGradOverpressure );
    matrixTransposeVectorProduct ( jacobianInverse, referenceGradOverpressure, gradOverpressure );
    //   porePressure = FiniteElementMethod::innerProduct ( currentElementPp, basis );

    fluidDensity   = fluid->density ( temperature, porePressure );
    heatCapacity   = fluid->heatCapacity ( temperature, porePressure );
    fluidViscosity = fluid->viscosity ( temperature, porePressure );

    bool hasFractured = false;

    computeFluidFlux ( true,
                       hasFractured,
                       VES,
                       maxVES,
                       currentCompoundPorosity,
                       fluidViscosity,
                       jacobian,
                       gradOverpressure,
                       1.0,
                       lithology,
                       fluidFlux );

    advectionScaling = fluidDensity * heatCapacity * temperature;

    heatFlow ( 1 ) = heatFlow ( 1 ) + advectionScaling * fluidFlux ( 1 );
    heatFlow ( 2 ) = heatFlow ( 2 ) + advectionScaling * fluidFlux ( 2 );
    heatFlow ( 3 ) = heatFlow ( 3 ) + advectionScaling * fluidFlux ( 3 );
  }

  heatFlow ( 3 ) = -heatFlow ( 3 );

  ///
  /// Convert from Watts to milli Watts. 
  ///
  scale ( heatFlow, 1000.0 );
}


//------------------------------------------------------------//


void Basin_Modelling::computeHeatFlow
   ( const bool                     isBasementFormation,
     const CompoundLithology*       lithology,
     const GeoPhysics::FluidType*   fluid,
     const bool                     includeChemicalCompaction,
     const bool                     includeAdvectionTerm,
     const ElementGeometryMatrix& geometryMatrix,
     const ElementVector&          currentElementVES,
     const ElementVector&          currentElementMaxVES,
     const ElementVector&          currentElementPo,
     const ElementVector&          currentElementPp,
     const ElementVector&          currentElementLp,
     const ElementVector&          currentElementTemperature,
     const ElementVector&          currentElementChemicalCompaction,
           ThreeVector&            heatFlow ) {

   computeHeatFlow ( isBasementFormation,
                     0.0, 0.0, 0.0, lithology, fluid,
                     includeChemicalCompaction,
                     includeAdvectionTerm,
                     geometryMatrix,
                     currentElementVES,
                     currentElementMaxVES,
                     currentElementPo,
                     currentElementPp,
                     currentElementLp,
                     currentElementTemperature,
                     currentElementChemicalCompaction,
                     heatFlow );
}

//------------------------------------------------------------//

void Basin_Modelling::assembleElementTemperatureResidual ( const bool                   isBasementFormation,
                                                           const int                    planeQuadratureDegree,
                                                           const int                    depthQuadratureDegree,
                                                           const double                 currentTime,
                                                           const double                 timeStep,
                                                           const bool                   Include_Advection_Term,
                                                           const BoundaryConditions&    bcs,
                                                           const CompoundLithology*     lithology,
                                                           const GeoPhysics::FluidType* fluid,
                                                           const bool                   includeChemicalCompaction,
                                                           const ElementGeometryMatrix& geometryMatrix,
                                                           const ElementVector&         Element_Heat_Production,
                                                           const ElementVector&         Previous_Ph,
                                                           const ElementVector&         Current_Ph,
                                                           const ElementVector&         Previous_Po,
                                                           const ElementVector&         Current_Po,
                                                           const ElementVector&         Current_Lp,
                                                           const ElementVector&         Previous_Element_VES,
                                                           const ElementVector&         Current_Element_VES,
                                                           const ElementVector&         Previous_Element_Max_VES,
                                                           const ElementVector&         Current_Element_Max_VES,
                                                           const ElementVector&         Previous_Element_Temperature,
                                                           const ElementVector&         Current_Element_Temperature,
                                                           const ElementVector&         Previous_Element_Chemical_Compaction,
                                                           const ElementVector&         Current_Element_Chemical_Compaction,
                                                           ElementVector&               elementResidual ) {

  double timeStepInv;

  if ( timeStep == 0.0 ) {
    timeStepInv  = 0.0;
  } else {
    timeStepInv  = 1.0 / ( timeStep * Secs_IN_MA );
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

  ThreeVector        gradSurfaceDepth;
  ThreeVector        referenceGradSurfaceDepth;

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

void Basin_Modelling::assembleElementTemperatureStiffnessMatrix ( const bool                   isBasementFormation,
                                                                  const int                    planeQuadratureDegree,
                                                                  const int                    depthQuadratureDegree,
                                                                  const double                 currentTime,
                                                                  const double                 timeStep,
                                                                  const bool                   Include_Advection_Term,
                                                                  const BoundaryConditions&    bcs,
                                                                  const CompoundLithology*     lithology,
                                                                  const GeoPhysics::FluidType* Fluid,
                                                                  const bool                   includeChemicalCompaction,

                                                                  const ElementGeometryMatrix& geometryMatrix,
                                                                  const ElementVector&         Element_Heat_Production,
                                                                  const ElementVector&         Current_Ph,
                                                                  const ElementVector&         Current_Po,

                                                                  const ElementVector&         Previous_Pp,
                                                                  const ElementVector&         Current_Pp,
                                                                  const ElementVector&         Previous_Lp,
                                                                  const ElementVector&         Current_Lp,
                                                                  const ElementVector&         Previous_Element_VES,
                                                                  const ElementVector&         Current_Element_VES,

                                                                  const ElementVector&         Previous_Element_Max_VES,
                                                                  const ElementVector&         Current_Element_Max_VES,

                                                                  const ElementVector&         Previous_Element_Temperature,
                                                                  const ElementVector&         Current_Element_Temperature,
                                                                  const ElementVector&         Previous_Element_Chemical_Compaction,
                                                                  const ElementVector&         Current_Element_Chemical_Compaction,

                                                                  ElementMatrix&               Element_Stiffness_Matrix,
                                                                  ElementVector&               Element_Load_Vector ) {


  double timeStepInv;


  if ( timeStep == 0.0 ) {
    timeStepInv  = 0.0;
  } else {
    timeStepInv  = 1.0 / ( timeStep * Secs_IN_MA );
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
  ThreeVector        Grad_Overpressure;
  ThreeVector        Reference_Grad_Overpressure;

  ThreeVector        gradHydrostaticPressure;
  ThreeVector        referenceGradHydrostaticPressure;

  Matrix3x3          Fluid_Mobility;
  ThreeVector        fluidFlux;
  ThreeVector        jacobianInverseFluidVelocity;


  double Fluid_Density_Heat_Capacity;
  double Fluid_Viscosity;
  double Fluid_RelativePermeability;
  
  ElementVector      Basis;
  ElementVector      Work_Space;
  GradElementVector Grad_Basis;
  GradElementVector Scaled_Grad_Basis;

  Matrix3x3          Jacobian;
  Matrix3x3          Jacobian_Inverse;
  Matrix3x3          Middle_Term;
  Matrix3x3          Middle_Term2;

  ThreeVector        gradSurfaceDepth;
  ThreeVector        referenceGradSurfaceDepth;

  ThreeVector        Heat_Flow;
  Matrix3x3          Conductivity_Tensor;
  FiniteElementMethod::BasisFunction basisFunction;

  double Previous_VES;
  double Current_VES;
  double Previous_Max_VES;
  double Current_Max_VES;
  double Previous_Pore_Pressure;
  double Current_Pore_Pressure;
  double Previous_Temperature;
  double Current_Temperature;
  double Previous_Porosity;
  double Current_Porosity;
  double Previous_Chemical_Compaction_Term;
  double Current_Chemical_Compaction_Term;
  double Previous_Bulk_Density_X_Capacity;
  double Current_Bulk_Density_X_Capacity;
  double Heat_Source_Term;
  double Load_Terms;
  double Conductivity_Normal;
  double Conductivity_Tangential;
  double PreviousLithoPressure;
  double CurrentLithoPressure;
  CompoundProperty Current_Compound_Porosity;
  CompoundProperty Previous_Compound_Porosity;

  Element_Load_Vector.zero ();
  Element_Stiffness_Matrix.zero ();

  // Remove when new DOF counting complete
  if ( Degenerate_Element ( geometryMatrix )) {
    //
    // This is only temporary until I sort out the whole degenerate segment thing!
    //
     applyDirichletBoundaryConditionsLinear ( bcs, Dirichlet_Scaling_Value, Element_Stiffness_Matrix, Element_Load_Vector );
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

        integrationWeight = X_Quadrature_Weights [ I ] *
                             Y_Quadrature_Weights [ J ] * 
                             Z_Quadrature_Weights [ K ] * 
                             determinant ( Jacobian );


        //----------------------------//

        Current_VES                      = innerProduct ( Basis, Current_Element_VES );
        Current_Max_VES                  = innerProduct ( Basis, Current_Element_Max_VES );
        Current_Pore_Pressure            = innerProduct ( Basis, Current_Pp );
        Current_Temperature              = innerProduct ( Basis, Current_Element_Temperature );
        Current_Chemical_Compaction_Term = innerProduct ( Basis, Current_Element_Chemical_Compaction );
        CurrentLithoPressure             = innerProduct ( Basis, Current_Lp );

        lithology->getPorosity ( Current_VES, Current_Max_VES, includeChemicalCompaction, Current_Chemical_Compaction_Term, Current_Compound_Porosity );
        Current_Porosity = Current_Compound_Porosity.mixedProperty ();


        Previous_VES                      = innerProduct ( Basis, Previous_Element_VES );
        Previous_Max_VES                  = innerProduct ( Basis, Previous_Element_Max_VES );
        Previous_Pore_Pressure            = innerProduct ( Basis, Previous_Pp );
        PreviousLithoPressure             = innerProduct ( Basis, Previous_Lp );
        Previous_Chemical_Compaction_Term = innerProduct ( Basis, Previous_Element_Chemical_Compaction );
        Previous_Temperature              = innerProduct ( Basis, Previous_Element_Temperature );

        lithology->getPorosity ( Previous_VES, Previous_Max_VES, includeChemicalCompaction, Previous_Chemical_Compaction_Term, Previous_Compound_Porosity );
        Previous_Porosity = Previous_Compound_Porosity.mixedProperty ();


        //----------------------------//

        //
        // Term 1
        //

        lithology -> calcBulkDensXHeatCapacity ( Fluid,
                                                 Current_Porosity,
                                                 Current_Pore_Pressure,
                                                 Current_Temperature,
                                                 CurrentLithoPressure,
                                                 Current_Bulk_Density_X_Capacity );

        lithology -> calcBulkDensXHeatCapacity ( Fluid,
                                                 Previous_Porosity,
                                                 Previous_Pore_Pressure,
                                                 Previous_Temperature,
                                                 PreviousLithoPressure,
                                                 Previous_Bulk_Density_X_Capacity );

        double t1 = Current_Bulk_Density_X_Capacity / ( 1.0 - Current_Porosity ) * ( Current_Porosity - Previous_Porosity );
        double t2 = ( 2.0 * Current_Bulk_Density_X_Capacity - Previous_Bulk_Density_X_Capacity );
        double t3 = Previous_Bulk_Density_X_Capacity * Previous_Temperature;
           
        //
        // Term 4
        //
        Heat_Source_Term = innerProduct ( Element_Heat_Production, Basis );
        Load_Terms = Heat_Source_Term + t3 * timeStepInv;

        //
        // Update the load vector with Source terms and previous time step term (Called Load_Terms).
        //
        Increment ( integrationWeight * Load_Terms, Basis, Element_Load_Vector );

        //----------------------------//
        //
        // Stiffness Matrix
        //

        //
        // Term 1
        //
        Work_Space = Basis;
        scale ( Work_Space, integrationWeight * ( t1 + t2 ) * timeStepInv );
        // scale ( Work_Space, integrationWeight * Current_Bulk_Density_X_Capacity * timeStepInv );
        addOuterProduct ( Work_Space, Basis, Element_Stiffness_Matrix );

        //
        // Term 3
        //
        if(isBasementFormation) {
           lithology -> calcBulkThermCondNPBasement ( Fluid, Current_Porosity, Current_Temperature,  CurrentLithoPressure,
                                                      Conductivity_Normal, Conductivity_Tangential );
        } else {
 
           lithology -> calcBulkThermCondNP ( Fluid, Current_Porosity, Current_Temperature, Current_Pore_Pressure, Conductivity_Normal, Conductivity_Tangential );
        }

        setTensor ( Conductivity_Normal, Conductivity_Tangential, Jacobian, Conductivity_Tensor );

        matrixMatrixProduct ( Jacobian_Inverse, Conductivity_Tensor, Middle_Term2 );
        matrixMatrixTransposeProduct ( Middle_Term2, Jacobian_Inverse, Middle_Term );
        scale ( Middle_Term, integrationWeight );

        matrixMatrixProduct ( Grad_Basis, Middle_Term, Scaled_Grad_Basis );

        addOuterProduct ( Scaled_Grad_Basis, Grad_Basis, Element_Stiffness_Matrix );

        // Add in the Advection term if required.
        if ( Include_Advection_Term && Fluid != 0 ) {

          Fluid_Density_Heat_Capacity = Fluid->densXheatCapacity ( Current_Temperature, Current_Pore_Pressure );
          Fluid_Viscosity = Fluid->viscosity ( Current_Temperature, Current_Pore_Pressure );
          Fluid_RelativePermeability = Fluid->relativePermeability ( Current_Temperature, Current_Pore_Pressure );

          matrixTransposeVectorProduct ( Grad_Basis, Current_Po, Reference_Grad_Overpressure );
          matrixTransposeVectorProduct ( Jacobian_Inverse, Reference_Grad_Overpressure, Grad_Overpressure );

          matrixTransposeVectorProduct ( Grad_Basis, Current_Ph, referenceGradHydrostaticPressure );
          matrixTransposeVectorProduct ( Jacobian_Inverse, referenceGradHydrostaticPressure, gradHydrostaticPressure );

          Grad_Overpressure ( 1 ) += gradHydrostaticPressure ( 1 );
          Grad_Overpressure ( 2 ) += gradHydrostaticPressure ( 2 );

          //
          // DONT Know what to do about this!
          // Since the fluid is removed from the system if fracturing occurs?
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
                             // 1.0,
                             Fluid_RelativePermeability,
                             lithology,
                             fluidFlux );

          fluidFlux ( 1 ) = Fluid_Density_Heat_Capacity * integrationWeight * fluidFlux ( 1 );
          fluidFlux ( 2 ) = Fluid_Density_Heat_Capacity * integrationWeight * fluidFlux ( 2 );
          fluidFlux ( 3 ) = Fluid_Density_Heat_Capacity * integrationWeight * fluidFlux ( 3 );

          matrixVectorProduct ( Jacobian_Inverse, fluidFlux, jacobianInverseFluidVelocity );
          matrixVectorProduct ( Grad_Basis, jacobianInverseFluidVelocity, Work_Space );

          addOuterProduct ( Basis, Work_Space, Element_Stiffness_Matrix );
        }

      }

    }

  }

  // Need the Neumann BCs for the heat flow.
  if ( bcs.getBoundaryCondition ( 4 ) == Bottom_Boundary_Flux ) {

    // If one bottom node is Neumann then all nodes will be.
    applyHeatFlowBoundaryConditions ( geometryMatrix, bcs, Element_Load_Vector );
  }

  applyDirichletBoundaryConditionsLinear ( bcs, Dirichlet_Scaling_Value, Element_Stiffness_Matrix, Element_Load_Vector );

}

//------------------------------------------------------------//

void Basin_Modelling::assembleElementPressureSystem ( const BasisFunctionCache&              basisFunctions,
                                                      const double                           currentTime,
                                                      const double                           timeStep,
                                                      const BoundaryConditions&              bcs,
                                                      const bool                             isIceSheetLayer,
                                                      const CompoundLithology*               lithology,
                                                      const GeoPhysics::FluidType*           Fluid,
                                                      const bool                             includeChemicalCompaction,
                                                      const Interface::FracturePressureModel fractureModel,
                                                      const ElementGeometryMatrix&           geometryMatrix,
                                                      const ElementVector&                   Previous_Ph,
                                                      const ElementVector&                   Current_Ph,
                                                      const ElementVector&                   Previous_Po,
                                                      const ElementVector&                   Current_Po,
                                                      const ElementVector&                   Current_Pl,
                                                      const ElementVector&                   Previous_Element_VES,
                                                      const ElementVector&                   Current_Element_VES,
                                                      const ElementVector&                   Previous_Element_Max_VES,
                                                      const ElementVector&                   Current_Element_Max_VES,
                                                      const ElementVector&                   Previous_Element_Temperature,
                                                      const ElementVector&                   Current_Element_Temperature,
                                                      const ElementVector&                   Previous_Element_Chemical_Compaction,
                                                      const ElementVector&                   Current_Element_Chemical_Compaction,
                                                      const ElementVector&                   Fracture_Pressure_Exceeded,
                                                      const bool                             includeWaterSaturation,
                                                      const Saturation&                      currentSaturation,
                                                      const Saturation&                      previousSaturation,
                                                      ElementMatrix&                         elementJacobian,
                                                      ElementVector&                         elementResidual ) {

   const double Thickness_Tolerance = 1.0e-6;

   // Would it be better to multiply the equation by the delta t rather than divide by it?
   const double timeStepInv = 1.0 / ( timeStep * Secs_IN_MA );

   NumericFunctions::Quadrature::QuadratureArray X_Quadrature_Points;
   NumericFunctions::Quadrature::QuadratureArray X_Quadrature_Weights;

   NumericFunctions::Quadrature::QuadratureArray Y_Quadrature_Points;
   NumericFunctions::Quadrature::QuadratureArray Y_Quadrature_Weights;

   NumericFunctions::Quadrature::QuadratureArray Z_Quadrature_Points;
   NumericFunctions::Quadrature::QuadratureArray Z_Quadrature_Weights;

   const int Number_Of_X_Points = basisFunctions.getNumberOfPointsX ();
   const int Number_Of_Y_Points = basisFunctions.getNumberOfPointsY ();
   const int Number_Of_Z_Points = basisFunctions.getNumberOfPointsZ ();

   NumericFunctions::Quadrature::getInstance ().getGaussLegendreQuadrature ( Number_Of_X_Points, X_Quadrature_Points, X_Quadrature_Weights );
   NumericFunctions::Quadrature::getInstance ().getGaussLegendreQuadrature ( Number_Of_Y_Points, Y_Quadrature_Points, Y_Quadrature_Weights );
   NumericFunctions::Quadrature::getInstance ().getGaussLegendreQuadrature ( Number_Of_Z_Points, Z_Quadrature_Points, Z_Quadrature_Weights );

   int I, J, K;

   ThreeVector        gradHydrostaticPressure;
   ThreeVector        referenceGradHydrostaticPressure;
   BasisFunction      basisFunction;

   ElementVector      Basis;
   ElementVector      Term_3;
   GradElementVector  gradBasis;
   GradElementVector  scaledGradBasis;
   GradElementVector  scaledGradBasis2;

   Matrix3x3          Jacobian;
   Matrix3x3          jacobianInverse;

   Matrix3x3          fluidMobility;
   ThreeVector        gradOverpressure;
   ThreeVector        referenceGradOverpressure;
   ThreeVector        fluidVelocity;

   CompoundProperty currentCompoundPorosity;

   elementJacobian.zero ();
   elementResidual.zero ();

   double fractureScaling = maxValue ( Fracture_Pressure_Exceeded );

   // If non-conservative fracturing has been switched-on then Has-Fractured constant will
   // be assigned false because the permeabilities are not to be scaled in this fracturing model.
   const bool   Has_Fractured = ( fractureModel == Interface::NON_CONSERVATIVE_TOTAL ? false : ( fractureScaling > 0.0 ) );

   // For the ice sheet with Permafrost taking in account, we do not want to "compute" the overpressure in the ice lithology - we want to impose it.
#if 0
   // Remove #if 0 after release
   if ( isIceSheetLayer ) { 
      applyDirichletBoundaryConditionsNewton ( bcs,
                                               Dirichlet_Scaling_Value,
                                               MPa_To_Pa,
                                               Current_Po,
                                               elementJacobian,
                                               elementResidual );

      return;
   }
#endif
   
   double usedWaterSaturation         = includeWaterSaturation ? currentSaturation ( Saturation::WATER )  : 1.0;
   double usedPreviousWaterSaturation = includeWaterSaturation ? previousSaturation ( Saturation::WATER ) : 1.0;

   // How to remove the cast here? Should be dynamic_cast anyway.
   double relativePermeability = ( includeWaterSaturation ? dynamic_cast<const Lithology*>( lithology )->relativePermeability ( Saturation::WATER, currentSaturation ) : 1.0 );

   // dVes / dP = d(pL - P) / dP = -1
   const double dVesDp = -1.0;

   for ( I = 0; I < Number_Of_X_Points; I++ ) {

      for ( J = 0; J < Number_Of_Y_Points; J++ ) {

         for ( K = 0; K < Number_Of_Z_Points; K++ ) {

            basisFunction ( X_Quadrature_Points [ I ],
                            Y_Quadrature_Points [ J ],
                            Z_Quadrature_Points [ K ],
                            Basis, 
                            gradBasis );

            matrixMatrixProduct ( geometryMatrix, gradBasis, Jacobian );

            invert ( Jacobian, jacobianInverse );

            matrixMatrixProduct ( gradBasis, jacobianInverse, scaledGradBasis );

            double integrationWeight = X_Quadrature_Weights [ I ] *
                                       Y_Quadrature_Weights [ J ] * 
                                       Z_Quadrature_Weights [ K ] * 
                                       determinant ( Jacobian );

            //----------------------------//

            double previousVes    = FiniteElementMethod::innerProduct ( Basis, Previous_Element_VES );
            double currentVes     = FiniteElementMethod::innerProduct ( Basis, Current_Element_VES );
            double previousMaxVes = FiniteElementMethod::innerProduct ( Basis, Previous_Element_Max_VES );
            double currentMaxVes  = FiniteElementMethod::innerProduct ( Basis, Current_Element_Max_VES );

            double previousHydrostaticPressure = FiniteElementMethod::innerProduct ( Basis, Previous_Ph ) * MPa_To_Pa;
            double currentHydrostaticPressure  = FiniteElementMethod::innerProduct ( Basis, Current_Ph ) * MPa_To_Pa;

            double previousOverpressure = FiniteElementMethod::innerProduct ( Basis, Previous_Po ) * MPa_To_Pa;
            double currentOverpressure  = FiniteElementMethod::innerProduct ( Basis, Current_Po ) * MPa_To_Pa;

            double previousPorePressure = previousHydrostaticPressure + previousOverpressure;
            double currentPorePressure  = currentHydrostaticPressure  + currentOverpressure;

            double previousTemperature = FiniteElementMethod::innerProduct ( Basis, Previous_Element_Temperature );
            double currentTemperature  = FiniteElementMethod::innerProduct ( Basis, Current_Element_Temperature );

            double Previous_Chemical_Compaction_Term = FiniteElementMethod::innerProduct ( Basis, Previous_Element_Chemical_Compaction );
            double Current_Chemical_Compaction_Term  = FiniteElementMethod::innerProduct ( Basis, Current_Element_Chemical_Compaction );

            double previousPorosity = lithology->porosity ( previousVes, previousMaxVes, includeChemicalCompaction, Previous_Chemical_Compaction_Term );

            lithology->getPorosity ( currentVes, currentMaxVes, includeChemicalCompaction, Current_Chemical_Compaction_Term, currentCompoundPorosity );
            double currentPorosity = currentCompoundPorosity.mixedProperty ();

            double previousFluidDensity = Fluid->density ( previousTemperature, Pa_To_MPa * previousPorePressure );
            double currentFluidDensity  = Fluid->density ( currentTemperature,  Pa_To_MPa * currentPorePressure );

            double fluidViscosity       = Fluid->viscosity ( currentTemperature, Pa_To_MPa * currentPorePressure );

            double currentRelativePermeability = relativePermeability * Fluid->relativePermeability( currentTemperature, Pa_To_MPa * currentPorePressure );

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
                                                                                  usedWaterSaturation * currentFluidDensity * currentPorosity / ( 1.0 - currentPorosity ));
            //
            // Term 2
            //
            double previousFluidDensityTerm = integrationWeight * timeStepInv * ( usedWaterSaturation * previousFluidDensity * currentPorosity + 
                                                                                  usedWaterSaturation * currentFluidDensity * previousPorosity / ( 1.0 - currentPorosity ));
            //
            // Term 3
            //
            matrixTransposeVectorProduct ( gradBasis, Current_Po, referenceGradOverpressure );
            matrixTransposeVectorProduct ( jacobianInverse, referenceGradOverpressure, gradOverpressure );

            matrixTransposeVectorProduct ( gradBasis, Current_Ph, referenceGradHydrostaticPressure );
            matrixTransposeVectorProduct ( jacobianInverse, referenceGradHydrostaticPressure, gradHydrostaticPressure );

            gradOverpressure ( 1 ) += gradHydrostaticPressure ( 1 );
            gradOverpressure ( 2 ) += gradHydrostaticPressure ( 2 );

            computeFluidMobilityTerms ( Has_Fractured,
                                        fractureScaling,
                                        currentVes,
                                        currentMaxVes,
                                        currentCompoundPorosity, 
                                        currentFluidDensity,
                                        fluidViscosity,
                                        currentRelativePermeability,
                                        Jacobian,
                                        gradOverpressure,
                                        lithology,
                                        fluidVelocity,
                                        fluidMobility );

            matrixVectorProduct ( scaledGradBasis, fluidVelocity, Term_3 );

            double Scaling = -currentFluidDensityTerm + previousFluidDensityTerm;

            Increment ( Scaling, Basis, elementResidual );
            Increment ( -integrationWeight, Term_3, elementResidual );

            //----------------------------//

            //
            // The fluid density derivative is scaled by Pa_To_MPa because the fluid density function 
            // requires the pressure to be in MPa. The pressure that is computed here is in Pa.
            //

            const double dPhiDves( lithology->computePorosityDerivativeWRTVes( currentVes,
                                                                               currentMaxVes,
                                                                               includeChemicalCompaction,
                                                                               Current_Chemical_Compaction_Term ) );

            const double dPhiDP( dPhiDves * dVesDp );

            double dRhoDP = Pa_To_MPa * Fluid->computeDensityDerivativeWRTPressure ( currentTemperature, Pa_To_MPa * currentPorePressure );

            double bulkFluidDensityDerivative = dRhoDP * usedWaterSaturation * currentPorosity / ( 1.0 - currentPorosity ) +
                                                currentFluidDensity * usedWaterSaturation / ( 1.0 - currentPorosity ) * dPhiDP +
                                                currentFluidDensity * usedWaterSaturation * currentPorosity / pow ( 1.0 - currentPorosity, 2 ) * dPhiDP;

            addOuterProduct ( integrationWeight * bulkFluidDensityDerivative * timeStepInv, Basis, Basis, elementJacobian );

            matrixMatrixProduct ( scaledGradBasis, fluidMobility, scaledGradBasis2 );
            addOuterProduct ( integrationWeight, scaledGradBasis, scaledGradBasis2, elementJacobian );
         }

      }

   }

   // The -ve sign 
   applyDirichletBoundaryConditionsNewton ( bcs,
                                            Dirichlet_Scaling_Value,
                                            MPa_To_Pa,
                                            Current_Po,
                                            elementJacobian,
                                            elementResidual );
}

//------------------------------------------------------------//

void Basin_Modelling::assembleElementTemperatureSystem ( const bool                   isBasementFormation,
                                                         const int                    planeQuadratureDegree,
                                                         const int                    depthQuadratureDegree,
                                                         const double                 currentTime,
                                                         const double                 timeStep,
                                                         const bool                   includeAdvectionTerm,
                                                         const BoundaryConditions&    bcs,
                                                         const CompoundLithology*     lithology,
                                                         const GeoPhysics::FluidType* fluid,
                                                         const bool                   includeChemicalCompaction,
                                                         const ElementGeometryMatrix& geometryMatrix,
                                                         const ElementVector&         elementHeatProduction,
                                                         const ElementVector&         previousPh,
                                                         const ElementVector&         currentPh,
                                                         const ElementVector&         previousPo,
                                                         const ElementVector&         currentPo,
                                                         const ElementVector&         currentLp,
                                                         const ElementVector&         previousElementVes,
                                                         const ElementVector&         currentElementVes,
                                                         const ElementVector&         previousElementMaxVes,
                                                         const ElementVector&         currentElementMaxVes,
                                                         const ElementVector&         previousElementTemperature,
                                                         const ElementVector&         currentElementTemperature,
                                                         const ElementVector&         previousElementChemicalCompaction,
                                                         const ElementVector&         currentElementChemicalCompaction,
                                                         ElementMatrix&               elementJacobian,
                                                         ElementVector&               elementResidual ) {


  double timeStepInv;


  if ( timeStep == 0.0 ) {
    timeStepInv  = 0.0;
  } else {
    timeStepInv  = 1.0 / ( timeStep * Secs_IN_MA );
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

  ThreeVector        gradSurfaceDepth;
  ThreeVector        referenceGradSurfaceDepth;

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


void Basin_Modelling::applyDirichletBoundaryConditionsLinear ( const BoundaryConditions& bcs,
                                                               const double              Dirichlet_Boundary_Scaling,
                                                               ElementMatrix&            Stiffness_Matrix,
                                                               ElementVector&            Load_Vector ) {

  for ( int i = 1; i <= 8; ++i ) 
  {

     if ( bcs.getBoundaryCondition ( i - 1 ) == Surface_Boundary  or bcs.getBoundaryCondition(i - 1) == Interior_Constrained_Temperature )
	 {
      Load_Vector ( i ) = Dirichlet_Boundary_Scaling * bcs.getBoundaryConditionValue ( i - 1 );
      Stiffness_Matrix ( i, i ) = Dirichlet_Boundary_Scaling;
     } 
  }

}

//------------------------------------------------------------//
