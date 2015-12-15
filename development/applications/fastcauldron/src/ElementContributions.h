#ifndef _FASTCAULDRON__ELEMENT_CONTRIBUTIONS__H_
#define _FASTCAULDRON__ELEMENT_CONTRIBUTIONS__H_

#include "FiniteElement.h"
#include "FiniteElementTypes.h"

#include "LayerElement.h"
#include "property_manager.h"

#include "BoundaryId.h"

#include "globaldefs.h"


/// \brief Extract the coefficients for the property.
///
/// Coefficients are extracted from the current-properties vector.
void getCoefficients ( const LayerElement&                       element,
                       const PETSC_3D_Array&                     property,
                             FiniteElementMethod::ElementVector& coefficients );

/// \brief Extract the coefficients for the property.
///
/// Coefficients are extracted from the current-properties vector.
void getCoefficients ( const LayerElement&                         element,
                       const Basin_Modelling::Fundamental_Property property,
                             FiniteElementMethod::ElementVector&   coefficients );

/// \brief Extract the coefficients for the property.
///
/// Coefficients are extracted from the previous-properties vector.
void getPreviousCoefficients ( const LayerElement&                         element,
                               const Basin_Modelling::Fundamental_Property property,
                                     FiniteElementMethod::ElementVector&   coefficients );

void interpolateCoefficients ( const LayerElement&                         element,
                               const Basin_Modelling::Fundamental_Property property,
                                     FiniteElementMethod::ElementVector&   coefficients,
                               const double                                lambda );

/// \brief Construct the geometry-matrix from the volume-elememt.
void getGeometryMatrix ( const LayerElement&                         element,
                         FiniteElementMethod::ElementGeometryMatrix& geometryMatrix );

/// \brief Construct the geometry-matrix from the volume-elememt.
void getGeometryMatrix ( const LayerElement&                         element,
                         FiniteElementMethod::ElementGeometryMatrix& geometryMatrix,
                         const double                                lambda );

/// \brief Assign the geometry-matrix to the finite-element object.
void fillGeometryMatrix ( const LayerElement&                       element,
                                FiniteElementMethod::FiniteElement& finiteElement );

/// \brief Evalaute one of the "fundamental properties" defined in the element.
///
/// The default is to evluate the property at the centre of the element.
double computeProperty ( const LayerElement&                         element,
                         const Basin_Modelling::Fundamental_Property property,
                         const double                                x = 0.0,
                         const double                                y = 0.0,
                         const double                                z = 0.0 );

/// \brief Evalaute one of the "fundamental properties" defined in the element from the previous time-step.
///
/// The default is to evluate the property at the centre of the element.
double computePreviousProperty ( const LayerElement&                         element,
                                 const Basin_Modelling::Fundamental_Property property,
                                 const double                                x = 0.0,
                                 const double                                y = 0.0,
                                 const double                                z = 0.0 );

/// \brief Evalaute one of the "fundamental properties" defined in the element.
///
/// The default is to evluate the property at the centre of the element.
/// Interpolate the property at some point between the last and current time-step.
double interpolateProperty ( const LayerElement&                         element,
                             const Basin_Modelling::Fundamental_Property property,
                             const double                                lambda,
                             const double                                x = 0.0,
                             const double                                y = 0.0,
                             const double                                z = 0.0 );


/// \brief Evalaute one of the "fundamental properties" defined in the element.
///
/// The default is to evluate the property at the centre of the element.
// A temporary function during development.
FiniteElementMethod::ThreeVector computeGradientProperty ( const LayerElement&                         element,
                                                           const FiniteElementMethod::Matrix3x3&       jacobian,
                                                           const Basin_Modelling::Fundamental_Property property,
                                                           const double                                x = 0.0,
                                                           const double                                y = 0.0,
                                                           const double                                z = 0.0 );

/// \brief Compute the volume of the element.
double volumeOfElement ( const LayerElement&                               element,
                         const FiniteElementMethod::ElementGeometryMatrix& geometryMatrix,
                         const int                                         numberOfQuadraturePoints = DefaultQuadratureDegree );

/// \brief Compute the volume of the element.
double volumeOfElement ( const LayerElement&  element,
                         const int            numberOfQuadraturePoints = DefaultQuadratureDegree );

/// \brief Compute the volume of the pore space of the element.
double poreVolumeOfElement ( const LayerElement&                               element,
                             const FiniteElementMethod::ElementGeometryMatrix& geometryMatrix,
                             const int                                         numberOfQuadraturePoints = DefaultQuadratureDegree );

