//------------------------------------------------------------//

#include "element_contributions.h"

//------------------------------------------------------------//

#include "Quadrature3D.h"
#include "BasisFunction.h"
#include "FiniteElement.h"
// #include "preevaluated_basis_functions.h"
#include "globaldefs.h"
#include "utils.h"
#include "Lithology.h"
#include "PVTCalculator.h"

//------------------------------------------------------------//

#include "BasisFunction.h"
#include "NumericFunctions.h"
using namespace FiniteElementMethod;

//------------------------------------------------------------//


string Basin_Modelling::Element_Boundary_Image ( const Element_Boundary Boundary ) {

  switch ( Boundary ) {

    case Gamma_1 : return " Gamma_1";
    case Gamma_2 : return " Gamma_2";
    case Gamma_3 : return " Gamma_3";
    case Gamma_4 : return " Gamma_4";
    case Gamma_5 : return " Gamma_5";
    case Gamma_6 : return " Gamma_6";

    default: return "No such Boundary ";

  }

}


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


void Basin_Modelling::Set_Permeability_Tensor ( const PetscScalar    Permeability_Normal,
                                                const PetscScalar    Permeability_Plane,
                                                const Matrix3x3&    Jacobian,
                                                      Matrix3x3&    Permeability_Tensor ) {

  int I;
  int J;

  PetscScalar Permeability_Difference;

  ThreeVector Normal;
  PetscScalar  Normal_Length;

  //
  // Gamma_1 here because this is the top of the model and we want the normal 
  // that "points downwards". Because we have positive depth, downwards here
  // means upwards in real coordinates.
  //
  Compute_Normal ( Jacobian, Normal );
  Normal_Length = FiniteElementMethod::length ( Normal );
  scale ( Normal, 1.0 / Normal_Length );

  Permeability_Difference = Permeability_Normal - Permeability_Plane;

  for ( I = 1; I <= 3; I++ ) {

    for ( J = 1; J <= 3; J++ ) {
      Permeability_Tensor ( I, J ) = Permeability_Difference * Normal ( I ) * Normal ( J );
    }

  }

  for ( I = 1; I <= 3; I++ ) {
    Permeability_Tensor ( I, I ) = Permeability_Tensor ( I, I ) + Permeability_Plane;
  }

}

//------------------------------------------------------------//


double Basin_Modelling::Compute_Solid_Thickness ( const ElementVector& Thickness,
                                                  const ElementVector& Basis ) {


  int    I;
  double Solid_Thickness = 0.0;

  for ( I = 1; I <= 4; I++ ) {
    Solid_Thickness = Solid_Thickness + ( Basis ( I ) + Basis ( I + 4 )) * Thickness ( I );
  }

  return Solid_Thickness;
}

//------------------------------------------------------------//


double Basin_Modelling::Maximum_Diameter ( const ElementGeometryMatrix& geometryMatrix ) {

  int I;
  int J;

  double Maximum = 0.0;
  double Diameter;

  double XmX;
  double YmY;
  double ZmZ;

  for ( I = 1; I <= 4; I++ ) {

    for ( J = 5; J <= 8; J++ ) {
      XmX = geometryMatrix ( 1, I ) - geometryMatrix ( 1, J );
      YmY = geometryMatrix ( 2, I ) - geometryMatrix ( 2, J );
      ZmZ = geometryMatrix ( 3, I ) - geometryMatrix ( 3, J );

      Diameter = sqrt ( XmX * XmX + YmY * YmY + ZmZ * ZmZ );

      Maximum = NumericFunctions::Maximum ( Maximum, Diameter );
    }

  }

  return Maximum;
}


//------------------------------------------------------------//

void Basin_Modelling::computePermGradPTerm ( const bool                debugParameter,
                                             const bool                Has_Fractured,
                                             const double              fractureScaling,
                                             const double              VES,
                                             const double              Max_VES,
                                             const CompoundProperty&   Porosity,
                                             const double              relativePermeability,
                                             const Matrix3x3&          Jacobian,
                                             const ThreeVector&        Grad_Overpressure,
                                             const CompoundLithology*  lithology,
                                                   ThreeVector&        permGradP ) {

  double Permeability_Normal;
  double Permeability_Plane;
  double Permeability_Scaling;

  Matrix3x3 fluidMobility;

  // Why square-root ( 10 )?
  static const double Sqrt10 = sqrt ( 10.0 );

  if ( Has_Fractured ) {
    Permeability_Scaling = lithology->fracturedPermeabilityScaling () * pow ( Sqrt10, 0.25 * fractureScaling );
  } else {
    Permeability_Scaling = 1.0;
  }


  lithology->calcBulkPermeabilityNP ( VES, Max_VES, Porosity, Permeability_Normal, Permeability_Plane );

  Permeability_Normal = Permeability_Scaling * relativePermeability * Permeability_Normal;
  Permeability_Plane  =                        relativePermeability * Permeability_Plane;

  Set_Permeability_Tensor ( Permeability_Normal, Permeability_Plane, Jacobian, fluidMobility );

  matrixVectorProduct ( fluidMobility, Grad_Overpressure, permGradP );

  // Need to scale the permGradP by MPa_To_Pa here, because the overpressure value
  // has been scaled by Pa_To_MPa.
  permGradP ( 1 ) *= MPa_To_Pa;
  permGradP ( 2 ) *= MPa_To_Pa;
  permGradP ( 3 ) *= MPa_To_Pa;
}

//------------------------------------------------------------//

void Basin_Modelling::computeFluidMobilityTerms ( const bool                debugParameter,
                                                  const bool                Has_Fractured,
                                                  const double              fractureScaling,
                                                  const double              VES,
                                                  const double              Max_VES,
                                                  const CompoundProperty&   Porosity,
                                                  const double              Fluid_Density,
                                                  const double              Fluid_Viscosity,
                                                  const double              relativePermeability,
                                                  const Matrix3x3&          Jacobian,
                                                  const ThreeVector&        Grad_Overpressure,
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

  Permeability_Normal = Permeability_Scaling * relativePermeability * Permeability_Normal * Fluid_Density / Fluid_Viscosity;
  Permeability_Plane  =                        relativePermeability * Permeability_Plane  * Fluid_Density / Fluid_Viscosity;

  Set_Permeability_Tensor ( Permeability_Normal, Permeability_Plane, Jacobian, Fluid_Mobility );

  matrixVectorProduct ( Fluid_Mobility, Grad_Overpressure, Fluid_Velocity );

  // Need to scale the Fluid Velocity by MPa_To_Pa here, because the overpressure value
  // has been scaled by Pa_To_MPa for the fluid_density function (because it is a function
  // of pressure in MPa)
  Fluid_Velocity ( 1 ) = Fluid_Velocity ( 1 ) * MPa_To_Pa;
  Fluid_Velocity ( 2 ) = Fluid_Velocity ( 2 ) * MPa_To_Pa;
  Fluid_Velocity ( 3 ) = Fluid_Velocity ( 3 ) * MPa_To_Pa;

}

//------------------------------------------------------------//

