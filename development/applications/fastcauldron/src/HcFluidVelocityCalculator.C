//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

#include "HcFluidVelocityCalculator.h"
#include "DerivedOutputPropertyMap.h"
#include "PropertyManager.h"
#include "FastcauldronSimulator.h"
#include "element_contributions.h"
#include "FiniteElement.h"
#include "FiniteElementTypes.h"
#include "PetscBlockVector.h"
#include "PVTCalculator.h"
#include "ElementContributions.h"
#include "Lithology.h"

#include "Interface/RunParameters.h"


// utilities library
#include "ConstantsNumerical.h"
using Utilities::Numerical::CauldronNoDataValue;
#include "ConstantsMathematics.h"
using Utilities::Maths::YearToSecond;

using namespace FiniteElementMethod;

OutputPropertyMap* allocateHcFluidVelocityVolumeCalculator ( const PropertyList property, LayerProps* formation, const Interface::Snapshot* snapshot ) {
   return new DerivedOutputPropertyMap<HcFluidVelocityVolumeCalculator>( property, formation, snapshot );
}



HcFluidVelocityVolumeCalculator::HcFluidVelocityVolumeCalculator ( LayerProps* formation, const Interface::Snapshot* snapshot ) :
   m_formation ( formation ), m_snapshot ( snapshot ) {

   m_pvtProperties = 0;
   m_isCalculated = false;
}