/// \brief Compute the volume of the pore space of the element.
double poreVolumeOfElement ( const LayerElement&  element,
                             const int            numberOfQuadraturePoints = DefaultQuadratureDegree );

/// \brief Compute the average porosity for an element.
///
/// \f$ \frac{\int_{e} \phi dx}{\int_{e} dx} \f$
double averageElementPorosity ( const LayerElement&                               element,
                                const FiniteElementMethod::ElementGeometryMatrix& geometryMatrix,
                                const int                                         numberOfQuadraturePoints = DefaultQuadratureDegree );

/// \brief Compute the average porosity for an element.
///
/// \f$ \frac{\int_{e} \phi dx}{\int_{e} dx} \f$
double averageElementPorosity ( const LayerElement&                               element,
                                const int                                         numberOfQuadraturePoints = DefaultQuadratureDegree );

/// \brief Calculate both the element volume and element pore volume.
///
/// This is used by both the element-pore-volume and the average-element-porosity functions.
/// Combining the calculation of both values into a single function removes the need to
/// compute a colume integral twice.
void elementVolumeCalculations ( const LayerElement&                               element,
                                 const FiniteElementMethod::ElementGeometryMatrix& geometryMatrix,
                                       double&                                     elementVolume,
                                       double&                                     elementPoreVolume,
                                 const int                                         numberOfQuadraturePoints = DefaultQuadratureDegree );
                                 
void elementVolumeCalculations ( const LayerElement&                               element,
                                       double&                                     elementVolume,
                                       double&                                     elementPoreVolume,
                                 const int                                         numberOfQuadraturePoints = DefaultQuadratureDegree );

void elementVolumeCalculations ( const LayerElement&                               element,
                                 const FiniteElementMethod::ElementGeometryMatrix& geometryMatrix,
                                       double&                                     elementVolume,
                                       double&                                     elementPoreVolume,
                                 const double                                      lambda,
                                 const int                                         numberOfQuadraturePoints = DefaultQuadratureDegree );
                                 
                                 
/// Get the outward pointing normal to the given face.
///
/// Element is orientated in usual fem order.
FiniteElementMethod::ThreeVector elementBoundaryNormal ( const LayerElement&                   element,
                                                         const FiniteElementMethod::Matrix3x3& jacobian,
                                                         const VolumeData::BoundaryId          whichBoundary,
                                                         const bool                            normaliseVector = true );

/// Get the outward pointing normal to the given face.
///
/// Element is orientated in usual fem order.
void getElementBoundaryNormal ( const LayerElement&                   element,
                                const FiniteElementMethod::Matrix3x3& jacobian,
                                const VolumeData::BoundaryId          whichBoundary,
                                FiniteElementMethod::ThreeVector&     normal,
                                double&                               dsDt );

/// \brief Get the coordinate at the centre of the face.
///
/// In reference coordinates.
void getCentreOfElementFace ( const LayerElement&          element,
                              const VolumeData::BoundaryId whichBoundary,
                                    double&                x,
                                    double&                y,
                                    double&                z );

FiniteElementMethod::ThreeVector computeMassFlux ( const LayerElement&                       element,
                                                   const FiniteElementMethod::FiniteElement& finiteElement,
                                                   const FiniteElementMethod::ElementVector& phasePressure,
                                                   const FiniteElementMethod::Matrix3x3&     permeability,
                                                   const double                              fluidDensity,
                                                   const double                              fluidViscosity,
                                                   const bool                                limitGradPressure = false,
                                                   const double                              gradPressureMaximum = DefaultMaximumGradPressure );

// brineOverpressure, brineHydrostaticPressure and capillaryPressure must be in Pascals.
FiniteElementMethod::ThreeVector computeFlowVelocity ( const LayerElement&  element,
                                                       const FiniteElement& finiteElement,
                                                       const ElementVector& brineOverpressure,
                                                       const ElementVector& brineHydrostaticPressure,
                                                       const ElementVector& capillaryPressure,
                                                       const Matrix3x3&     permeability,
                                                       const double         brineDensity,
                                                       const double         hcDensity,
                                                       const double         hcViscosity,
                                                       const bool           limitGradPressure = false,
                                                       const double         gradPressureMaximum = DefaultMaximumGradPressure,
                                                       const bool           print = false );


double centreOfElement ( const LayerElement& element );

double centreOfElement ( const FiniteElementMethod::ElementGeometryMatrix& geometryMatrix );

#endif // _FASTCAULDRON__ELEMENT_CONTRIBUTIONS__H_
