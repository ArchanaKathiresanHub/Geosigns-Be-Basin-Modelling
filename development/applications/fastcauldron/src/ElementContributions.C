#include "mpi.h"

#include "ElementContributions.h"


#include "FiniteElementTypes.h"
#include "BasisFunction.h"
#include "FiniteElement.h"

#include "element_contributions.h"

#include "Quadrature.h"
#include "Quadrature3D.h"


#include "FastcauldronSimulator.h"
#include "layer.h"
#include "Lithology.h"
#include "globaldefs.h"


using namespace FiniteElementMethod;

//------------------------------------------------------------//

void getCoefficients ( const LayerElement&                 element,
                       const PETSC_3D_Array&               property,
                       FiniteElementMethod::ElementVector& coefficients ) {

   for ( int i = 0; i < 8; ++i ) {
      coefficients ( i + 1 ) = property ( element.getNodeLocalKPosition ( i ),
                                          element.getNodeJPosition ( i ),
                                          element.getNodeIPosition ( i ));

   }

}

//------------------------------------------------------------//

void getCoefficients ( const LayerElement&                         element,
                       const Basin_Modelling::Fundamental_Property property,
                             FiniteElementMethod::ElementVector&   coefficients ) {

   const LayerProps* layer = element.getFormation ();

   int i;

   for ( i = 1; i <= 8; ++i ) {
      coefficients ( i ) = layer->Current_Properties ( property, 
                                                       element.getNodeLocalKPosition ( i - 1 ),
                                                       element.getNodeJPosition ( i - 1 ),
                                                       element.getNodeIPosition ( i - 1 ));

   }

}

//------------------------------------------------------------//

void getPreviousCoefficients ( const LayerElement&                         element,
                               const Basin_Modelling::Fundamental_Property property,
                                     FiniteElementMethod::ElementVector&   coefficients ) {

   PetscBool isValid;
   int i;

   const LayerProps* layer = element.getFormation ();

   Vec propertyVector = const_cast<Vec>(layer->Previous_Properties ( property ));

   // VecValid ( propertyVector, &isValid ); not in 3.3
   isValid = ( propertyVector != 0 ? PETSC_TRUE : PETSC_FALSE );
   // VecValid ( layer->Previous_Properties ( property ), &isValid );
   
   if ( isValid ) {

      for ( i = 1; i <= 8; ++i ) {
         coefficients ( i ) = layer->Previous_Properties ( property, 
                                                           element.getNodeLocalKPosition ( i - 1 ),
                                                           element.getNodeJPosition ( i - 1 ),
                                                           element.getNodeIPosition ( i - 1 ));
      }

   } else {
      // Temporary fix, should pass isValid as parameter.
      coefficients.zero ();
   }

}

//------------------------------------------------------------//

void interpolateCoefficients ( const LayerElement&                         element,
                               const Basin_Modelling::Fundamental_Property property,
                                     FiniteElementMethod::ElementVector&   coefficients,
                               const double                                lambda ) {

   FiniteElementMethod::ElementVector previous;
   FiniteElementMethod::ElementVector current;

   // Get the properties
   getCoefficients ( element, property, current );
   getPreviousCoefficients ( element, property, previous );

   // Interpolate to required position.
   interpolate ( previous, current, lambda, coefficients );

}

//------------------------------------------------------------//

void getGeometryMatrix ( const LayerElement&    element,
                         ElementGeometryMatrix& geometryMatrix ) {

   const CauldronGridDescription& grid = FastcauldronSimulator::getInstance ().getCauldronGridDescription ();

   const LayerProps* layer = element.getFormation ();

   double deltaX  = grid.deltaI;
   double deltaY  = grid.deltaJ;
   double originX = grid.originI;
   double originY = grid.originJ;

   int i;

   for ( i = 1; i <= 8; ++i ) {
      geometryMatrix ( 1, i ) = originX + deltaX * element.getNodeIPosition ( i - 1 );
      geometryMatrix ( 2, i ) = originY + deltaY * element.getNodeJPosition ( i - 1 );

      geometryMatrix ( 3, i ) = layer->Current_Properties ( Basin_Modelling::Depth, 
                                                            element.getNodeLocalKPosition ( i - 1 ),
                                                            element.getNodeJPosition ( i - 1 ),
                                                            element.getNodeIPosition ( i - 1 ));

   }

}