bool HcFluidVelocityVolumeCalculator::operator ()( const OutputPropertyMap::OutputPropertyList& properties, 
                                                   OutputPropertyMap::PropertyValueList&  propertyValues ) {

   using namespace Basin_Modelling;

   if ( m_isCalculated ) {
      return true;
   }

   const bool IncludeGhosts = true;

   FiniteElementMethod::FiniteElement fe;
   FiniteElementMethod::ElementVector porePressureCoeffs;
   FiniteElementMethod::ElementVector temperatureCoeffs;
   FiniteElementMethod::ElementVector vesCoeffs;
   FiniteElementMethod::ElementVector maxVesCoeffs;
   FiniteElementMethod::Matrix3x3     permeability;

   PVTPhaseComponents           phaseComposition;
   PVTComponents                composition;
   PVTComponents                molarMasses;

   GeoPhysics::CompoundProperty porosity;
   FiniteElementMethod::ThreeVector hcVapourVelocity;
   FiniteElementMethod::ThreeVector hcLiquidVelocity;
   Saturation                       saturation;

   PVTPhaseValues hcViscosity;
   PVTPhaseValues hcDensity;

   const Lithology* lithology;
   double porePressure;
   double temperature;
   double ves;
   double maxVes;
   double permNormal;
   double permPlane;
   double vapourVelocityMagnitude;
   double liquidVelocityMagnitude;
   unsigned int elementCount;
   unsigned int i;
   unsigned int j;
   unsigned int k;

   const LayerElementArray& elements = m_formation->getLayerElements ();
   const ElementVolumeGrid& grid = m_formation->getVolumeGrid ( Saturation::NumberOfPhases );

   bool porePressureIsActive = m_formation->Current_Properties.propertyIsActivated ( Basin_Modelling::Pore_Pressure );
   bool temperatureIsActive = m_formation->Current_Properties.propertyIsActivated ( Basin_Modelling::Temperature );
   bool vesIsActive = m_formation->Current_Properties.propertyIsActivated ( Basin_Modelling::VES_FP );
   bool maxVesIsActive = m_formation->Current_Properties.propertyIsActivated ( Basin_Modelling::Max_VES );
   bool depthIsActive = m_formation->Current_Properties.propertyIsActivated ( Basin_Modelling::Depth );

   if ( m_pvtProperties == 0 ) {
      return false;
   }

   if ( not m_pvtProperties->isCalculated ()) {

      if ( not m_pvtProperties->calculate ()) {
         return false;
      }

   }

   if ( not porePressureIsActive ) {
      m_formation->Current_Properties.Activate_Property ( Basin_Modelling::Pore_Pressure, INSERT_VALUES, true );
   }

   if ( not temperatureIsActive ) {
      m_formation->Current_Properties.Activate_Property ( Basin_Modelling::Temperature, INSERT_VALUES, true );
   }

   if ( not vesIsActive ) {
      m_formation->Current_Properties.Activate_Property ( Basin_Modelling::VES_FP, INSERT_VALUES, true );
   }

   if ( not maxVesIsActive ) {
      m_formation->Current_Properties.Activate_Property ( Basin_Modelling::Max_VES, INSERT_VALUES, true );
   }

   if ( not depthIsActive ) {
      m_formation->Current_Properties.Activate_Property ( Basin_Modelling::Depth, INSERT_VALUES, true );
   }

   Interface::GridMap* hcVapourMaps [ 4 ];
   Interface::GridMap* hcLiquidMaps [ 4 ];

   PetscBlockVector<Saturation> saturations;
   saturations.setVector ( m_formation->getVolumeGrid ( Saturation::NumberOfPhases ), m_formation->getPhaseSaturationVec (), INSERT_VALUES );

   PetscBlockVector<PVTComponents> components;
   components.setVector ( m_formation->getVolumeGrid ( NumberOfPVTComponents ), m_formation->getPreviousComponentVec (), INSERT_VALUES );

   // Retrieve the heat-flow result maps.
   hcVapourMaps [ 0 ] = propertyValues [ 0 ]->getGridMap ();
   hcVapourMaps [ 0 ]->retrieveData ();

   hcVapourMaps [ 1 ] = propertyValues [ 1 ]->getGridMap ();
   hcVapourMaps [ 1 ]->retrieveData ();

   hcVapourMaps [ 2 ] = propertyValues [ 2 ]->getGridMap ();
   hcVapourMaps [ 2 ]->retrieveData ();

   hcVapourMaps [ 3 ] = propertyValues [ 3 ]->getGridMap ();
   hcVapourMaps [ 3 ]->retrieveData ();

   hcLiquidMaps [ 0 ] = propertyValues [ 4 ]->getGridMap ();
   hcLiquidMaps [ 0 ]->retrieveData ();

   hcLiquidMaps [ 1 ] = propertyValues [ 5 ]->getGridMap ();
   hcLiquidMaps [ 1 ]->retrieveData ();

   hcLiquidMaps [ 2 ] = propertyValues [ 6 ]->getGridMap ();
   hcLiquidMaps [ 2 ]->retrieveData ();

   hcLiquidMaps [ 3 ] = propertyValues [ 7 ]->getGridMap ();
   hcLiquidMaps [ 3 ]->retrieveData ();

   molarMasses = PVTCalc::getInstance ().getMolarMass ();
   molarMasses *= 1.0e-3;

   for ( i = grid.firstI (); i <= grid.lastI (); ++i ) {

      for ( j = grid.firstJ (); j <= grid.lastJ (); ++j ) {

         if ( FastcauldronSimulator::getInstance ().getMapElement ( i, j ).isValid ()) {
            lithology = dynamic_cast<const Lithology*>(m_formation->getCompoundLithology ( i, j ));

            for ( k = grid.firstK (); k <= grid.lastK (); ++k ) {

               const LayerElement& element = elements ( i, j, k );

               if ( element.isActive () and ( saturations ( k, j, i )( Saturation::VAPOUR ) > 0.0 or saturations ( k, j, i )( Saturation::LIQUID ) > 0.0 )) {
                  getCoefficients ( element, Basin_Modelling::Pore_Pressure, porePressureCoeffs );
                  
#if 0
                  getCoefficients ( element, Basin_Modelling::Temperature, temperatureCoeffs );
#endif

                  getCoefficients ( element, Basin_Modelling::VES_FP, vesCoeffs );
                  getCoefficients ( element, Basin_Modelling::Max_VES, maxVesCoeffs );
                  fillGeometryMatrix ( element, fe );

                  fe.setQuadraturePoint ( 0.0, 0.0, 0.0 );

                  ves = fe.interpolate ( vesCoeffs );
                  maxVes = fe.interpolate ( maxVesCoeffs );

#if 0
                  composition = components ( k, j, i );
                  composition *= molarMasses;
#endif

                  saturation = saturations ( k, j, i );

#if 0
                  // Convert temperature to Kelvin.
                  temperature = fe.interpolate ( temperatureCoeffs ) + 273.15;
#endif

                  // Convert pressure to Pa.
                  porePressure = 1.0e6 * fe.interpolate ( porePressureCoeffs );
                  porePressure = NumericFunctions::Maximum ( 1.0e5, porePressure );

#if 0
                  // Flash concentrations.
                  pvtFlash::EosPack::getInstance ().computeWithLumping ( temperature, porePressure,
                                                              composition.m_components,
                                                              phaseComposition.m_masses,
                                                              hcDensity.m_values,
                                                              hcViscosity.m_values );

                  // Convert to correct units.
                  hcViscosity *= 0.001;
#else
                  hcDensity ( PhaseId::VAPOUR ) = m_pvtProperties->getVolumeValue ( i, j, k, 0 );
                  hcDensity ( PhaseId::LIQUID ) = m_pvtProperties->getVolumeValue ( i, j, k, 1 );

                  hcViscosity ( PhaseId::VAPOUR ) = m_pvtProperties->getVolumeValue ( i, j, k, 2 );
                  hcViscosity ( PhaseId::LIQUID ) = m_pvtProperties->getVolumeValue ( i, j, k, 3 );
#endif

                  lithology->getPorosity ( ves, maxVes, false, 0.0, porosity );
                  lithology->calcBulkPermeabilityNP ( ves, maxVes, porosity, permNormal, permPlane );

                  fe.setTensor ( permNormal, permPlane, permeability );

                  if ( saturation ( Saturation::VAPOUR ) > 0.0 ) {
                     hcVapourVelocity = computeMassFlux ( element, fe, porePressureCoeffs, permeability, hcDensity ( PhaseId::VAPOUR ), hcViscosity ( PhaseId::VAPOUR ));
                     hcVapourVelocity *= lithology->relativePermeability ( Saturation::VAPOUR, saturation ) / ( porosity.mixedProperty () * saturation ( Saturation::VAPOUR ));

                     // Scale to mm/year from metres/sec.
                     hcVapourVelocity *= 1000.0 * YearToSecond;

                     vapourVelocityMagnitude = length ( hcVapourVelocity );
                  } else {
                     hcVapourVelocity.fill ( CauldronNoDataValue );
                     vapourVelocityMagnitude = CauldronNoDataValue;
                  }

                  if ( saturation ( Saturation::LIQUID ) > 0.0 ) {
                     hcLiquidVelocity = computeMassFlux ( element, fe, porePressureCoeffs, permeability, hcDensity ( PhaseId::LIQUID ), hcViscosity ( PhaseId::LIQUID ));
                     hcLiquidVelocity *= lithology->relativePermeability ( Saturation::LIQUID, saturation ) / ( porosity.mixedProperty () * saturation ( Saturation::LIQUID ));

                     // Scale to mm/year from metres/sec.
                     hcLiquidVelocity *= 1000.0 * YearToSecond;

                     liquidVelocityMagnitude = length ( hcLiquidVelocity );
                  } else {
                     hcLiquidVelocity.fill ( CauldronNoDataValue );
                     liquidVelocityMagnitude = CauldronNoDataValue;
                  }

                  hcVapourMaps [ 0 ]->setValue ( i, j, k, hcVapourVelocity ( 1 ));
                  hcVapourMaps [ 1 ]->setValue ( i, j, k, hcVapourVelocity ( 2 ));
                  hcVapourMaps [ 2 ]->setValue ( i, j, k, hcVapourVelocity ( 3 ));
                  hcVapourMaps [ 3 ]->setValue ( i, j, k, vapourVelocityMagnitude );

                  // Fill other heat flow nodes if current (i,j) position is at end of array
                  if ( i == grid.getNumberOfXElements () - 1 ) {
                     hcVapourMaps [ 0 ]->setValue ( i + 1, j, k, hcVapourVelocity ( 1 ));
                     hcVapourMaps [ 1 ]->setValue ( i + 1, j, k, hcVapourVelocity ( 2 ));
                     hcVapourMaps [ 2 ]->setValue ( i + 1, j, k, hcVapourVelocity ( 3 ));
                     hcVapourMaps [ 3 ]->setValue ( i + 1, j, k, vapourVelocityMagnitude );
                  }

                  if ( j == grid.getNumberOfYElements () - 1 ) {
                     hcVapourMaps [ 0 ]->setValue ( i, j + 1, k, hcVapourVelocity ( 1 ));
                     hcVapourMaps [ 1 ]->setValue ( i, j + 1, k, hcVapourVelocity ( 2 ));
                     hcVapourMaps [ 2 ]->setValue ( i, j + 1, k, hcVapourVelocity ( 3 ));
                     hcVapourMaps [ 3 ]->setValue ( i, j + 1, k, vapourVelocityMagnitude );
                  }

                  if ( i == grid.getNumberOfXElements () - 1  and j == grid.getNumberOfYElements () - 1 ) {
                     hcVapourMaps [ 0 ]->setValue ( i + 1, j + 1, k, hcVapourVelocity ( 1 ));
                     hcVapourMaps [ 1 ]->setValue ( i + 1, j + 1, k, hcVapourVelocity ( 2 ));
                     hcVapourMaps [ 2 ]->setValue ( i + 1, j + 1, k, hcVapourVelocity ( 3 ));
                     hcVapourMaps [ 3 ]->setValue ( i + 1, j + 1, k, vapourVelocityMagnitude );
                  }

                  // At top of layer.
                  if ( k == grid.getNumberOfZElements () - 1 ) {
                     hcVapourMaps [ 0 ]->setValue ( i, j, k + 1, hcVapourVelocity ( 1 ));
                     hcVapourMaps [ 1 ]->setValue ( i, j, k + 1, hcVapourVelocity ( 2 ));
                     hcVapourMaps [ 2 ]->setValue ( i, j, k + 1, hcVapourVelocity ( 3 ));
                     hcVapourMaps [ 3 ]->setValue ( i, j, k + 1, vapourVelocityMagnitude );

                     // Fill other heat flow nodes if current (i,j) position is at end of array if ( grid.getNumberOfXElements () - 1 ) {
                     if ( i == grid.getNumberOfXElements () - 1 ) {
                        hcVapourMaps [ 0 ]->setValue ( i + 1, j, k + 1, hcVapourVelocity ( 1 ));
                        hcVapourMaps [ 1 ]->setValue ( i + 1, j, k + 1, hcVapourVelocity ( 2 ));
                        hcVapourMaps [ 2 ]->setValue ( i + 1, j, k + 1, hcVapourVelocity ( 3 ));
                        hcVapourMaps [ 3 ]->setValue ( i + 1, j, k + 1, vapourVelocityMagnitude );
                     }

                     if (j ==  grid.getNumberOfYElements () - 1 ) {
                        hcVapourMaps [ 0 ]->setValue ( i, j + 1, k + 1, hcVapourVelocity ( 1 ));
                        hcVapourMaps [ 1 ]->setValue ( i, j + 1, k + 1, hcVapourVelocity ( 2 ));
                        hcVapourMaps [ 2 ]->setValue ( i, j + 1, k + 1, hcVapourVelocity ( 3 ));
                        hcVapourMaps [ 3 ]->setValue ( i, j + 1, k + 1, vapourVelocityMagnitude );
                     }

                     if ( i == grid.getNumberOfXElements () - 1  && j == grid.getNumberOfYElements () - 1 ) {
                        hcVapourMaps [ 0 ]->setValue ( i + 1, j + 1, k + 1, hcVapourVelocity ( 1 ));
                        hcVapourMaps [ 1 ]->setValue ( i + 1, j + 1, k + 1, hcVapourVelocity ( 2 ));
                        hcVapourMaps [ 2 ]->setValue ( i + 1, j + 1, k + 1, hcVapourVelocity ( 3 ));
                        hcVapourMaps [ 3 ]->setValue ( i + 1, j + 1, k + 1, vapourVelocityMagnitude );
                     }

                  }

                  hcLiquidMaps [ 0 ]->setValue ( i, j, k, hcLiquidVelocity ( 1 ));
                  hcLiquidMaps [ 1 ]->setValue ( i, j, k, hcLiquidVelocity ( 2 ));
                  hcLiquidMaps [ 2 ]->setValue ( i, j, k, hcLiquidVelocity ( 3 ));
                  hcLiquidMaps [ 3 ]->setValue ( i, j, k, liquidVelocityMagnitude );

                  // Fill other heat flow nodes if current (i,j) position is at end of array
                  if ( i == grid.getNumberOfXElements () - 1 ) {
                     hcLiquidMaps [ 0 ]->setValue ( i + 1, j, k, hcLiquidVelocity ( 1 ));
                     hcLiquidMaps [ 1 ]->setValue ( i + 1, j, k, hcLiquidVelocity ( 2 ));
                     hcLiquidMaps [ 2 ]->setValue ( i + 1, j, k, hcLiquidVelocity ( 3 ));
                     hcLiquidMaps [ 3 ]->setValue ( i + 1, j, k, liquidVelocityMagnitude );
                  }

                  if ( j == grid.getNumberOfYElements () - 1 ) {
                     hcLiquidMaps [ 0 ]->setValue ( i, j + 1, k, hcLiquidVelocity ( 1 ));
                     hcLiquidMaps [ 1 ]->setValue ( i, j + 1, k, hcLiquidVelocity ( 2 ));
                     hcLiquidMaps [ 2 ]->setValue ( i, j + 1, k, hcLiquidVelocity ( 3 ));
                     hcLiquidMaps [ 3 ]->setValue ( i, j + 1, k, liquidVelocityMagnitude );
                  }

                  if ( i == grid.getNumberOfXElements () - 1  and j == grid.getNumberOfYElements () - 1 ) {
                     hcLiquidMaps [ 0 ]->setValue ( i + 1, j + 1, k, hcLiquidVelocity ( 1 ));
                     hcLiquidMaps [ 1 ]->setValue ( i + 1, j + 1, k, hcLiquidVelocity ( 2 ));
                     hcLiquidMaps [ 2 ]->setValue ( i + 1, j + 1, k, hcLiquidVelocity ( 3 ));
                     hcLiquidMaps [ 3 ]->setValue ( i + 1, j + 1, k, liquidVelocityMagnitude );
                  }

                  // At top of layer.
                  if ( k == grid.getNumberOfZElements () - 1 ) {
                     hcLiquidMaps [ 0 ]->setValue ( i, j, k + 1, hcLiquidVelocity ( 1 ));
                     hcLiquidMaps [ 1 ]->setValue ( i, j, k + 1, hcLiquidVelocity ( 2 ));
                     hcLiquidMaps [ 2 ]->setValue ( i, j, k + 1, hcLiquidVelocity ( 3 ));
                     hcLiquidMaps [ 3 ]->setValue ( i, j, k + 1, liquidVelocityMagnitude );

                     // Fill other heat flow nodes if current (i,j) position is at end of array if ( grid.getNumberOfXElements () - 1 ) {
                     if ( i == grid.getNumberOfXElements () - 1 ) {
                        hcLiquidMaps [ 0 ]->setValue ( i + 1, j, k + 1, hcLiquidVelocity ( 1 ));
                        hcLiquidMaps [ 1 ]->setValue ( i + 1, j, k + 1, hcLiquidVelocity ( 2 ));
                        hcLiquidMaps [ 2 ]->setValue ( i + 1, j, k + 1, hcLiquidVelocity ( 3 ));
                        hcLiquidMaps [ 3 ]->setValue ( i + 1, j, k + 1, liquidVelocityMagnitude );
                     }

                     if ( j == grid.getNumberOfYElements () - 1 ) {
                        hcLiquidMaps [ 0 ]->setValue ( i, j + 1, k + 1, hcLiquidVelocity ( 1 ));
                        hcLiquidMaps [ 1 ]->setValue ( i, j + 1, k + 1, hcLiquidVelocity ( 2 ));
                        hcLiquidMaps [ 2 ]->setValue ( i, j + 1, k + 1, hcLiquidVelocity ( 3 ));
                        hcLiquidMaps [ 3 ]->setValue ( i, j + 1, k + 1, liquidVelocityMagnitude );
                     }

                     if ( i == grid.getNumberOfXElements () - 1  and j == grid.getNumberOfYElements () - 1 ) {
                        hcLiquidMaps [ 0 ]->setValue ( i + 1, j + 1, k + 1, hcLiquidVelocity ( 1 ));
                        hcLiquidMaps [ 1 ]->setValue ( i + 1, j + 1, k + 1, hcLiquidVelocity ( 2 ));
                        hcLiquidMaps [ 2 ]->setValue ( i + 1, j + 1, k + 1, hcLiquidVelocity ( 3 ));
                        hcLiquidMaps [ 3 ]->setValue ( i + 1, j + 1, k + 1, liquidVelocityMagnitude );
                     }

                  }

               } else {
                  hcVapourMaps [ 0 ]->setValue ( i, j, k, CauldronNoDataValue );
                  hcVapourMaps [ 1 ]->setValue ( i, j, k, CauldronNoDataValue );
                  hcVapourMaps [ 2 ]->setValue ( i, j, k, CauldronNoDataValue );
                  hcVapourMaps [ 3 ]->setValue ( i, j, k, CauldronNoDataValue );

                  hcLiquidMaps [ 0 ]->setValue ( i, j, k, CauldronNoDataValue );
                  hcLiquidMaps [ 1 ]->setValue ( i, j, k, CauldronNoDataValue );
                  hcLiquidMaps [ 2 ]->setValue ( i, j, k, CauldronNoDataValue );
                  hcLiquidMaps [ 3 ]->setValue ( i, j, k, CauldronNoDataValue );
               }
               
            }

         } else {

            for ( k = elements.firstK (); k <= elements.lastK (); ++k ) {
               hcVapourMaps [ 0 ]->setValue ( i, j, k, CauldronNoDataValue );
               hcVapourMaps [ 1 ]->setValue ( i, j, k, CauldronNoDataValue );
               hcVapourMaps [ 2 ]->setValue ( i, j, k, CauldronNoDataValue );
               hcVapourMaps [ 3 ]->setValue ( i, j, k, CauldronNoDataValue );

               hcLiquidMaps [ 0 ]->setValue ( i, j, k, CauldronNoDataValue );
               hcLiquidMaps [ 1 ]->setValue ( i, j, k, CauldronNoDataValue );
               hcLiquidMaps [ 2 ]->setValue ( i, j, k, CauldronNoDataValue );
               hcLiquidMaps [ 3 ]->setValue ( i, j, k, CauldronNoDataValue );
            }
         }
          
      }

   }

   hcVapourMaps [ 0 ]->restoreData ();
   hcVapourMaps [ 1 ]->restoreData ();
   hcVapourMaps [ 2 ]->restoreData ();
   hcVapourMaps [ 3 ]->restoreData ();

   hcLiquidMaps [ 0 ]->restoreData ();
   hcLiquidMaps [ 1 ]->restoreData ();
   hcLiquidMaps [ 2 ]->restoreData ();
   hcLiquidMaps [ 3 ]->restoreData ();

   if ( not depthIsActive ) {
      m_formation->Current_Properties.Restore_Property ( Basin_Modelling::Depth );
   }

   if ( not maxVesIsActive ) {
      m_formation->Current_Properties.Restore_Property ( Basin_Modelling::Max_VES );
   }

   if ( not vesIsActive ) {
      m_formation->Current_Properties.Restore_Property ( Basin_Modelling::VES_FP );
   }

   if ( not temperatureIsActive ) {
      m_formation->Current_Properties.Restore_Property ( Basin_Modelling::Temperature );
   }

   if ( not porePressureIsActive ) {
      m_formation->Current_Properties.Restore_Property ( Basin_Modelling::Pore_Pressure );
   }


   m_isCalculated = true;
   return true;
}