void Basin_Modelling::computeFluidMobilityTerms ( const bool                debugParameter,
                                                  const bool                Has_Fractured,
                                                  const double              fractureScaling,
                                                  const double              VES,
                                                  const double              Max_VES,
                                                  const CompoundProperty&   Porosity,
                                                  const double              fluidDensity,
                                                  const double              fluidDensityDerivativeWrtPressure,
                                                  const double              fluidViscosity,
                                                  const double              relativePermeability,
                                                  const Matrix3x3&          Jacobian,
                                                  const ThreeVector&        gradOverpressure,
                                                  const CompoundLithology*  lithology,
                                                        ThreeVector&        Fluid_Velocity,
                                                        Matrix3x3&          Fluid_Mobility,
                                                        ThreeVector&        fluidVelocityDerivative ) {

  double Permeability_Normal;
  double Permeability_Plane;
  double Permeability_Scaling;

  double dKnDPhi;
  double dKhDPhi;

  Matrix3x3 fluidMobilityDerivative;
  Matrix3x3 permeabilityTensor;
  Matrix3x3 permeabilityTensorDerivative;

  // Why square-root ( 10 )?
  static const double Sqrt10 = sqrt ( 10.0 );

  if ( Has_Fractured ) {
    Permeability_Scaling = lithology->fracturedPermeabilityScaling () * pow ( Sqrt10, 0.25 * fractureScaling );
  } else {
    Permeability_Scaling = 1.0;
  }


  lithology->calcBulkPermeabilityNP ( VES, Max_VES, Porosity, Permeability_Normal, Permeability_Plane );
  lithology->calcBulkPermeabilityNPDerivative ( VES, Max_VES, Porosity, dKnDPhi, dKhDPhi );

  Permeability_Normal = Permeability_Scaling * relativePermeability * Permeability_Normal * fluidDensity / fluidViscosity;
  Permeability_Plane  =                        relativePermeability * Permeability_Plane  * fluidDensity / fluidViscosity;

  Set_Permeability_Tensor ( Permeability_Normal, Permeability_Plane, Jacobian, Fluid_Mobility );

  matrixVectorProduct ( Fluid_Mobility, gradOverpressure, Fluid_Velocity );

  // Need to scale the Fluid Velocity by MPa_To_Pa here, because the overpressure value
  // has been scaled by Pa_To_MPa for the fluid_density function (because it is a function
  // of pressure in MPa)
  Fluid_Velocity ( 1 ) *= MPa_To_Pa;
  Fluid_Velocity ( 2 ) *= MPa_To_Pa;
  Fluid_Velocity ( 3 ) *= MPa_To_Pa;

  // now compute the derivative terms

  lithology->calcBulkPermeabilityNP ( VES, Max_VES, Porosity, Permeability_Normal, Permeability_Plane );
  Permeability_Normal = Permeability_Scaling * relativePermeability * Permeability_Normal;
  Permeability_Plane  =                        relativePermeability * Permeability_Plane;

  Set_Permeability_Tensor ( Permeability_Normal, Permeability_Plane, Jacobian, permeabilityTensor );
  permeabilityTensor *= fluidDensityDerivativeWrtPressure / fluidViscosity;

  //

  lithology->calcBulkPermeabilityNPDerivative ( VES, Max_VES, Porosity, dKnDPhi, dKhDPhi );
  Permeability_Normal = Permeability_Scaling * relativePermeability * dKnDPhi;
  Permeability_Plane  =                        relativePermeability * dKhDPhi;

  Set_Permeability_Tensor ( Permeability_Normal, Permeability_Plane, Jacobian, permeabilityTensorDerivative );
  permeabilityTensorDerivative *= fluidDensity / fluidViscosity * 0.0;

  add ( permeabilityTensor, permeabilityTensorDerivative, fluidMobilityDerivative );

  matrixVectorProduct ( fluidMobilityDerivative, gradOverpressure, fluidVelocityDerivative );

  // Need to scale the Fluid Velocity by MPa_To_Pa here, because the overpressure value
  // has been scaled by Pa_To_MPa for the fluid_density function (because it is a function
  // of pressure in MPa)
  fluidVelocityDerivative ( 1 ) *= MPa_To_Pa;
  fluidVelocityDerivative ( 2 ) *= MPa_To_Pa;
  fluidVelocityDerivative ( 3 ) *= MPa_To_Pa;
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


void Basin_Modelling::Compute_Fluid_Velocity ( const bool               imposeVelocityLimit,
                                               const bool               hasFractured,
                                               const double             VES,
                                               const double             maxVES,
                                               const CompoundProperty&  porosity,
                                               const double             fluidViscosity,
                                               const Matrix3x3&         jacobian,
                                               const ThreeVector&       gradOverpressure,
                                               const CompoundLithology* lithology,
                                                     ThreeVector&       fluidVelocity ) {

  /// Limit the water velocity to be within a reasonable range.
  /// This should be a depth related function, since it seems to be only
  /// the shallower parts where the high fluid velocities cause a problem.
  const double MaximumVelocity     = 1.0e-9;

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
  if ( imposeVelocityLimit and permeabilityNormal > MaximumPermeability ) {
    permeabilityNormal = MaximumPermeability;
  }

  // Limit the permeability to some maximum value.
  if ( imposeVelocityLimit and permeabilityPlane > MaximumPermeability ) {
    permeabilityPlane = MaximumPermeability;
  }

  permeabilityNormal = permeabilityScaling * permeabilityNormal / fluidViscosity;
  permeabilityPlane  =                       permeabilityPlane  / fluidViscosity;

  Set_Permeability_Tensor ( permeabilityNormal, permeabilityPlane, jacobian, permeabilityTensor );

  matrixVectorProduct ( permeabilityTensor, gradOverpressure, fluidVelocity );

  // Since pressure properties are stored in MPa units, we must convert to Pa to use in calculation.
  fluidVelocity ( 1 ) = -fluidVelocity ( 1 ) * MPa_To_Pa;
  fluidVelocity ( 2 ) = -fluidVelocity ( 2 ) * MPa_To_Pa;
  fluidVelocity ( 3 ) = -fluidVelocity ( 3 ) * MPa_To_Pa;

  /// Limit the fluid velocity to some maximum value.
  if ( imposeVelocityLimit ) {

    for ( I = 1; I <= 3; I++ ) {

      if ( fluidVelocity ( I ) > MaximumVelocity ) {
        fluidVelocity ( I ) = MaximumVelocity;
      } else if ( fluidVelocity ( I ) < -MaximumVelocity ) {
        fluidVelocity ( I ) = -MaximumVelocity;
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
           ThreeVector&            fluidVelocity ) {

  double temperature;
  double VES;
  double maxVES;
  double porosity;
  double fluidViscosity;
  double chemicalCompactionTerm;
  CompoundProperty currentCompoundPorosity;

  ElementVector      basis;
  GradElementVector gradBasis;

  ThreeVector        gradOverpressure;
  ThreeVector        referenceGradOverpressure;

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

  lithology->getPorosity ( VES, maxVES, includeChemicalCompaction, chemicalCompactionTerm, currentCompoundPorosity );
  porosity = currentCompoundPorosity.mixedProperty ();

  fluidViscosity = fluid->viscosity ( temperature );

  matrixTransposeVectorProduct ( gradBasis, currentElementPo, referenceGradOverpressure );
  matrixTransposeVectorProduct ( jacobianInverse, referenceGradOverpressure, gradOverpressure );

  matrixTransposeVectorProduct ( gradBasis, currentElementPh, referenceGradHydrostaticPressure );
  matrixTransposeVectorProduct ( jacobianInverse, referenceGradHydrostaticPressure, gradHydrostaticPressure );

  gradOverpressure ( 1 ) += gradHydrostaticPressure ( 1 );
  gradOverpressure ( 2 ) += gradHydrostaticPressure ( 2 );

  Compute_Fluid_Velocity ( false,
                           hasFractured,
                           VES,
                           maxVES,
                           currentCompoundPorosity,
                           fluidViscosity,
                           jacobian,
                           gradOverpressure,
                           lithology,
                           fluidVelocity );

  // Convert to mm/year.
  fluidVelocity ( 1 ) =  1000.0 * SecondsPerYear * fluidVelocity ( 1 ) / porosity;
  fluidVelocity ( 2 ) =  1000.0 * SecondsPerYear * fluidVelocity ( 2 ) / porosity;
  // +ve to represent upwards, so scale by -1
  fluidVelocity ( 3 ) = -1000.0 * SecondsPerYear * fluidVelocity ( 3 ) / porosity;

}

//------------------------------------------------------------//


double Basin_Modelling::CFL_Value
   ( const CompoundLithology*            lithology,
     const GeoPhysics::FluidType*        Fluid,
     const bool                          includeChemicalCompaction,
     const ElementGeometryMatrix&      geometryMatrix,
     const ElementVector&               Current_Ph,
     const ElementVector&               Current_Po,
     const ElementVector&               Current_Element_VES,
     const ElementVector&               Current_Element_Max_VES,
     const ElementVector&               Current_Element_Temperature,
     const ElementVector&               Current_Element_Chemical_Compaction,
     const ElementVector&               Fracture_Pressure_Exceeded ) {

  ThreeVector Grad_Overpressure;
  ThreeVector Reference_Grad_Overpressure;
  ThreeVector Fluid_Velocity;

  ThreeVector        gradSurfaceDepth;
  ThreeVector        referenceGradSurfaceDepth;

  ThreeVector        gradHydrostaticPressure;
  ThreeVector        referenceGradHydrostaticPressure;

  ElementVector      Basis;
  GradElementVector Grad_Basis;

  Matrix3x3          Jacobian;
  Matrix3x3          Jacobian_Inverse;
  FiniteElementMethod::BasisFunction basisFunction;

  if ( Degenerate_Element ( geometryMatrix )) {
    //
    // It is better if degenerate element do not influence the CFL value, 
    // so set it to be very high.
    //
    return 100.0 * Secs_IN_MA;

  } else if ( lithology -> surfacePorosity () == 0.0 ) {
    //
    // As these elements are not represented in the mesh they do not have 
    // any direct influence on the result. Therefore, set the CFL value to
    // a very high value.
    //
    return 100.0 * Secs_IN_MA;

  } else {
    //
    // Evaluate the CFL value at the centre of the element. Perhaps some time in the future
    // would it be possible to evaluate it at, say, the Gauss points. Would it be better to 
    // evaluate it during computation of the Jacobian and Residual? This may save some time.
    //
    basisFunction ( 0.0, 0.0, 0.0, Basis, Grad_Basis );

    double Current_Temperature      = FiniteElementMethod::innerProduct ( Basis, Current_Element_Temperature );
    double Current_VES              = FiniteElementMethod::innerProduct ( Basis, Current_Element_VES );
    double Current_Max_VES          = FiniteElementMethod::innerProduct ( Basis, Current_Element_Max_VES );
    double Chemical_Compaction_Term = FiniteElementMethod::innerProduct ( Basis, Current_Element_Chemical_Compaction );

    CompoundProperty Current_Porosity;
    lithology->getPorosity ( Current_VES, Current_Max_VES, includeChemicalCompaction, Chemical_Compaction_Term, Current_Porosity );

    double Fluid_Viscosity = Fluid->viscosity ( Current_Temperature );

    bool Has_Fractured = maxValue ( Fracture_Pressure_Exceeded ) > 0.0;

    matrixMatrixProduct ( geometryMatrix, Grad_Basis, Jacobian );
    invert ( Jacobian, Jacobian_Inverse );

    matrixTransposeVectorProduct ( Grad_Basis, Current_Po, Reference_Grad_Overpressure );
    matrixTransposeVectorProduct ( Jacobian_Inverse, Reference_Grad_Overpressure, Grad_Overpressure );

    matrixTransposeVectorProduct ( Grad_Basis, Current_Ph, referenceGradHydrostaticPressure );
    matrixTransposeVectorProduct ( Jacobian_Inverse, referenceGradHydrostaticPressure, gradHydrostaticPressure );

    Grad_Overpressure ( 1 ) += gradHydrostaticPressure ( 1 );
    Grad_Overpressure ( 2 ) += gradHydrostaticPressure ( 2 );

    Compute_Fluid_Velocity ( true,
                             Has_Fractured,
                             Current_VES,
                             Current_Max_VES,
                             Current_Porosity,
                             Fluid_Viscosity,
                             Jacobian,
                             Grad_Overpressure,
                             lithology,
                             Fluid_Velocity );

    return Maximum_Diameter ( geometryMatrix ) / sqrt ( FiniteElementMethod::innerProduct ( Fluid_Velocity, Fluid_Velocity ));
  } // 

}

//------------------------------------------------------------//


void Basin_Modelling::Set_Heat_Conductivity_Tensor ( const double      Conductivity_Normal,
                                                     const double      Conductivity_Plane,
                                                     const Matrix3x3& Jacobian,
                                                           Matrix3x3& Conductivity_Tensor ) {

  int I;
  int J;

  double       Conductivity_Difference;
  double       Normal_Length;
  ThreeVector Normal;

  Compute_Normal ( Jacobian, Normal );
  Normal_Length = FiniteElementMethod::length ( Normal );
  scale ( Normal, 1.0 / Normal_Length );

  Conductivity_Difference = Conductivity_Normal - Conductivity_Plane;

  for ( I = 1; I <= 3; I++ ) {

    for ( J = 1; J <= 3; J++ ) {
      Conductivity_Tensor ( I, J ) = Conductivity_Difference * Normal ( I ) * Normal ( J );
    }

  }

  for ( I = 1; I <= 3; I++ ) {
    Conductivity_Tensor ( I, I ) = Conductivity_Tensor ( I, I ) + Conductivity_Plane;
  }

}


//------------------------------------------------------------//


void Basin_Modelling::Compute_Heat_Flow ( const bool                 isBasementFormation,      
                                          const CompoundLithology*   lithology,
                                          const FluidType*           Fluid,
                                          const ElementVector&       Temperature_Vector,
                                          const double               Temperature_Value,
                                          const double               Porosity,
                                          const double               LithostaticPressure,
                                          const Matrix3x3&           Jacobian,
                                          const GradElementVector&   Grad_Basis,
                                                ThreeVector&         Heat_Flow,
                                                Matrix3x3&           Conductivity_Tensor ) {

  double Conductivity_Normal;
  double Conductivity_Tangential;

  if(isBasementFormation) {
     lithology -> calcBulkThermCondNPBasement ( Fluid, Porosity, Temperature_Value, LithostaticPressure, Conductivity_Normal, Conductivity_Tangential );
  } else {
     lithology -> calcBulkThermCondNP ( Fluid, Porosity, Temperature_Value, Conductivity_Normal, Conductivity_Tangential );
  }
  GradElementVector Grad_Basis2;

  Set_Heat_Conductivity_Tensor ( Conductivity_Normal, Conductivity_Tangential, Jacobian, Conductivity_Tensor );
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

  lithology->getPorosity ( VES, maxVES, includeChemicalCompaction, chemicalCompactionTerm, currentCompoundPorosity );
  porosity = currentCompoundPorosity.mixedProperty ();
  if(isBasementFormation) {
     lithology -> calcBulkThermCondNPBasement ( fluid, porosity, temperature, lithostaticPressure, conductivityNormal, conductivityTangential );
  } else {
     lithology -> calcBulkThermCondNP ( fluid, porosity, temperature, conductivityNormal, conductivityTangential );
  }     
  Set_Heat_Conductivity_Tensor ( conductivityNormal, conductivityTangential, jacobian, conductivityTensor );
  matrixMatrixProduct ( scaledGradBasis, conductivityTensor, gradBasis2 );
  matrixTransposeVectorProduct ( gradBasis2, currentElementTemperature, heatFlow );

  ///
  /// Fouriers law states:  q = -k \grad T
  ///
  scale ( heatFlow, -1.0 );

  if ( includeAdvectionTerm && fluid != 0 ) {

    double fluidDensity;
    double fluidViscosity;
    double heatCapacity;
    double porePressure;
    double advectionScaling;

    ThreeVector referenceGradOverpressure;
    ThreeVector gradOverpressure;
    ThreeVector fluidVelocity;

    matrixTransposeVectorProduct ( gradBasis, currentElementPo, referenceGradOverpressure );
    matrixTransposeVectorProduct ( jacobianInverse, referenceGradOverpressure, gradOverpressure );
    porePressure = FiniteElementMethod::innerProduct ( currentElementPp, basis );

    fluidDensity   = fluid->density ( temperature, porePressure );
    heatCapacity   = fluid->heatCapacity ( temperature, porePressure );
    fluidViscosity = fluid->viscosity ( temperature );

    bool hasFractured = false;

    Compute_Fluid_Velocity ( true,
                             hasFractured,
                             VES,
                             maxVES,
                             currentCompoundPorosity,
                             fluidViscosity,
                             jacobian,
                             gradOverpressure,
                             lithology,
                             fluidVelocity );

    advectionScaling = fluidDensity * heatCapacity * temperature;

    heatFlow ( 1 ) = heatFlow ( 1 ) + advectionScaling * fluidVelocity ( 1 );
    heatFlow ( 2 ) = heatFlow ( 2 ) + advectionScaling * fluidVelocity ( 2 );
    heatFlow ( 3 ) = heatFlow ( 3 ) + advectionScaling * fluidVelocity ( 3 );
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


void Basin_Modelling::Apply_Heat_Flow_Boundary_Conditions 
   ( const ElementGeometryMatrix& geometryMatrix,
     const ElementVector&          Nodal_BC_Values,
           ElementVector&          ElementVector_Contributions ) {

  int I;

  double X_Quadrature_Points [ 4 ] = { -1.0,  1.0, 1.0, -1.0 };
  double Y_Quadrature_Points [ 4 ] = { -1.0, -1.0, 1.0,  1.0 };
  double Z_Quadrature_Point = 1.0;

  double              Normal_Length;
  ElementVector      Basis;
  GradElementVector Grad_Basis;
  Matrix3x3          Jacobian;
  ThreeVector        Normal;
  FiniteElementMethod::BasisFunction basisFunction;

  ///
  /// Integrate the Neumann boundary conditions using a Gauss-Lobatto quadrature.
  /// This greatly simplifies the computation here, and is safe since the elements
  /// here, in the basement, will never have degenerate segments.
  ///
  for ( I = 1; I <= 4; I++ ) {
    basisFunction ( X_Quadrature_Points [ I - 1 ],
                    Y_Quadrature_Points [ I - 1 ],
                    Z_Quadrature_Point,
                    Basis, Grad_Basis );

    matrixMatrixProduct ( geometryMatrix, Grad_Basis, Jacobian );

    Compute_Normal ( Jacobian, Normal );
    Normal_Length = FiniteElementMethod::length ( Normal );

    ///
    /// Since we are using a Gauss-Lobatto quadrature and the only non-zero entry will be 
    /// at the node then the calculation can be simplified to only adding in the contribution
    /// from the boundary condition value and the normal vector in the z-direction.
    ///
    ElementVector_Contributions ( I + 4 ) = ElementVector_Contributions ( I + 4 ) - Normal_Length * Nodal_BC_Values ( I + 4 );
  }

}

//------------------------------------------------------------//


void Basin_Modelling::applyPressureNeumannBoundaryConditions 
   ( const ElementGeometryMatrix& geometryMatrix,
     const CompoundLithology*       lithology,
     const GeoPhysics::FluidType*   Fluid,
     const bool                     includeChemicalCompaction,
     const Boundary_Conditions*     BCs,
     const ElementVector&          Current_Ph,
     const ElementVector&          Current_Po,
     const ElementVector&          Current_Element_VES,
     const ElementVector&          Current_Element_Max_VES,
     const ElementVector&          Current_Element_Temperature,
     const ElementVector&          Current_Element_Chemical_Compaction,
           ElementVector&          ElementVector_Contributions ) {

  int I;
  int J;

  NumericFunctions::Quadrature::QuadratureArray X_Quadrature_Points;
  NumericFunctions::Quadrature::QuadratureArray X_Quadrature_Weights;

  NumericFunctions::Quadrature::QuadratureArray Y_Quadrature_Points;
  NumericFunctions::Quadrature::QuadratureArray Y_Quadrature_Weights;

  const int Number_Of_Quadrature_Points = 3;

  NumericFunctions::Quadrature::getInstance ().getGaussLegendreQuadrature ( Number_Of_Quadrature_Points, X_Quadrature_Points, X_Quadrature_Weights );
  NumericFunctions::Quadrature::getInstance ().getGaussLegendreQuadrature ( Number_Of_Quadrature_Points, Y_Quadrature_Points, Y_Quadrature_Weights );

  double Z_Quadrature_Point = 1.0;

  double              Normal_Length;
  ElementVector      Basis;
  GradElementVector Grad_Basis;
  Matrix3x3          middleTerm;
  Matrix3x3          Jacobian;
  Matrix3x3          Jacobian_Inverse;

  ThreeVector        Normal;

  Matrix3x3          Fluid_Mobility;
  ThreeVector        Grad_Overpressure;
  ThreeVector        Reference_Grad_Overpressure;
  ThreeVector        Fluid_Velocity;
  FiniteElementMethod::BasisFunction basisFunction;

  double integrationWeight;
  double Current_Fluid_Density;
  double Fluid_Viscosity;
  double Current_VES;
  double Current_Max_VES;
  double Current_Hydrostatic_Pressure;
  double Current_Overpressure;
  double Current_Pore_Pressure;
  double Current_Temperature;
  CompoundProperty Current_Compound_Porosity;
  double Current_Chemical_Compaction;
  double Boundary_Flux;

  bool Has_Fractured = false;
  double fractureScaling = 1.0;


  ///
  /// Integrate the Neumann boundary conditions using a Gauss-Lobatto quadrature.
  /// This greatly simplifies the computation here, and is safe since the elements
  /// here, in the basement, will never have degenerate segments.
  ///
  for ( I = 0; I < Number_Of_Quadrature_Points; I++ ) {

     for ( J = 0; J < Number_Of_Quadrature_Points; J++ ) {


        basisFunction ( X_Quadrature_Points [ I ],
                        Y_Quadrature_Points [ J ],
                        Z_Quadrature_Point,
                        Basis, Grad_Basis );

        integrationWeight = X_Quadrature_Weights [ I ] * Y_Quadrature_Weights [ J ];

        matrixMatrixProduct ( geometryMatrix, Grad_Basis, Jacobian );
        invert ( Jacobian, Jacobian_Inverse );

        Current_Overpressure = FiniteElementMethod::innerProduct ( Basis, Current_Po );
        Current_Hydrostatic_Pressure = FiniteElementMethod::innerProduct ( Basis, Current_Ph );

        Current_VES = FiniteElementMethod::innerProduct ( Basis, Current_Element_VES );
        Current_Max_VES = FiniteElementMethod::innerProduct ( Basis, Current_Element_Max_VES );
        Current_Temperature = FiniteElementMethod::innerProduct ( Basis, Current_Element_Temperature );
        Current_Chemical_Compaction = FiniteElementMethod::innerProduct ( Basis, Current_Element_Chemical_Compaction );

        Current_Pore_Pressure  = Current_Hydrostatic_Pressure  + Current_Overpressure;

        lithology->getPorosity ( Current_VES, Current_Max_VES, includeChemicalCompaction, Current_Chemical_Compaction, Current_Compound_Porosity );

        Current_Fluid_Density  = Fluid->density ( Current_Temperature,  Pa_To_MPa * Current_Pore_Pressure );
        Fluid_Viscosity = Fluid->viscosity ( Current_Temperature );

        Compute_Normal ( Jacobian, Normal );
        Normal_Length = FiniteElementMethod::length ( Normal );

        matrixTransposeVectorProduct ( Grad_Basis, Current_Po, Reference_Grad_Overpressure );
        matrixTransposeVectorProduct ( Jacobian_Inverse, Reference_Grad_Overpressure, Grad_Overpressure );

        computeFluidMobilityTerms ( false,
                                    Has_Fractured,
                                    fractureScaling,
                                    Current_VES,
                                    Current_Max_VES,
                                    Current_Compound_Porosity, 
                                    Current_Fluid_Density,
                                    Fluid_Viscosity, 1.0, 
                                    Jacobian,
                                    Grad_Overpressure,
                                    lithology,
                                    Fluid_Velocity,
                                    Fluid_Mobility );

        Boundary_Flux = FiniteElementMethod::innerProduct ( Fluid_Velocity, Normal ) / Normal_Length;
        Increment ( integrationWeight * Boundary_Flux, Basis, ElementVector_Contributions );
     }

  }

}

//------------------------------------------------------------//


void Basin_Modelling::Apply_Dirichlet_Boundary_Conditions_Newton
   ( const Boundary_Conditions*          BCs,
     const ElementVector&               Dirichlet_Boundary_Values,
     const double                        Dirichlet_Boundary_Scaling,
     const ElementVector&               Current_Property_Values,
           ElementMatrix&               Jacobian,
           ElementVector&               Residual ) {

  int I;

  for ( I = 1; I <= 8; I++ ) {

    if ( BCs [ I - 1 ] == Surface_Boundary || BCs [ I - 1 ] == Interior_Constrained_Overpressure ) {
      Residual ( I ) = Dirichlet_Boundary_Scaling * ( Dirichlet_Boundary_Values ( I ) - Current_Property_Values ( I )) * MPa_To_Pa;
      Jacobian ( I, I ) = Dirichlet_Boundary_Scaling;
    } else if ( BCs [ I - 1 ] == Interior_Constrained_Temperature ) {
      Residual ( I ) = Dirichlet_Boundary_Scaling * ( Dirichlet_Boundary_Values ( I ) - Current_Property_Values ( I ));
      Jacobian ( I, I ) = Dirichlet_Boundary_Scaling;
    }

  }

  scale ( Residual, Element_Scaling );
  scale ( Jacobian, Element_Scaling );

}

//------------------------------------------------------------//


void Basin_Modelling::Apply_Dirichlet_Boundary_Conditions_Newton
   ( const Boundary_Conditions*          BCs,
     const ElementVector&               Dirichlet_Boundary_Values,
     const double                        Dirichlet_Boundary_Scaling,
     const ElementVector&               Current_Property_Values,
           ElementVector&               Residual ) {

  int I;

  for ( I = 1; I <= 8; I++ ) {

    if ( BCs [ I - 1 ] == Surface_Boundary || BCs [ I - 1 ] == Interior_Constrained_Overpressure ) {
      Residual ( I ) = Dirichlet_Boundary_Scaling * ( Dirichlet_Boundary_Values ( I ) - Current_Property_Values ( I )) * MPa_To_Pa;
    } else if (BCs [ I - 1 ] == Interior_Constrained_Temperature ) {
      Residual ( I ) = Dirichlet_Boundary_Scaling * ( Dirichlet_Boundary_Values ( I ) - Current_Property_Values ( I ));
    }

  }

  scale ( Residual, Element_Scaling );
}

//------------------------------------------------------------//


void Basin_Modelling::Apply_Dirichlet_Boundary_Conditions_Linear
   ( const Boundary_Conditions*          BCs,
     const ElementVector&               Dirichlet_Boundary_Values,
     const double                        Dirichlet_Boundary_Scaling,
           ElementMatrix&               Stiffness_Matrix,
           ElementVector&               Load_Vector ) {

  int I;

  for ( I = 1; I <= 8; I++ ) {

    if ( BCs [ I - 1 ] == Surface_Boundary ) {
      Load_Vector ( I ) = Dirichlet_Boundary_Scaling * Dirichlet_Boundary_Values ( I );
      Stiffness_Matrix ( I, I ) = Dirichlet_Boundary_Scaling;
    } else if (BCs [ I - 1 ] == Interior_Constrained_Temperature ) {
      Load_Vector ( I ) = Dirichlet_Boundary_Scaling * Dirichlet_Boundary_Values ( I );
      Stiffness_Matrix ( I, I ) = Dirichlet_Boundary_Scaling;
    }

  }

}

//------------------------------------------------------------//


void Basin_Modelling::Assemble_Element_Pressure_Salt_System
   ( const double                   timeStep,
     const CompoundLithology*       lithology,
     const GeoPhysics::FluidType*   Fluid,
     const ElementGeometryMatrix& geometryMatrix,
           ElementMatrix&          Element_Jacobian,
           ElementVector&          Element_Residual ) {


  const int Number_Of_Quadrature_Points = 2;

  NumericFunctions::Quadrature::QuadratureArray Quadrature_Points;
  NumericFunctions::Quadrature::QuadratureArray Quadrature_Weights;

  NumericFunctions::Quadrature::getInstance ().getGaussLegendreQuadrature ( Number_Of_Quadrature_Points, Quadrature_Points, Quadrature_Weights );

  int I, J, K, III;

  double integrationWeight;

  ElementVector      Basis;
  GradElementVector Grad_Basis;
  GradElementVector Scaled_Grad_Basis;

  Matrix3x3          Jacobian;
  Matrix3x3          Jacobian_Inverse;

  Matrix3x3          Fluid_Mobility;
  ThreeVector        Grad_Overpressure;

  double Fluid_Viscosity;
  double Current_VES;
  double Current_Max_VES;
  CompoundProperty Current_Compound_Porosity;
  double Permeability_Normal;
  double Permeability_Plane;
  double Current_Fluid_Density;
  double Scaling;
  FiniteElementMethod::BasisFunction basisFunction;

  Element_Jacobian.zero ();
  Element_Residual.zero ();

  ///
  /// Porosity = 0, Salt lithologies have zero porosity by definition
  ///
  Current_Compound_Porosity ( 0 ) = 0.0;
  Current_Compound_Porosity ( 1 ) = 0.0;
  Current_Compound_Porosity ( 2 ) = 0.0;

  ///
  /// Since the permeability is constant, then we could put any value for the VES and Max_VES, it would ignored.
  ///
  Current_VES = 0.0;
  Current_Max_VES = 0.0;

  ///
  /// There is no fluid (porosity=0), but to satisify the assumptions made on the calculation
  /// a fluid density is required. So the density of standard saline water is chosen.
  ///
  Current_Fluid_Density  = 1030.0;

  ///
  /// The fluid viscosity is for a fluiid with 0.13 Salinity and temperature=200
  ///
  Fluid_Viscosity = 2.236e-04; // Assume Salinity = 0.13, T = 200;

  ///
  /// The permeability is a constant, so can be removed frmo the loop
  ///
  lithology->calcBulkPermeabilityNP ( Current_VES, Current_Max_VES, Current_Compound_Porosity, Permeability_Normal, Permeability_Plane );

  for ( I = 0; I < Number_Of_Quadrature_Points; I++ ) {

    for ( J = 0; J < Number_Of_Quadrature_Points; J++ ) {

      for ( K = 0; K < Number_Of_Quadrature_Points; K++ ) {

        basisFunction ( Quadrature_Points [ I ],
                        Quadrature_Points [ J ],
                        Quadrature_Points [ K ],
                        Basis, 
                        Grad_Basis );

        matrixMatrixProduct ( geometryMatrix, Grad_Basis, Jacobian );

        invert ( Jacobian, Jacobian_Inverse );

        matrixMatrixProduct ( Grad_Basis, Jacobian_Inverse, Scaled_Grad_Basis );

        integrationWeight = Quadrature_Weights [ I ] *
                             Quadrature_Weights [ J ] * 
                             Quadrature_Weights [ K ] * 
                             determinant ( Jacobian );


        //----------------------------//

        Scaling = Element_Scaling * integrationWeight * Permeability_Normal * Current_Fluid_Density / Fluid_Viscosity;

        ///
        /// diag ( \nabla N \frac{\kappa \rho_f}{\mu}) \nabla N )
        ///
        for ( III = 1; III <= 8; III++ ) {
          Element_Jacobian ( III, III ) = Element_Jacobian ( III, III ) 
                                           + Scaling * (  Scaled_Grad_Basis ( III, 1 ) * Scaled_Grad_Basis ( III, 1 ) 
                                                        + Scaled_Grad_Basis ( III, 2 ) * Scaled_Grad_Basis ( III, 2 ) 
                                                        + Scaled_Grad_Basis ( III, 3 ) * Scaled_Grad_Basis ( III, 3 ));
        }

      }

    }

  }

}

//------------------------------------------------------------//

void Basin_Modelling::computeGradSurfaceDepth ( const PetscScalar          xi,
                                                const PetscScalar          eta,
                                                const ElementVector&      surfaceDepth,
                                                      ThreeVector&        referenceGradSurfaceDepth ) {


  GradElementVector grad2DBasis;
  int i;

  grad2DBasis.zero ();

  grad2DBasis ( 1, 1 ) = -0.25 * ( 1.0 - eta );
  grad2DBasis ( 1, 2 ) = -0.25 * ( 1.0 - xi  );

  grad2DBasis ( 2, 1 ) =  0.25 * ( 1.0 - eta );
  grad2DBasis ( 2, 2 ) = -0.25 * ( 1.0 + xi  );

  grad2DBasis ( 3, 1 ) =  0.25 * ( 1.0 + eta );
  grad2DBasis ( 3, 2 ) =  0.25 * ( 1.0 + xi  );

  grad2DBasis ( 4, 1 ) = -0.25 * ( 1.0 + eta );
  grad2DBasis ( 4, 2 ) =  0.25 * ( 1.0 - xi  );

  referenceGradSurfaceDepth ( 1 ) = 0.0;
  referenceGradSurfaceDepth ( 2 ) = 0.0;
  referenceGradSurfaceDepth ( 3 ) = 0.0;

  for ( i = 1; i <= 4; ++i ) {
    referenceGradSurfaceDepth ( 1 ) = referenceGradSurfaceDepth ( 1 ) + grad2DBasis ( i, 1 ) * surfaceDepth ( i );
    referenceGradSurfaceDepth ( 2 ) = referenceGradSurfaceDepth ( 2 ) + grad2DBasis ( i, 2 ) * surfaceDepth ( i );
  }

}


//------------------------------------------------------------//

void Basin_Modelling::Assemble_Element_Pressure_System
   ( const int                      planeQuadratureDegree,
     const int                      depthQuadratureDegree,
     const double                   currentTime,
     const double                   timeStep,
     const Boundary_Conditions*     Element_BCs,
     const Boundary_Conditions*     BCs,
     const ElementVector&          Dirichlet_Boundary_Values,
     const CompoundLithology*       lithology,
     const GeoPhysics::FluidType*   Fluid,
     const bool                     includeChemicalCompaction,
     const Interface::FracturePressureModel    fractureModel,
     const ElementGeometryMatrix& previousGeometryMatrix,
     const ElementGeometryMatrix& geometryMatrix,

     const ElementVector&          Previous_Element_Solid_Thickness,
     const ElementVector&          Current_Element_Solid_Thickness,
     const ElementVector&          Previous_Ph,
     const ElementVector&          Current_Ph,
     const ElementVector&          Previous_Po,
     const ElementVector&          Current_Po,
     const ElementVector&          Current_Pl,
     const ElementVector&          Previous_Element_VES,
     const ElementVector&          Current_Element_VES,
     const ElementVector&          Previous_Element_Max_VES,
     const ElementVector&          Current_Element_Max_VES,
     const ElementVector&          Previous_Element_Temperature,
     const ElementVector&          Current_Element_Temperature,

     const ElementVector&          Previous_Element_Chemical_Compaction,
     const ElementVector&          Current_Element_Chemical_Compaction,

     const ElementVector&          Fracture_Pressure_Exceeded,
     const ElementVector&          preFractureScaling,
     const BooleanVector&          Included_Nodes,

     const bool                    includeWaterSaturation,
     const Saturation&             currentSaturation,
     const Saturation&             previousSaturation,

           ElementMatrix&          Element_Jacobian,
           ElementVector&          Element_Residual ) {


   const Lithology* litho = static_cast<const Lithology*>( lithology );

  const double Thickness_Tolerance = 1.0e-6;

  // Would it be better to multiply the equation by the delta t rather than divide by it?
  const double timeStepInv = 1.0 / ( timeStep * Secs_IN_MA );

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

  double previousIntegrationWeight;
  double integrationWeight;

  ElementVector      currentDepth;
  ElementVector      previousDepth;
  ThreeVector        gradSurfaceDepth;
  ThreeVector        referenceGradSurfaceDepth;

  ThreeVector        gradHydrostaticPressure;
  ThreeVector        referenceGradHydrostaticPressure;
  BasisFunction basisFunction;

  ElementVector      Basis;
  ElementVector      Term_3;
  GradElementVector gradBasis;
  GradElementVector scaledGradBasis;
  GradElementVector scaledGradBasis2;

  Matrix3x3          Middle_Term;
  Matrix3x3          Middle_Term2;
  Matrix3x3          Jacobian;
  Matrix3x3          jacobianInverse;
  Matrix3x3          Previous_Jacobian;

  Matrix3x3          fluidMobility;
  ThreeVector        gradOverpressure;
  ThreeVector        referenceGradOverpressure;
  ThreeVector        fluidVelocity;
  ThreeVector        fluidVelocityDerivative;
  ThreeVector        jacobianInverseFluidVelocity;
  double              sourceTerm;

  double bulkFluidDensityDerivative;
  double previousFluidDensityTerm;
  double currentFluidDensityTerm;
  double currentFluidDensityDerivative;

  double fluidViscosity;

  double previousVes;
  double currentVes;

  double previousMaxVes;
  double currentMaxVes;

  double previousHydrostaticPressure;
  double currentHydrostaticPressure;

  double previousOverpressure;
  double currentOverpressure;

  double previousPorePressure;
  double currentPorePressure;

  double previousTemperature;
  double currentTemperature;

  double previousPorosity;
  double currentPorosity;
  CompoundProperty currentCompoundPorosity;

  double Previous_Chemical_Compaction_Term;
  double Current_Chemical_Compaction_Term;

  double previousFluidDensity;
  double currentFluidDensity;

  double previousSolidThickness;
  double currentSolidThickness;

  double relativePermeability = litho->relativePermeability ( Saturation::WATER, currentSaturation );
  double usedWaterSaturation;
  double usedPreviousWaterSaturation;

  double dPhiDP;
  double dRhoDP;
  double t1;
  double t2;
  double t3;
  double t4;

  ElementMatrix Grad_Term_3;

  Element_Jacobian.zero ();
  Element_Residual.zero ();

  double fractureScaling = maxValue ( Fracture_Pressure_Exceeded );

  // If non-conservative fracturing has been switched-on then Has-Fractured constant will
  // be assigned false because the permeabilities are not to be scaled in this fracturing model.
  const bool   Has_Fractured = ( fractureModel == Interface::NON_CONSERVATIVE_TOTAL ? false : ( fractureScaling > 0.0 ));

  bool Is_At_Top     = false;
  ThreeVector        Outward_Normal;

  fractureScaling = fractureScaling + maxValue ( preFractureScaling );

  if ( Degenerate_Element ( geometryMatrix )) {

    ///
    /// This is only temporary until I sort out the whole degenerate segment thing!
    ///
    Apply_Dirichlet_Boundary_Conditions_Newton ( BCs, Dirichlet_Boundary_Values, Dirichlet_Scaling_Value,
                                                 Current_Po, Element_Jacobian, Element_Residual );


    return;
  }

  if ( lithology -> surfacePorosity () == 0.0 ) {

    for ( I = 1; I <= 8; I++ ) {

      ///
      /// Only set the diagonal to a Dirichlet node if the node is not included AND the segment is not degenerate
      ///
      if ( ! Included_Nodes ( I ) && (  fabs ( geometryMatrix ( 3, ( I - 1 ) % 4 + 1 ) - geometryMatrix ( 3, ( I - 1 ) % 4 + 5 )) > 0.001 )) {
        Element_Jacobian ( I, I ) = Dirichlet_Scaling_Value;
      }

    }

    Apply_Dirichlet_Boundary_Conditions_Newton ( BCs, Dirichlet_Boundary_Values, Dirichlet_Scaling_Value,
                                                 Current_Po, Element_Jacobian, Element_Residual );


    return;
  }
  

  for ( I = 1; I <= 8; I++ ) {

    if ( BCs [ I - 1 ] == Surface_Boundary ) {
      Is_At_Top = true;
      break;
    }

  }


  for ( I = 1; I <= 8; ++I ) {
    currentDepth  ( I ) = geometryMatrix ( 3, I );
    previousDepth ( I ) = previousGeometryMatrix ( 3, I );
  }

  basisFunction ( 0.0, 0.0, 0.0, Basis, gradBasis );

  previousSolidThickness = Compute_Solid_Thickness ( Previous_Element_Solid_Thickness, Basis );
  currentSolidThickness  = Compute_Solid_Thickness ( Current_Element_Solid_Thickness,  Basis );

  if ( includeWaterSaturation ) {
     usedWaterSaturation = currentSaturation ( Saturation::WATER );
     usedPreviousWaterSaturation = previousSaturation ( Saturation::WATER );
  } else {
     usedWaterSaturation = 1.0;
     usedPreviousWaterSaturation = 1.0;
  }

  if ( includeWaterSaturation ) {
     relativePermeability = litho->relativePermeability ( Saturation::WATER, currentSaturation );
  } else {
     relativePermeability = 1.0;
  }

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

        integrationWeight = X_Quadrature_Weights [ I ] *
                             Y_Quadrature_Weights [ J ] * 
                             Z_Quadrature_Weights [ K ] * 
                             determinant ( Jacobian );

        //----------------------------//

        previousVes = FiniteElementMethod::innerProduct ( Basis, Previous_Element_VES );
        currentVes  = FiniteElementMethod::innerProduct ( Basis, Current_Element_VES );
        previousMaxVes = FiniteElementMethod::innerProduct ( Basis, Previous_Element_Max_VES );
        currentMaxVes  = FiniteElementMethod::innerProduct ( Basis, Current_Element_Max_VES );

        previousHydrostaticPressure = FiniteElementMethod::innerProduct ( Basis, Previous_Ph ) * MPa_To_Pa;
        currentHydrostaticPressure  = FiniteElementMethod::innerProduct ( Basis, Current_Ph ) * MPa_To_Pa;

        previousOverpressure = FiniteElementMethod::innerProduct ( Basis, Previous_Po ) * MPa_To_Pa;
        currentOverpressure  = FiniteElementMethod::innerProduct ( Basis, Current_Po ) * MPa_To_Pa;

        previousPorePressure = previousHydrostaticPressure + previousOverpressure;
        currentPorePressure  = currentHydrostaticPressure  + currentOverpressure;

        previousTemperature = FiniteElementMethod::innerProduct ( Basis, Previous_Element_Temperature );
        currentTemperature  = FiniteElementMethod::innerProduct ( Basis, Current_Element_Temperature );

        Previous_Chemical_Compaction_Term = FiniteElementMethod::innerProduct ( Basis, Previous_Element_Chemical_Compaction );
        Current_Chemical_Compaction_Term  = FiniteElementMethod::innerProduct ( Basis, Current_Element_Chemical_Compaction );

        previousPorosity = lithology->porosity ( previousVes, previousMaxVes, includeChemicalCompaction, Previous_Chemical_Compaction_Term );

        lithology->getPorosity ( currentVes, currentMaxVes, includeChemicalCompaction, Current_Chemical_Compaction_Term, currentCompoundPorosity );
        currentPorosity = currentCompoundPorosity.mixedProperty ();

        previousFluidDensity = Fluid->density ( previousTemperature, Pa_To_MPa * previousPorePressure );
        currentFluidDensity  = Fluid->density ( currentTemperature,  Pa_To_MPa * currentPorePressure );
        fluidViscosity = Fluid->viscosity ( currentTemperature );

        currentFluidDensityDerivative = Pa_To_MPa * Fluid->computeDensityDerivativeWRTPressure ( currentTemperature, Pa_To_MPa * currentPorePressure );

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
        currentFluidDensityTerm = integrationWeight * timeStepInv * (( usedWaterSaturation * currentFluidDensity * currentPorosity ) +
                                                                       usedWaterSaturation * currentFluidDensity * currentPorosity / ( 1.0 - currentPorosity ));

        //
        // Term 2
        //
        previousFluidDensityTerm = integrationWeight * timeStepInv * ( usedWaterSaturation * previousFluidDensity * currentPorosity + 
                                                                       usedWaterSaturation * currentFluidDensity * previousPorosity / ( 1.0 - currentPorosity ));

        //
        // Term 1
        //
        // currentFluidDensityTerm = integrationWeight * timeStepInv * (( usedWaterSaturation * currentFluidDensity * currentPorosity ) +
        //                                                                usedWaterSaturation * currentFluidDensity * currentPorosity / ( 1.0 - currentPorosity ) + 
        //                                                                currentFluidDensity * currentPorosity / ( 1.0 - currentPorosity ) * usedWaterSaturation );

        // //
        // // Term 2
        // //
        // previousFluidDensityTerm = integrationWeight * timeStepInv * ( usedWaterSaturation * previousFluidDensity * currentPorosity + 
        //                                                                usedWaterSaturation * currentFluidDensity * previousPorosity / ( 1.0 - currentPorosity ) + 
        //                                                                currentFluidDensity * currentPorosity / ( 1.0 - currentPorosity ) * usedPreviousWaterSaturation ));



        //
        // Term 3
        //
        matrixTransposeVectorProduct ( gradBasis, Current_Po, referenceGradOverpressure );
        matrixTransposeVectorProduct ( jacobianInverse, referenceGradOverpressure, gradOverpressure );

        matrixTransposeVectorProduct ( gradBasis, Current_Ph, referenceGradHydrostaticPressure );
        matrixTransposeVectorProduct ( jacobianInverse, referenceGradHydrostaticPressure, gradHydrostaticPressure );

        gradOverpressure ( 1 ) += gradHydrostaticPressure ( 1 );
        gradOverpressure ( 2 ) += gradHydrostaticPressure ( 2 );


        computeFluidMobilityTerms ( false,
                                    Has_Fractured,
                                    fractureScaling,
                                    currentVes,
                                    currentMaxVes,
                                    currentCompoundPorosity, 
                                    currentFluidDensity,
                                    currentFluidDensityDerivative,
                                    fluidViscosity,
                                    relativePermeability,
                                    Jacobian,
                                    gradOverpressure,
                                    lithology,
                                    fluidVelocity,
                                    fluidMobility,
                                    fluidVelocityDerivative );

        matrixVectorProduct ( scaledGradBasis, fluidVelocity, Term_3 );


        double Scaling = -currentFluidDensityTerm + previousFluidDensityTerm;

        Increment ( Scaling, Basis, Element_Residual );
        Increment ( -integrationWeight, Term_3, Element_Residual );

        //----------------------------//

        //
        // The fluid density derivative is scaled by Pa_To_MPa because the fluid density function 
        // requires the pressure to be in MPa. The pressure that is computed here is in Pa.
        //

#if 1
        dPhiDP = lithology -> computePorosityDerivativeWRTPressure ( currentVes, currentMaxVes,
                                                                     includeChemicalCompaction,
                                                                     Current_Chemical_Compaction_Term );

        dRhoDP = Pa_To_MPa * Fluid->computeDensityDerivativeWRTPressure ( currentTemperature, Pa_To_MPa * currentPorePressure );

        // Bulk_Fluid_Density_Derivative = waterSaturation * dPhiDP * currentFluidDensity + waterSaturation * currentPorosity * dRhoDP - waterSaturation * dPhiDP * previousFluidDensity
        //    + ( 1.0 / ( 1.0 - waterSaturation * currentPorosity )) * ( 2.0 * dRhoDP * waterSaturation * currentPorosity + 2.0 * currentFluidDensity * waterSaturation * dPhiDP - dRhoDP * waterSaturation * previousPorosity
        //                                                               - dRhoDP * currentPorosity * previousWaterSaturation - currentFluidDensity * dRhoDP * previousWaterSaturation )
        //    + ( 1.0 / (( 1.0 - waterSaturation * currentPorosity ) * ( 1.0 - waterSaturation * currentPorosity ))) * dPhiDP * ( currentFluidDensity * waterSaturation * currentPorosity - currentFluidDensity * waterSaturation * previousPorosity -
        //                                                                                                                        currentFluidDensity * previousWaterSaturation * currentPorosity );
           

        bulkFluidDensityDerivative = dRhoDP * usedWaterSaturation * currentPorosity / ( 1.0 - currentPorosity ) +
           currentFluidDensity * usedWaterSaturation / ( 1.0 - currentPorosity ) * dPhiDP +
           currentFluidDensity * usedWaterSaturation * currentPorosity / pow ( 1.0 - currentPorosity, 2 ) * dPhiDP;

        addOuterProduct ( integrationWeight * bulkFluidDensityDerivative * timeStepInv, Basis, Basis, Element_Jacobian );

#else
        // Need to add saturation to this section then remove "if 0" branch 

        dPhiDP = lithology -> computePorosityDerivativeWRTPressure ( currentVes, currentMaxVes,
                                                                     includeChemicalCompaction,
                                                                     Current_Chemical_Compaction_Term );

        dRhoDP = Pa_To_MPa * Fluid->computeDensityDerivativeWRTPressure ( currentTemperature, Pa_To_MPa * currentPorePressure );


        t1 = dRhoDP * currentPorosity / ( 1.0 - currentPorosity ) +
             currentFluidDensity / ( 1.0 - currentPorosity ) * dPhiDP +
             currentFluidDensity * currentPorosity / pow ( 1.0 - currentPorosity, 2 ) * dPhiDP;

        t2 = dRhoDP * currentPorosity + currentFluidDensity * dPhiDP;

        t3 = -dRhoDP * previousPorosity / ( 1.0 - currentPorosity ) - currentFluidDensity * previousPorosity / pow ( 1.0 - currentPorosity, 2 ) * dPhiDP;

        t4 = -previousFluidDensity * dPhiDP;

        bulkFluidDensityDerivative = t1 + t2 + t3 + t4;

        addOuterProduct ( integrationWeight * bulkFluidDensityDerivative * timeStepInv, Basis, Basis, Element_Jacobian );
#endif



        // double dPhiDp = lithology -> computePorosityDerivativeWRTPressure ( Current_VES, Current_Max_VES,
        //                                                                     includeChemicalCompaction,
        //                                                                     Current_Chemical_Compaction_Term );

        // double dRhoDp = Pa_To_MPa * Fluid->computeDensityDerivativeWRTPressure ( Current_Temperature, Pa_To_MPa * Current_Pore_Pressure );

        // T3a + T3b
        matrixVectorProduct ( scaledGradBasis, fluidVelocityDerivative, Term_3 );
        addOuterProduct ( integrationWeight, Term_3, Basis, Element_Jacobian );

        // T3c
        matrixMatrixProduct ( scaledGradBasis, fluidMobility, scaledGradBasis2 );
        addOuterProduct ( integrationWeight, scaledGradBasis, scaledGradBasis2, Element_Jacobian );
      }

    }

  }



#if 0
  if ( BCs [ 4 ] == Bottom_Boundary_Flux ) {
     applyPressureNeumannBoundaryConditions ( geometryMatrix,
                                              lithology,
                                              Fluid,
                                              includeChemicalCompaction,
                                              BCs,
                                              Current_Ph,
                                              Current_Po,
                                              Current_Element_VES,
                                              Current_Element_Max_VES,
                                              Current_Element_Temperature,
                                              Current_Element_Chemical_Compaction,
                                              Element_Residual );
  }

  for ( I = 0; I < 6; ++I ) {

    static const double DomainNormals [] = { 1.0, 1.0, 1.0, 1.0, 1.0, 1.0 };

    if ( Element_BCs [ I ] == Side_Neumann_Boundary ) {
      applyPressureNeumannBoundaryConditions ( Element_BCs [ I ], DomainNormals [ I ]);
    }

  }
#endif

  Apply_Dirichlet_Boundary_Conditions_Newton ( BCs, Dirichlet_Boundary_Values, Dirichlet_Scaling_Value,
                                               Current_Po, Element_Jacobian, Element_Residual );

}

//------------------------------------------------------------//

void Basin_Modelling::Assemble_Element_Temperature_System
   ( const bool                     isBasementFormation,
     const int                      planeQuadratureDegree,
     const int                      depthQuadratureDegree,
     const double                   currentTime,
     const double                   timeStep,
     const bool                     Is_Steady_State,
     const bool                     Include_Advection_Term,
     const Boundary_Conditions*     BCs,
     const ElementVector&          Dirichlet_Boundary_Values,
     const CompoundLithology*       lithology,
     const GeoPhysics::FluidType*   Fluid,
     const bool                     includeChemicalCompaction,

     const ElementGeometryMatrix& previousGeometryMatrix,
     const ElementGeometryMatrix& geometryMatrix,

     const ElementVector&          Element_Heat_Production,
     const ElementVector&          Previous_Ph,
     const ElementVector&          Current_Ph,
     const ElementVector&          Previous_Po,
     const ElementVector&          Current_Po,
     const ElementVector&          Current_Lp,
     const ElementVector&          Previous_Element_VES,
     const ElementVector&          Current_Element_VES,
     const ElementVector&          Previous_Element_Max_VES,
     const ElementVector&          Current_Element_Max_VES,
     const ElementVector&          Previous_Element_Temperature,
     const ElementVector&          Current_Element_Temperature,

     const ElementVector&          Previous_Element_Chemical_Compaction,
     const ElementVector&          Current_Element_Chemical_Compaction,

           ElementMatrix&          Element_Jacobian,
           ElementVector&          Element_Residual ) {


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
  ThreeVector        gradHydrostaticPressure;
  ThreeVector        referenceGradHydrostaticPressure;

  Matrix3x3          Fluid_Mobility;
  ThreeVector        Fluid_Velocity;
  ThreeVector        Fluid_Velocity_Intermediate;
  BasisFunction basisFunction;

  double Fluid_Density_Heat_Capacity;
  double Fluid_Viscosity;

  ThreeVector        gradSurfaceDepth;
  ThreeVector        referenceGradSurfaceDepth;

  ElementVector      Basis;
  ElementVector      Work_Space;
  ElementVector      Term_3;
  GradElementVector Grad_Basis;
  GradElementVector Scaled_Grad_Basis;
  GradElementVector Scaled_Grad_Basis2;

  Matrix3x3          Jacobian;
  Matrix3x3          Jacobian_Inverse;
  Matrix3x3          Previous_Jacobian;


  ThreeVector        Heat_Flow;
  Matrix3x3          Conductivity_Tensor;

  double Current_VES;
  double Current_Max_VES;
  double Current_Hydrostatic_Pressure;
  double Current_Overpressure;
  double Current_LithostaticPressure;
  double Current_Pore_Pressure;
  double Previous_Temperature;
  double Current_Temperature;
  double Current_Porosity;
  CompoundProperty Current_Compound_Porosity;

  double Current_Chemical_Compaction_Term;
  double Current_Bulk_Density_X_Capacity;
  double Heat_Source_Term;
  double Scaling;

  Element_Residual.zero ();
  Element_Jacobian.zero ();

  if ( Degenerate_Element ( geometryMatrix )) {
    //
    // This is only temporary until I sort out the whole degenerate segment thing!
    //
    Apply_Dirichlet_Boundary_Conditions_Newton ( BCs, Dirichlet_Boundary_Values, Dirichlet_Scaling_Value,
                                                 Current_Element_Temperature, Element_Jacobian, Element_Residual );
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
        matrixMatrixProduct ( previousGeometryMatrix, Grad_Basis, Previous_Jacobian );

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
        Current_LithostaticPressure = FiniteElementMethod::innerProduct ( Basis, Current_Lp );
        Current_Pore_Pressure  = Current_Hydrostatic_Pressure  + Current_Overpressure;
        Current_Temperature  = FiniteElementMethod::innerProduct ( Basis, Current_Element_Temperature );
        Previous_Temperature = FiniteElementMethod::innerProduct ( Basis, Previous_Element_Temperature );

        Current_Chemical_Compaction_Term  = FiniteElementMethod::innerProduct ( Basis, Current_Element_Chemical_Compaction );

        lithology->getPorosity ( Current_VES, Current_Max_VES, includeChemicalCompaction, Current_Chemical_Compaction_Term, Current_Compound_Porosity );
        Current_Porosity = Current_Compound_Porosity.mixedProperty ();

        //----------------------------//

        //
        // Term 1
        //
        lithology -> calcBulkDensXHeatCapacity ( Fluid,
                                                 Current_Porosity,
                                                 Current_Pore_Pressure,
                                                 Current_Temperature,
                                                 Current_LithostaticPressure,
                                                 Current_Bulk_Density_X_Capacity );

        Scaling = Current_Bulk_Density_X_Capacity * Current_Temperature * integrationWeight * timeStepInv;
        Increment ( -Scaling, Basis, Element_Residual );

        //
        // Term 2
        //
        Scaling = Current_Bulk_Density_X_Capacity * Previous_Temperature * integrationWeight * timeStepInv;
        Increment ( Scaling, Basis, Element_Residual );

        //
        // Term 3
        //
        Compute_Heat_Flow ( isBasementFormation,
                            lithology,
                            Fluid,
                            Current_Element_Temperature,
                            Current_Temperature,
                            Current_Porosity,
                            Current_LithostaticPressure,
                            Jacobian,
                            Scaled_Grad_Basis,
                            Heat_Flow,
                            Conductivity_Tensor );

        matrixVectorProduct ( Scaled_Grad_Basis, Heat_Flow, Term_3 );
        Increment ( -integrationWeight, Term_3, Element_Residual );

        //
        // Term 4
        //
        Heat_Source_Term = FiniteElementMethod::innerProduct ( Element_Heat_Production, Basis );
        Increment ( integrationWeight * Heat_Source_Term, Basis, Element_Residual );

        //----------------------------//
        //
        // Jacobian
        //
        //

        //
        // Term 1
        //
        addOuterProduct ( integrationWeight * Current_Bulk_Density_X_Capacity * timeStepInv, Basis, Basis, Element_Jacobian );

        //
        // Term 3
        //
        matrixMatrixProduct ( Scaled_Grad_Basis, Conductivity_Tensor, Scaled_Grad_Basis2 );
        addOuterProduct ( integrationWeight, Scaled_Grad_Basis, Scaled_Grad_Basis2, Element_Jacobian );

        //
        // Convection term, if required
        //
        if ( Include_Advection_Term && Fluid != 0 ) {

          Fluid_Density_Heat_Capacity = Fluid->densXheatCapacity ( Current_Temperature, Current_Pore_Pressure );
          Fluid_Viscosity = Fluid->viscosity ( Current_Temperature );

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

          Compute_Fluid_Velocity ( true,
                                   Has_Fractured,
                                   Current_VES,
                                   Current_Max_VES,
                                   Current_Compound_Porosity,
                                   Fluid_Viscosity,
                                   Jacobian,
                                   Grad_Overpressure,
                                   lithology,
                                   Fluid_Velocity );

          Increment ( -integrationWeight * Fluid_Density_Heat_Capacity * FiniteElementMethod::innerProduct ( Grad_Temperature, Fluid_Velocity ), 
                      Basis,
                      Element_Residual );


          matrixVectorProduct ( Jacobian_Inverse, Fluid_Velocity, Fluid_Velocity_Intermediate );
          matrixVectorProduct ( Grad_Basis, Fluid_Velocity_Intermediate, Work_Space );

          scale ( Work_Space, -integrationWeight * Current_Porosity * Fluid_Density_Heat_Capacity );
          addOuterProduct ( Work_Space, Basis, Element_Jacobian );
        }


      }

    }

  }

  //
  // Need the Neumann BCs for the heat flow.
  //
  if ( BCs [ 4 ] == Bottom_Boundary_Flux ) {
    //
    //
    // If one bottom node is then all nodes will be.
    //
    Apply_Heat_Flow_Boundary_Conditions ( geometryMatrix, Dirichlet_Boundary_Values, Element_Residual );
  }


  Apply_Dirichlet_Boundary_Conditions_Newton ( BCs, Dirichlet_Boundary_Values, Dirichlet_Scaling_Value,
                                               Current_Element_Temperature, Element_Jacobian, Element_Residual );

}

//------------------------------------------------------------//


void Basin_Modelling::Assemble_Element_Temperature_Residual
   ( const bool                          isBasementFormation,
     const int                           planeQuadratureDegree,
     const int                           depthQuadratureDegree,
     const double                        currentTime,
     const double                        timeStep,
     const bool                          Is_Steady_State,
     const bool                          Include_Advection_Term,
     const Boundary_Conditions*          BCs,
     const ElementVector&               Dirichlet_Boundary_Values,
     const CompoundLithology*            lithology,
     const GeoPhysics::FluidType*        Fluid,
     const bool                          includeChemicalCompaction,

     const ElementGeometryMatrix&      previousGeometryMatrix,
     const ElementGeometryMatrix&      geometryMatrix,

     const ElementVector&               Element_Heat_Production,
     const ElementVector&               Previous_Ph,
     const ElementVector&               Current_Ph,
     const ElementVector&               Previous_Po,
     const ElementVector&               Current_Po,
     const ElementVector&               Current_Lp,
     const ElementVector&               Previous_Element_VES,
     const ElementVector&               Current_Element_VES,
     const ElementVector&               Previous_Element_Max_VES,
     const ElementVector&               Current_Element_Max_VES,
     const ElementVector&               Previous_Element_Temperature,
     const ElementVector&               Current_Element_Temperature,

     const ElementVector&               Previous_Element_Chemical_Compaction,
     const ElementVector&               Current_Element_Chemical_Compaction,

           ElementVector&               Element_Residual ) {


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
  ThreeVector        Fluid_Velocity;
  ThreeVector        Fluid_Velocity_Intermediate;
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
  Matrix3x3          Previous_Jacobian;

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

  Element_Residual.zero ();


  if ( Degenerate_Element ( geometryMatrix )) {
    //
    // This is only temporary until I sort out the whole degenerate segment thing!
    //
    Apply_Dirichlet_Boundary_Conditions_Newton ( BCs, Dirichlet_Boundary_Values, Dirichlet_Scaling_Value,
                                                 Current_Element_Temperature, Element_Residual );
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
        matrixMatrixProduct ( previousGeometryMatrix, Grad_Basis, Previous_Jacobian );

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
        lithology -> calcBulkDensXHeatCapacity ( Fluid,
                                                 Current_Porosity,
                                                 Current_Pore_Pressure,
                                                 Current_Temperature,
        	                                 Current_LithostaticPressure,
                                                 Current_Bulk_Density_X_Capacity );

        Scaling = Current_Bulk_Density_X_Capacity * Current_Temperature * integrationWeight * timeStepInv;
        Increment ( -Scaling, Basis, Element_Residual );

        //
        // Term 2
        //
        Scaling = Current_Bulk_Density_X_Capacity * Previous_Temperature * integrationWeight * timeStepInv;
        Increment ( Scaling, Basis, Element_Residual );

        //
        // Term 3
        //
        Compute_Heat_Flow ( isBasementFormation,
                            lithology,
                            Fluid,
                            Current_Element_Temperature,
                            Current_Temperature,
                            Current_Porosity,
                            Current_LithostaticPressure,
                            Jacobian,
                            Scaled_Grad_Basis,
                            Heat_Flow,
                            Conductivity_Tensor );

        matrixVectorProduct ( Scaled_Grad_Basis, Heat_Flow, Term_3 );
        Increment ( -integrationWeight, Term_3, Element_Residual );

        //
        // Term 4
        //
        Heat_Source_Term = FiniteElementMethod::innerProduct ( Element_Heat_Production, Basis );
        Increment ( integrationWeight * Heat_Source_Term, Basis, Element_Residual );


        //
        // Convection term, if required
        //
        if ( Include_Advection_Term && Fluid != 0 ) {

          Fluid_Density_Heat_Capacity = Fluid->densXheatCapacity ( Current_Temperature, Current_Pore_Pressure );
          Fluid_Viscosity = Fluid->viscosity ( Current_Temperature );

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

          Compute_Fluid_Velocity ( true,
                                   Has_Fractured,
                                   Current_VES,
                                   Current_Max_VES,
                                   Current_Compound_Porosity,
                                   Fluid_Viscosity,
                                   Jacobian,
                                   Grad_Overpressure,
                                   lithology,
                                   Fluid_Velocity );

          Increment ( -integrationWeight * Fluid_Density_Heat_Capacity * FiniteElementMethod::innerProduct ( Grad_Temperature, Fluid_Velocity ), 
                      Basis,
                      Element_Residual );
        }

      }

    }

  }

  // Need the Neumann BCs for the heat flow.
  if ( BCs [ 4 ] == Bottom_Boundary_Flux ) {
    // If one bottom node is Neumann bc then all nodes will be.
    Apply_Heat_Flow_Boundary_Conditions ( geometryMatrix, Dirichlet_Boundary_Values, Element_Residual );
  }

  Apply_Dirichlet_Boundary_Conditions_Newton ( BCs, Dirichlet_Boundary_Values, Dirichlet_Scaling_Value,
                                               Current_Element_Temperature, Element_Residual );

}