//------------------------------------------------------------//

void getGeometryMatrix ( const LayerElement&    element,
                         ElementGeometryMatrix& geometryMatrix,
                         const double           lambda ) {

   const CauldronGridDescription& grid = FastcauldronSimulator::getInstance ().getCauldronGridDescription ();

   const LayerProps* layer = element.getFormation ();

   FiniteElementMethod::ElementVector depth;
   double deltaX  = grid.deltaI;
   double deltaY  = grid.deltaJ;
   double originX = grid.originI;
   double originY = grid.originJ;

   int i;

   interpolateCoefficients ( element, Basin_Modelling::Depth, depth, lambda );

   for ( i = 1; i <= 8; ++i ) {
      geometryMatrix ( 1, i ) = originX + deltaX * element.getNodeIPosition ( i - 1 );
      geometryMatrix ( 2, i ) = originY + deltaY * element.getNodeJPosition ( i - 1 );
      geometryMatrix ( 3, i ) = depth ( i );

   }

}

//------------------------------------------------------------//

void fillGeometryMatrix ( const LayerElement&                       element,
                                FiniteElementMethod::FiniteElement& finiteElement ) {

   const CauldronGridDescription& grid = FastcauldronSimulator::getInstance ().getCauldronGridDescription ();

   const LayerProps* layer = element.getFormation ();

   double deltaX  = grid.deltaI;
   double deltaY  = grid.deltaJ;
   double originX = grid.originI;
   double originY = grid.originJ;

   unsigned int i;

   for ( i = 1; i <= 8; ++i ) {
      finiteElement.setGeometryPoint ( i, 
                                       originX + deltaX * element.getNodeIPosition ( i - 1 ),
                                       originY + deltaY * element.getNodeJPosition ( i - 1 ),
                                       layer->Current_Properties ( Basin_Modelling::Depth, 
                                                                   element.getNodeLocalKPosition ( i - 1 ),
                                                                   element.getNodeJPosition ( i - 1 ),
                                                                   element.getNodeIPosition ( i - 1 )));
   }

}

//------------------------------------------------------------//

double computeProperty ( const LayerElement&                         element,
                         const Basin_Modelling::Fundamental_Property property,
                         const double                                x,
                         const double                                y,
                         const double                                z ) {

   FiniteElementMethod::BasisFunction basis;
   FiniteElementMethod::ElementVector phi;
   FiniteElementMethod::ElementVector coefficients;

   basis ( x, y, z, phi );
   getCoefficients ( element, property, coefficients );

   return FiniteElementMethod::innerProduct ( coefficients, phi );
}

//------------------------------------------------------------//

double computePreviousProperty ( const LayerElement&                         element,
                                 const Basin_Modelling::Fundamental_Property property,
                                 const double                                x,
                                 const double                                y,
                                 const double                                z ) {

   FiniteElementMethod::BasisFunction basis;
   FiniteElementMethod::ElementVector phi;
   FiniteElementMethod::ElementVector coefficients;

   basis ( x, y, z, phi );
   getPreviousCoefficients ( element, property, coefficients );

   return FiniteElementMethod::innerProduct ( coefficients, phi );
}

//------------------------------------------------------------//

double interpolateProperty ( const LayerElement&                         element,
                             const Basin_Modelling::Fundamental_Property property,
                             const double                                lambda,
                             const double                                x,
                             const double                                y,
                             const double                                z ) {

   FiniteElementMethod::BasisFunction basis;
   FiniteElementMethod::ElementVector phi;
   FiniteElementMethod::ElementVector coefficients;

   basis ( x, y, z, phi );
   interpolateCoefficients ( element, property, coefficients, lambda );
   // getCoefficients ( element, property, coefficients );

   return FiniteElementMethod::innerProduct ( coefficients, phi );
}