void HcFluidVelocityVolumeCalculator::allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties ) {

   PropertyValue* property;
   
   property = (PropertyValue*)(FastcauldronSimulator::getInstance ().createVolumePropertyValue ( "HcVapourVelocityX", 
                                                                                                 m_snapshot, 0,
                                                                                                 m_formation,
                                                                                                 m_formation->getMaximumNumberOfElements () + 1 ));
   properties.push_back ( property );
   
   property = (PropertyValue*)(FastcauldronSimulator::getInstance ().createVolumePropertyValue ( "HcVapourVelocityY", 
                                                                                                 m_snapshot, 0,
                                                                                                 m_formation,
                                                                                                 m_formation->getMaximumNumberOfElements () + 1 ));
   properties.push_back ( property );
   
   property = (PropertyValue*)(FastcauldronSimulator::getInstance ().createVolumePropertyValue ( "HcVapourVelocityZ", 
                                                                                                 m_snapshot, 0,
                                                                                                 m_formation,
                                                                                                 m_formation->getMaximumNumberOfElements () + 1 ));
   properties.push_back ( property );
   
   property = (PropertyValue*)(FastcauldronSimulator::getInstance ().createVolumePropertyValue ( "HcVapourVelocityMagnitude", 
                                                                                                 m_snapshot, 0,
                                                                                                 m_formation,
                                                                                                 m_formation->getMaximumNumberOfElements () + 1 ));
   properties.push_back ( property );
   
   property = (PropertyValue*)(FastcauldronSimulator::getInstance ().createVolumePropertyValue ( "HcLiquidVelocityX", 
                                                                                                 m_snapshot, 0,
                                                                                                 m_formation,
                                                                                                 m_formation->getMaximumNumberOfElements () + 1 ));
   properties.push_back ( property );
   
   property = (PropertyValue*)(FastcauldronSimulator::getInstance ().createVolumePropertyValue ( "HcLiquidVelocityY", 
                                                                                                 m_snapshot, 0,
                                                                                                 m_formation,
                                                                                                 m_formation->getMaximumNumberOfElements () + 1 ));
   properties.push_back ( property );
   
   property = (PropertyValue*)(FastcauldronSimulator::getInstance ().createVolumePropertyValue ( "HcLiquidVelocityZ", 
                                                                                                 m_snapshot, 0,
                                                                                                 m_formation,
                                                                                                 m_formation->getMaximumNumberOfElements () + 1 ));
   properties.push_back ( property );
   
   property = (PropertyValue*)(FastcauldronSimulator::getInstance ().createVolumePropertyValue ( "HcLiquidVelocityMagnitude", 
                                                                                                 m_snapshot, 0,
                                                                                                 m_formation,
                                                                                                 m_formation->getMaximumNumberOfElements () + 1 ));
   properties.push_back ( property );


}

bool HcFluidVelocityVolumeCalculator::initialise ( OutputPropertyMap::PropertyValueList& propertyValues ) {

   m_pvtProperties = PropertyManager::getInstance().findOutputPropertyVolume ( "PVTProperties", m_formation, m_snapshot );

   return true;
}