//------------------------------------------------------------//


void Basin_Modelling::Assemble_Element_Temperature_Stiffness_Matrix
     ( const bool                          isBasementFormation,
       const int                           planeQuadratureDegree,
       const int                           depthQuadratureDegree,
       const double                        currentTime,
       const double                        timeStep,
       const bool                          Is_Steady_State,
       const bool                          Include_Advection_Term,
       const Boundary_Conditions*          BCs,
       const ElementVector&               Nodal_BC_Values,
       const CompoundLithology*            lithology,
       const GeoPhysics::FluidType*        Fluid,
       const bool                          includeChemicalCompaction,

       const ElementGeometryMatrix&       geometryMatrix,
       const ElementVector&               Element_Heat_Production,
       const ElementVector&               Current_Ph,
       const ElementVector&               Current_Po,

       const ElementVector&               Previous_Pp,
       const ElementVector&               Current_Pp,
       const ElementVector&               Previous_Lp,
       const ElementVector&               Current_Lp,
       const ElementVector&               Previous_Element_VES,
       const ElementVector&               Current_Element_VES,

       const ElementVector&               Previous_Element_Max_VES,
       const ElementVector&               Current_Element_Max_VES,

       const ElementVector&               Previous_Element_Temperature,
       const ElementVector&               Current_Element_Temperature,
       const ElementVector&               Previous_Element_Chemical_Compaction,
       const ElementVector&               Current_Element_Chemical_Compaction,

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
  ThreeVector        Fluid_Velocity;
  ThreeVector        Fluid_Velocity_Intermediate;
  ThreeVector        jacobianInverseFluidVelocity;


  double Fluid_Density_Heat_Capacity;
  double Fluid_Viscosity;

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

  if ( Degenerate_Element ( geometryMatrix )) {
    //
    // This is only temporary until I sort out the whole degenerate segment thing!
    //
    Apply_Dirichlet_Boundary_Conditions_Linear ( BCs, Nodal_BC_Values, Dirichlet_Scaling_Value,
                                                 Element_Stiffness_Matrix, Element_Load_Vector );
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
                                                 Current_Temperature > Previous_Temperature,
                                                 Current_Bulk_Density_X_Capacity );

        lithology -> calcBulkDensXHeatCapacity ( Fluid,
                                                 Previous_Porosity,
                                                 Previous_Pore_Pressure,
                                                 Previous_Temperature,
                                                 PreviousLithoPressure,
                                                 Current_Temperature > Previous_Temperature,
                                                 Previous_Bulk_Density_X_Capacity );

        // Load_Terms = Current_Bulk_Density_X_Capacity * Previous_Temperature * timeStepInv;

        double t1 = Current_Bulk_Density_X_Capacity / ( 1.0 - Current_Porosity ) * ( Current_Porosity - Previous_Porosity );
        double t2 = ( 2.0 * Current_Bulk_Density_X_Capacity - Previous_Bulk_Density_X_Capacity );
        double t3 = Previous_Bulk_Density_X_Capacity * Previous_Temperature;
           
        //
        // Term 4
        //
        Heat_Source_Term = innerProduct ( Element_Heat_Production, Basis );
        Load_Terms = Heat_Source_Term + t3 * timeStepInv;
        // Load_Terms = Load_Terms + Heat_Source_Term;

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
 
           lithology -> calcBulkThermCondNP ( Fluid, Current_Porosity, Current_Temperature, Conductivity_Normal, Conductivity_Tangential );
        }

        Set_Heat_Conductivity_Tensor ( Conductivity_Normal, Conductivity_Tangential, Jacobian, Conductivity_Tensor );

        matrixMatrixProduct ( Jacobian_Inverse, Conductivity_Tensor, Middle_Term2 );
        matrixMatrixTransposeProduct ( Middle_Term2, Jacobian_Inverse, Middle_Term );
        scale ( Middle_Term, integrationWeight );

        matrixMatrixProduct ( Grad_Basis, Middle_Term, Scaled_Grad_Basis );

        addOuterProduct ( Scaled_Grad_Basis, Grad_Basis, Element_Stiffness_Matrix );

        // Add in the Advection term if required.
        if ( Include_Advection_Term && Fluid != 0 ) {

          Fluid_Density_Heat_Capacity = Fluid->densXheatCapacity ( Current_Temperature, Current_Pore_Pressure );
          Fluid_Viscosity = Fluid->viscosity ( Current_Temperature );

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

          Compute_Fluid_Velocity ( true,
                                   Has_Fractured,
                                   Current_VES,
                                   Current_Max_VES,
                                   Current_Compound_Porosity,
                                   Fluid_Viscosity,
                                   Jacobian,
                                   Grad_Overpressure,
                                   lithology,
                                   Fluid_Velocity );

          Fluid_Velocity ( 1 ) = Fluid_Density_Heat_Capacity * integrationWeight * Fluid_Velocity ( 1 );
          Fluid_Velocity ( 2 ) = Fluid_Density_Heat_Capacity * integrationWeight * Fluid_Velocity ( 2 );
          Fluid_Velocity ( 3 ) = Fluid_Density_Heat_Capacity * integrationWeight * Fluid_Velocity ( 3 );

          matrixVectorProduct ( Jacobian_Inverse, Fluid_Velocity, jacobianInverseFluidVelocity );
          matrixVectorProduct ( Grad_Basis, jacobianInverseFluidVelocity, Work_Space );

          addOuterProduct ( Basis, Work_Space, Element_Stiffness_Matrix );
        }

      }

    }

  }

  // Need the Neumann BCs for the heat flow.
  if ( BCs [ 4 ] == Bottom_Boundary_Flux ) {

    // If one bottom node is Neumann then all nodes will be.
    Apply_Heat_Flow_Boundary_Conditions ( geometryMatrix, Nodal_BC_Values, Element_Load_Vector );
  }

  Apply_Dirichlet_Boundary_Conditions_Linear ( BCs, Nodal_BC_Values, Dirichlet_Scaling_Value,
                                               Element_Stiffness_Matrix, Element_Load_Vector );

}

//------------------------------------------------------------//