//------------------------------------------------------------//

ThreeVector computeGradientProperty ( const LayerElement&                         element,
                                      const FiniteElementMethod::Matrix3x3&       jacobian,
                                      const Basin_Modelling::Fundamental_Property property,
                                      const double                                x,
                                      const double                                y,
                                      const double                                z ) {

   BasisFunction     basis;
   ThreeVector       gradient;
   ThreeVector       referenceGradient;
   Matrix3x3         jacobianInverse;
   GradElementVector gradBasis;
   ElementVector     phi;
   ElementVector     coefficients;

   basis ( x, y, z, gradBasis );

   getCoefficients ( element, property, coefficients );

   matrixTransposeVectorProduct ( gradBasis, coefficients, referenceGradient );
   matrixTransposeVectorProduct ( jacobianInverse, referenceGradient, gradient );

   return gradient;
}

//------------------------------------------------------------//

double volumeOfElement ( const LayerElement&                               element,
                         const FiniteElementMethod::ElementGeometryMatrix& geometryMatrix,
                         const int                                         numberOfQuadraturePoints ) {

   NumericFunctions::Quadrature3D::Iterator quad;
   FiniteElementMethod::FiniteElement finiteElement;
   double elementVolume = 0.0;
   double weight;

   finiteElement.setGeometry ( geometryMatrix );

   NumericFunctions::Quadrature3D::getInstance ().get ( numberOfQuadraturePoints, quad );

   for ( quad.initialise (); not quad.isDone (); ++quad ) {
      finiteElement.setQuadraturePoint ( quad.getX (), quad.getY (), quad.getZ ());
      weight = determinant ( finiteElement.getJacobian ()) * quad.getWeight ();
      elementVolume += weight;
   }

   return elementVolume;
}

//------------------------------------------------------------//

double volumeOfElement ( const LayerElement&  element,
                         const int            numberOfQuadraturePoints ) {

   FiniteElementMethod::ElementGeometryMatrix geometryMatrix;

   getGeometryMatrix ( element, geometryMatrix );

   return volumeOfElement ( element, geometryMatrix, numberOfQuadraturePoints );
}

//------------------------------------------------------------//

double poreVolumeOfElement ( const LayerElement&                               element,
                             const FiniteElementMethod::ElementGeometryMatrix& geometryMatrix,
                             const int                                         numberOfQuadraturePoints ) {

   double elementVolume;
   double elementPoreVolume;

   elementVolumeCalculations ( element, geometryMatrix, elementVolume, elementPoreVolume, numberOfQuadraturePoints );

   return elementPoreVolume;
}

//------------------------------------------------------------//

double poreVolumeOfElement ( const LayerElement&  element,
                             const int            numberOfQuadraturePoints ) {

   FiniteElementMethod::ElementGeometryMatrix geometryMatrix;

   getGeometryMatrix ( element, geometryMatrix );

   return poreVolumeOfElement ( element, geometryMatrix, numberOfQuadraturePoints );
}

//------------------------------------------------------------//

double averageElementPorosity ( const LayerElement&                               element,
                                const FiniteElementMethod::ElementGeometryMatrix& geometryMatrix,
                                const int                                         numberOfQuadraturePoints ) {

   double elementPoreVolume;
   double elementVolume;

   elementVolumeCalculations ( element, geometryMatrix, elementVolume, elementPoreVolume, numberOfQuadraturePoints );
   return elementPoreVolume / elementVolume;
}

//------------------------------------------------------------//

double averageElementPorosity ( const LayerElement&  element,
                                const int            numberOfQuadraturePoints ) {

   FiniteElementMethod::ElementGeometryMatrix geometryMatrix;

   getGeometryMatrix ( element, geometryMatrix );

   return averageElementPorosity ( element, geometryMatrix, numberOfQuadraturePoints );
}

//------------------------------------------------------------//

void elementVolumeCalculations ( const LayerElement&                               element,
                                       double&                                     elementVolume,
                                       double&                                     elementPoreVolume,
                                 const int                                         numberOfQuadraturePoints ) {

   FiniteElementMethod::ElementGeometryMatrix geometryMatrix;

   getGeometryMatrix ( element, geometryMatrix );
   elementVolumeCalculations ( element, geometryMatrix, elementVolume, elementPoreVolume, numberOfQuadraturePoints );

}

//------------------------------------------------------------//

void elementVolumeCalculations ( const LayerElement&                               element,
                                 const FiniteElementMethod::ElementGeometryMatrix& geometryMatrix,
                                       double&                                     elementVolume,
                                       double&                                     elementPoreVolume,
                                 const int                                         numberOfQuadraturePoints ) {


   const Lithology* lithology = element.getLithology ();

   FiniteElementMethod::FiniteElement finiteElement;
   FiniteElementMethod::ElementVector vesCoeffs;
   FiniteElementMethod::ElementVector maxVesCoeffs;
   NumericFunctions::Quadrature3D::Iterator quad;

   double weight;
   double ves;
   double maxVes;
   double porosity;

   NumericFunctions::Quadrature3D::getInstance ().get ( numberOfQuadraturePoints, quad );
   finiteElement.setGeometry ( geometryMatrix );

   getCoefficients ( element, Basin_Modelling::VES_FP, vesCoeffs );
   getCoefficients ( element, Basin_Modelling::Max_VES, maxVesCoeffs );

   elementVolume = 0.0;
   elementPoreVolume = 0.0;

   for ( quad.initialise (); not quad.isDone (); ++quad ) {
      finiteElement.setQuadraturePoint ( quad.getX (), quad.getY (), quad.getZ ());
      weight = determinant ( finiteElement.getJacobian ()) * quad.getWeight ();
      ves = FiniteElementMethod::innerProduct ( finiteElement.getBasis (), vesCoeffs );
      maxVes = FiniteElementMethod::innerProduct ( finiteElement.getBasis (), maxVesCoeffs );

      porosity = lithology->porosity ( ves, maxVes, false, 0.0 );

      elementVolume += weight;
      elementPoreVolume += porosity * weight;
   }

}

//------------------------------------------------------------//

void elementVolumeCalculations ( const LayerElement&                               element,
                                 const FiniteElementMethod::ElementGeometryMatrix& geometryMatrix,
                                       double&                                     elementVolume,
                                       double&                                     elementPoreVolume,
                                 const double                                      lambda,
                                 const int                                         numberOfQuadraturePoints ) {


   const Lithology* lithology = element.getLithology ();

   FiniteElementMethod::FiniteElement finiteElement;
   FiniteElementMethod::ElementVector vesCoeffs;
   FiniteElementMethod::ElementVector maxVesCoeffs;
   NumericFunctions::Quadrature3D::Iterator quad;

   double weight;
   double ves;
   double maxVes;
   double porosity;

   NumericFunctions::Quadrature3D::getInstance ().get ( numberOfQuadraturePoints, quad );
   finiteElement.setGeometry ( geometryMatrix );

   interpolateCoefficients ( element, Basin_Modelling::VES_FP,  vesCoeffs,    lambda );
   interpolateCoefficients ( element, Basin_Modelling::Max_VES, maxVesCoeffs, lambda );

   elementVolume = 0.0;
   elementPoreVolume = 0.0;

   for ( quad.initialise (); not quad.isDone (); ++quad ) {
      finiteElement.setQuadraturePoint ( quad.getX (), quad.getY (), quad.getZ ());
      weight = determinant ( finiteElement.getJacobian ()) * quad.getWeight ();
      ves = FiniteElementMethod::innerProduct ( finiteElement.getBasis (), vesCoeffs );
      maxVes = FiniteElementMethod::innerProduct ( finiteElement.getBasis (), maxVesCoeffs );

      porosity = lithology->porosity ( ves, maxVes, false, 0.0 );

      elementVolume += weight;
      elementPoreVolume += porosity * weight;
   }

}


//------------------------------------------------------------//

FiniteElementMethod::ThreeVector elementBoundaryNormal ( const LayerElement&                   element,
                                                         const FiniteElementMethod::Matrix3x3& jacobian,
                                                         const VolumeData::BoundaryId          whichBoundary,
                                                         const bool                            normaliseVector ) {

#if 0
   static const int colum1Positions [ 6 ] = { 2, 1, 2, 3, 3, 1 };
   static const int colum2Positions [ 6 ] = { 1, 3, 3, 1, 2, 2 };
#endif

   ThreeVector normal;
   int col1 = 0;
   int col2 = 0;

#if 0
   col1 = colum1Positions [ whichBoundary ];
   col2 = colum2Positions [ whichBoundary ];
#endif

   bool error = false;

   switch ( whichBoundary ) {

     case VolumeData::GAMMA_1 : col1 = 2;
                                col2 = 1;
                                break;

     case VolumeData::GAMMA_2 : col1 = 1;
                                col2 = 3;
                                break;

     case VolumeData::GAMMA_3 : col1 = 2;
                                col2 = 3;
                                break;

     case VolumeData::GAMMA_4 : col1 = 3;
                                col2 = 1;
                                break;

     case VolumeData::GAMMA_5 : col1 = 3;
                                col2 = 2;
                                break;

     case VolumeData::GAMMA_6 : col1 = 1;
                                col2 = 2;
                                break;

   default :

      cout << " Error boundary not known "  << whichBoundary << endl;
      error = true;

   }

   if ( not error ) {
      // Normal = R1 x R2 -- vector cross product
      normal ( 1 ) = jacobian ( 2, col1 ) * jacobian ( 3, col2 ) - jacobian ( 3, col1 ) * jacobian ( 2, col2 );
      normal ( 2 ) = jacobian ( 3, col1 ) * jacobian ( 1, col2 ) - jacobian ( 1, col1 ) * jacobian ( 3, col2 );
      normal ( 3 ) = jacobian ( 1, col1 ) * jacobian ( 2, col2 ) - jacobian ( 2, col1 ) * jacobian ( 1, col2 );

      if ( normaliseVector ) {
         normalise ( normal );
      }

   } else {
      normal ( 1 ) = 0.0;
      normal ( 2 ) = 0.0;
      normal ( 3 ) = 0.0;
   }

   return normal;
}

//------------------------------------------------------------//

void getElementBoundaryNormal ( const LayerElement&                   element,
                                const FiniteElementMethod::Matrix3x3& jacobian,
                                const VolumeData::BoundaryId          whichBoundary,
                                FiniteElementMethod::ThreeVector&     normal,
                                double&                               dsDt ) {

   normal = elementBoundaryNormal ( element, jacobian, whichBoundary, false );
   dsDt = length ( normal );
   scale ( normal, 1.0 / dsDt );
}

//------------------------------------------------------------//

void getCentreOfElementFace ( const LayerElement&          element,
                              const VolumeData::BoundaryId whichBoundary,
                                    double&                x,
                                    double&                y,
                                    double&                z ) {

   switch ( whichBoundary ) {

     case VolumeData::GAMMA_1 : x =  0.0;
                                y =  0.0;
                                z = -1.0;
                                break;

     case VolumeData::GAMMA_2 : x =  0.0;
                                y = -1.0;
                                z =  0.0;
                                break;

     case VolumeData::GAMMA_3 : x = 1.0;
                                y = 0.0;
                                z = 0.0;
                                break;

     case VolumeData::GAMMA_4 : x = 0.0;
                                y = 1.0;
                                z = 0.0;
                                break;

     case VolumeData::GAMMA_5 : x = -1.0;
                                y =  0.0;
                                z =  0.0;
                                break;

     case VolumeData::GAMMA_6 : x = 0.0;
                                y = 0.0;
                                z = 1.0;
                                break;

   default :

      cout << " Error boundary not known: "  << whichBoundary << endl;

   }

}

//------------------------------------------------------------//

FiniteElementMethod::ThreeVector computeMassFlux ( const LayerElement&  element,
                                                   const FiniteElement& finiteElement,
                                                   const ElementVector& phasePressure,
                                                   const Matrix3x3&     permeability,
                                                   const double         fluidDensity,
                                                   const double         fluidViscosity,
                                                   const bool           limitGradPressure,
                                                   const double         gradPressureMaximum ) {

   ThreeVector flux;

   // Compute the pressure-gradient.
   flux = finiteElement.interpolateGrad ( phasePressure );

   // if ( limitGradPressure ) {
   //    // Limit the pressure gradient.
   //    flux ( 1 ) = NumericFunctions::clipValueToRange<double> ( flux ( 1 ), -gradPressureMaximum, gradPressureMaximum );
   //    flux ( 2 ) = NumericFunctions::clipValueToRange<double> ( flux ( 2 ), -gradPressureMaximum, gradPressureMaximum );
   //    flux ( 3 ) = NumericFunctions::clipValueToRange<double> ( flux ( 3 ), -gradPressureMaximum, gradPressureMaximum );
   // }

   // Subtract the "buoyancy term"
   flux ( 3 ) -= fluidDensity * GRAVITY;

   // Now scale by the permeability tensor.
   flux = matrixVectorProduct ( permeability, flux );
   flux *= -1.0 / fluidViscosity;

   return flux;
}

//------------------------------------------------------------//

FiniteElementMethod::ThreeVector computeFlowVelocity ( const LayerElement&  element,
                                                       const FiniteElement& finiteElement,
                                                       const ElementVector& brineOverpressure,
                                                       const ElementVector& brineHydrostaticPressure,
                                                       const ElementVector& capillaryPressure,
                                                       const Matrix3x3&     permeability,
                                                       const double         brineDensity,
                                                       const double         hcDensity,
                                                       const double         hcViscosity,
                                                       const bool           limitGradPressure,
                                                       const double         gradPressureMaximum,
                                                       const bool           print ) {



   ThreeVector velocity;
   ThreeVector gradOverpressure;
   ThreeVector gradHydrostaticPressure;

   ThreeVector gradPorePressure;

   ElementVector phasePressure;

   add ( brineOverpressure, brineHydrostaticPressure, phasePressure );
   Increment ( capillaryPressure, phasePressure );

   // Compute the pressure-gradient.
   gradOverpressure = finiteElement.interpolateGrad ( capillaryPressure );
   velocity ( 1 ) = gradOverpressure ( 1 );
   velocity ( 2 ) = gradOverpressure ( 2 );
   velocity ( 3 ) = gradOverpressure ( 3 );

   if ( limitGradPressure ) {
      // Limit the pressure gradient.
      velocity ( 1 ) = NumericFunctions::clipValueToRange<double> ( velocity ( 1 ), -gradPressureMaximum, gradPressureMaximum );
      velocity ( 2 ) = NumericFunctions::clipValueToRange<double> ( velocity ( 2 ), -gradPressureMaximum, gradPressureMaximum );
      velocity ( 3 ) = NumericFunctions::clipValueToRange<double> ( velocity ( 3 ), -gradPressureMaximum, gradPressureMaximum );
   }

   // Now scale by the permeability tensor.
   velocity = matrixVectorProduct ( permeability, velocity );
   velocity *= -1.0 / hcViscosity;

   return velocity;
}
//------------------------------------------------------------//

double centreOfElement ( const LayerElement& element ) {

   ElementGeometryMatrix geometryMatrix;

   getGeometryMatrix ( element, geometryMatrix );
   return centreOfElement ( geometryMatrix );
}

//------------------------------------------------------------//

double centreOfElement ( const FiniteElementMethod::ElementGeometryMatrix& geometryMatrix ) {

   double centre = 0.0;
   int i;

   for ( i = 1; i <= 8; ++i ) {
      centre += geometryMatrix ( 3, i );
   }

   return 0.125 * centre;
}


//------------------------------------------------------------//
