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

using namespace FiniteElementMethod;


// OutputPropertyMap* allocateHcFluidVelocityCalculator ( const PropertyList property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) {
//    return new DerivedOutputPropertyMap<HcFluidVelocityCalculator>( property, formation, surface, snapshot );
// }

OutputPropertyMap* allocateHcFluidVelocityVolumeCalculator ( const PropertyList property, LayerProps* formation, const Interface::Snapshot* snapshot ) {
   return new DerivedOutputPropertyMap<HcFluidVelocityVolumeCalculator>( property, formation, snapshot );
}

// HcFluidVelocityCalculator::HcFluidVelocityCalculator ( LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) :
//    m_formation ( formation ), m_surface ( surface ), m_snapshot ( snapshot ) {

//    m_depth = 0;
//    m_temperature = 0;
//    m_hydrostaticPressure = 0;
//    m_overpressure = 0;
//    m_ves = 0;
//    m_maxVes = 0;
//    m_chemicalCompaction = 0;
//    m_isCalculated = false;
//    m_fluid = 0;

//    m_chemicalCompactionRequired = m_formation->hasChemicalCompaction () and 
//                                   FastcauldronSimulator::getInstance ().getRunParameters ()->getChemicalCompaction ();


//    if ( m_formation->getTopSurface () == surface ) {
//       // Number-of-nodes = number-of-elements + 1.
//       // But C array indexing starts a 0, so 1 must be subtracted.
//       m_kIndex = (unsigned int)(m_formation->getMaximumNumberOfElements ()) - 1;

//       // The top of the element is the bottom of the reference element, since depth is positive.
//       m_zPosition = -1.0;
//    } else if ( m_formation->getBottomSurface () == surface ) {
//       m_kIndex = 0;

//       // The bottom of the element is the top of the reference element, since depth is positive.
//       m_zPosition = 1.0;
//    } else {
//       assert ( false );
//       // Error
//    }

// }

// bool HcFluidVelocityCalculator::operator ()( const OutputPropertyMap::OutputPropertyList& properties, 
//                                                  OutputPropertyMap::PropertyValueList&  propertyValues ) {

//    using namespace Basin_Modelling;

//    if ( m_isCalculated ) {
//       return true;
//    }

//    const bool IncludeGhosts = true;
//    const ElementList& elements = FastcauldronSimulator::getInstance ().getCauldron ()->mapElementList;

//    unsigned int elementCount;
//    unsigned int i;
//    unsigned int j;
//    int globalXNodes;
//    int globalYNodes;
//    int node;
//    Interface::GridMap* fluidVelocityMapX;
//    Interface::GridMap* fluidVelocityMapY;
//    Interface::GridMap* fluidVelocityMapZ;
//    CompoundLithology* lithology;


//    ThreeVector fluidVelocity;
//    ElementGeometryMatrix geometryMatrix;
//    ElementVector hydrostaticPressure;
//    ElementVector overpressure;
//    ElementVector ves;
//    ElementVector maxVes;
//    ElementVector temperature;
//    ElementVector chemCompaction;

//    DAGetInfo( *FastcauldronSimulator::getInstance ().getCauldron ()->mapDA, 
//               PETSC_NULL, &globalXNodes, &globalYNodes,
//               PETSC_NULL, PETSC_NULL, PETSC_NULL, PETSC_NULL, 
//               PETSC_NULL, PETSC_NULL, PETSC_NULL, PETSC_NULL );

//    if ( not m_depth->isCalculated ()) {

//       if ( not m_depth->calculate ()) {
//          return false;
//       }

//    }

//    if ( not m_temperature->isCalculated ()) {

//       if ( not m_temperature->calculate ()) {
//          return false;
//       }

//    }

//    if ( not m_hydrostaticPressure->isCalculated ()) {

//       if ( not m_hydrostaticPressure->calculate ()) {
//          return false;
//       }

//    }

//    if ( not m_overpressure->isCalculated ()) {

//       if ( not m_overpressure->calculate ()) {
//          return false;
//       }

//    }

//    if ( not m_ves->isCalculated ()) {

//       if ( not m_ves->calculate ()) {
//          return false;
//       }

//    }

//    if ( not m_maxVes->isCalculated ()) {

//       if ( not m_maxVes->calculate ()) {
//          return false;
//       } 

//    }

//    if ( m_chemicalCompactionRequired and not m_chemicalCompaction->isCalculated ()) {

//       if ( not m_chemicalCompaction->calculate ()) {
//          return false;
//       } 

//    }

//    // Get all the properties required  for the calculation of the heat-flow.
//    PETSC_3D_Array layerTemperature
//       ( m_formation->layerDA,
//         m_formation->Current_Properties ( Basin_Modelling::Temperature ),
//         INSERT_VALUES, IncludeGhosts );

//    PETSC_3D_Array layerDepth
//       ( m_formation->layerDA,
//         m_formation->Current_Properties ( Basin_Modelling::Depth ),
//         INSERT_VALUES, IncludeGhosts );

//    PETSC_3D_Array layerCurrentVES
//       ( m_formation->layerDA, 
//         m_formation->Current_Properties ( Basin_Modelling::VES_FP ),
//         INSERT_VALUES, IncludeGhosts );

//    PETSC_3D_Array layerCurrentMaxVES
//       ( m_formation->layerDA, 
//         m_formation->Current_Properties ( Basin_Modelling::Max_VES ),
//         INSERT_VALUES, IncludeGhosts );

//    PETSC_3D_Array layerCurrentChemicalCompaction
//       ( m_formation->layerDA, 
//         m_formation->Current_Properties ( Basin_Modelling::Chemical_Compaction ),
//         INSERT_VALUES, IncludeGhosts );

//    PETSC_3D_Array layerCurrentPo
//       ( m_formation->layerDA, 
//         m_formation->Current_Properties ( Basin_Modelling::Overpressure ),
//         INSERT_VALUES, IncludeGhosts );

//    PETSC_3D_Array layerCurrentPh
//       ( m_formation->layerDA, 
//         m_formation->Current_Properties ( Basin_Modelling::Hydrostatic_Pressure ),
//         INSERT_VALUES, IncludeGhosts );

//    // Retrieve the heat-flow result maps.
//    fluidVelocityMapX = propertyValues [ 0 ]->getGridMap ();
//    fluidVelocityMapX->retrieveData ();

//    fluidVelocityMapY = propertyValues [ 1 ]->getGridMap ();
//    fluidVelocityMapY->retrieveData ();

//    fluidVelocityMapZ = propertyValues [ 2 ]->getGridMap ();
//    fluidVelocityMapZ->retrieveData ();

//    const double deltaX  = fluidVelocityMapX->deltaI ();
//    const double deltaY  = fluidVelocityMapX->deltaJ ();
//    const double originX = fluidVelocityMapX->minI ();
//    const double originY = fluidVelocityMapX->minJ ();

//    for ( elementCount = 0; elementCount < elements.size(); elementCount++ ) {

//       if ( elements[elementCount].exists) {
//          i = elements [ elementCount ].i [ 0 ];
//          j = elements [ elementCount ].j [ 0 ];

//          lithology = m_formation->getCompoundLithology ( i, j );

//          if ( lithology != 0 ) {

//             // Retrieve element data.
//             for ( node = 1; node <= 8; ++node ) {
//                int LidxZ = m_kIndex + (( node - 1 ) < 4 ? 1 : 0);
//                int GidxY = elements [ elementCount ].j [( node - 1 ) % 4 ];
//                int GidxX = elements [ elementCount ].i [( node - 1 ) % 4 ];

//                geometryMatrix ( 1, node ) = originX + (deltaX * GidxX);
//                geometryMatrix ( 2, node ) = originY + (deltaY * GidxY);
//                geometryMatrix ( 3, node ) = layerDepth ( LidxZ, GidxY, GidxX );

//                ves                 ( node ) = layerCurrentVES ( LidxZ, GidxY, GidxX );
//                maxVes              ( node ) = layerCurrentMaxVES ( LidxZ, GidxY, GidxX );
//                overpressure        ( node ) = layerCurrentPo ( LidxZ, GidxY, GidxX );
//                hydrostaticPressure ( node ) = layerCurrentPh ( LidxZ, GidxY, GidxX );
//                temperature         ( node ) = layerTemperature ( LidxZ, GidxY, GidxX );
//                chemCompaction      ( node ) = layerCurrentChemicalCompaction ( LidxZ, GidxY, GidxX );
//             }

//             computeFluidVelocity ( 0.0, 0.0, m_zPosition,
//                                    lithology,
//                                    m_fluid,
//                                    m_chemicalCompactionRequired,
//                                    geometryMatrix,
//                                    ves,
//                                    maxVes,
//                                    hydrostaticPressure,
//                                    overpressure,
//                                    temperature,
//                                    chemCompaction,
//                                    fluidVelocity );

//             fluidVelocityMapX->setValue ( i, j, fluidVelocity ( 1 ));
//             fluidVelocityMapY->setValue ( i, j, fluidVelocity ( 2 ));
//             fluidVelocityMapZ->setValue ( i, j, fluidVelocity ( 3 ));

//             // Fill other heat flow nodes if current (i,j) position is at end of array
//             if ( i == (unsigned int )(globalXNodes) - 2 ) {
//                fluidVelocityMapX->setValue ( i + 1, j, fluidVelocity ( 1 ));
//                fluidVelocityMapY->setValue ( i + 1, j, fluidVelocity ( 2 ));
//                fluidVelocityMapZ->setValue ( i + 1, j, fluidVelocity ( 3 ));
//             }

//             if ( grid.getNumberOfYElements ()) {
//                fluidVelocityMapX->setValue ( i, j + 1, fluidVelocity ( 1 ));
//                fluidVelocityMapY->setValue ( i, j + 1, fluidVelocity ( 2 ));
//                fluidVelocityMapZ->setValue ( i, j + 1, fluidVelocity ( 3 ));
//             }

//             if ( i == (unsigned int)(globalXNodes) - 2 and grid.getNumberOfYElements ()) {
//                fluidVelocityMapX->setValue ( i + 1, j + 1, fluidVelocity ( 1 ));
//                fluidVelocityMapY->setValue ( i + 1, j + 1, fluidVelocity ( 2 ));
//                fluidVelocityMapZ->setValue ( i + 1, j + 1, fluidVelocity ( 3 ));
//             }

//          }
          
//       }

//    }

//    fluidVelocityMapX->restoreData ();
//    fluidVelocityMapY->restoreData ();
//    fluidVelocityMapZ->restoreData ();

//    m_isCalculated = true;
//    return true;
// }

// void HcFluidVelocityCalculator::allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties ) {


//    const Interface::Formation* formation = m_formation;

//    PropertyValue* vapourVelocityX;
//    PropertyValue* vapourVelocityY;
//    PropertyValue* vapourVelocityZ;

//    PropertyValue* liquidVelocityX;
//    PropertyValue* liquidVelocityY;
//    PropertyValue* liquidVelocityZ;
   

//    vapourVelocityX = (PropertyValue*)(FastcauldronSimulator::getInstance ().createMapPropertyValue ( "HCVapourVelocityX", 
//                                                                                                      m_snapshot, 0,
//                                                                                                      formation,
//                                                                                                      m_surface ));

//    vapourVelocityY = (PropertyValue*)(FastcauldronSimulator::getInstance ().createMapPropertyValue ( "HCVapourVelocityY", 
//                                                                                                      m_snapshot, 0,
//                                                                                                      formation,
//                                                                                                      m_surface ));

//    vapourVelocityZ = (PropertyValue*)(FastcauldronSimulator::getInstance ().createMapPropertyValue ( "HCVapourVelocityZ", 
//                                                                                                      m_snapshot, 0,
//                                                                                                      formation,
//                                                                                                      m_surface ));


//    liquidVelocityX = (PropertyValue*)(FastcauldronSimulator::getInstance ().createMapPropertyValue ( "HCLiquidVelocityX", 
//                                                                                                      m_snapshot, 0,
//                                                                                                      formation,
//                                                                                                      m_surface ));

//    liquidVelocityY = (PropertyValue*)(FastcauldronSimulator::getInstance ().createMapPropertyValue ( "HCLiquidVelocityY", 
//                                                                                                      m_snapshot, 0,
//                                                                                                      formation,
//                                                                                                      m_surface ));

//    liquidVelocityZ = (PropertyValue*)(FastcauldronSimulator::getInstance ().createMapPropertyValue ( "HCLiquidVelocityZ", 
//                                                                                                      m_snapshot, 0,
//                                                                                                      formation,
//                                                                                                      m_surface ));

//    properties.push_back ( vapourVelocityX );
//    properties.push_back ( vapourVelocityY );
//    properties.push_back ( vapourVelocityZ );

//    properties.push_back ( liquidVelocityX );
//    properties.push_back ( liquidVelocityY );
//    properties.push_back ( liquidVelocityZ );

// }

// bool HcFluidVelocityCalculator::initialise ( OutputPropertyMap::PropertyValueList& propertyValues ) {

//    m_depth = PropertyManager::getInstance().findOutputPropertyMap ( "Depth", m_formation, m_surface, m_snapshot );
//    m_temperature = PropertyManager::getInstance().findOutputPropertyMap ( "Temperature", m_formation, m_surface, m_snapshot );
//    m_porePressure = PropertyManager::getInstance().findOutputPropertyMap ( "PorePressure", m_formation, m_surface, m_snapshot );
//    m_ves = PropertyManager::getInstance().findOutputPropertyMap ( "Ves", m_formation, m_surface, m_snapshot );
//    m_maxVes = PropertyManager::getInstance().findOutputPropertyMap ( "MaxVes", m_formation, m_surface, m_snapshot );

//    if ( m_chemicalCompactionRequired ) {
//       m_chemicalCompaction = PropertyManager::getInstance().findOutputPropertyMap ( "ChemicalCompaction", m_formation, m_surface, m_snapshot );
//    } else {
//       m_chemicalCompaction = 0;
//    }

//    m_fluid = m_formation->fluid;

//    return m_depth != 0 and m_temperature != 0 and m_hydrostaticPressure != 0 and m_overpressure != 0 and 
//           m_ves != 0 and m_maxVes != 0 and 
//           ( m_chemicalCompactionRequired ? m_chemicalCompaction != 0 : true );
// }



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
                  hcDensity ( pvtFlash::VAPOUR_PHASE ) = m_pvtProperties->getVolumeValue ( i, j, k, 0 );
                  hcDensity ( pvtFlash::LIQUID_PHASE ) = m_pvtProperties->getVolumeValue ( i, j, k, 1 );

                  hcViscosity ( pvtFlash::VAPOUR_PHASE ) = m_pvtProperties->getVolumeValue ( i, j, k, 2 );
                  hcViscosity ( pvtFlash::LIQUID_PHASE ) = m_pvtProperties->getVolumeValue ( i, j, k, 3 );
#endif

                  lithology->getPorosity ( ves, maxVes, false, 0.0, porosity );
                  lithology->calcBulkPermeabilityNP ( ves, maxVes, porosity, permNormal, permPlane );

                  fe.setTensor ( permNormal, permPlane, permeability );

                  if ( saturation ( Saturation::VAPOUR ) > 0.0 ) {
                     hcVapourVelocity = computeMassFlux ( element, fe, porePressureCoeffs, permeability, hcDensity ( pvtFlash::VAPOUR_PHASE ), hcViscosity ( pvtFlash::VAPOUR_PHASE ));
                     hcVapourVelocity *= lithology->relativePermeability ( Saturation::VAPOUR, saturation ) / ( porosity.mixedProperty () * saturation ( Saturation::VAPOUR ));

                     // Scale to mm/year from metres/sec.
                     hcVapourVelocity *= 1000.0 * SecondsPerYear;

                     vapourVelocityMagnitude = length ( hcVapourVelocity );
                  } else {
                     hcVapourVelocity.fill ( CAULDRONIBSNULLVALUE );
                     vapourVelocityMagnitude = CAULDRONIBSNULLVALUE;
                  }

                  if ( saturation ( Saturation::LIQUID ) > 0.0 ) {
                     hcLiquidVelocity = computeMassFlux ( element, fe, porePressureCoeffs, permeability, hcDensity ( pvtFlash::LIQUID_PHASE ), hcViscosity ( pvtFlash::LIQUID_PHASE ));
                     hcLiquidVelocity *= lithology->relativePermeability ( Saturation::LIQUID, saturation ) / ( porosity.mixedProperty () * saturation ( Saturation::LIQUID ));

                     // Scale to mm/year from metres/sec.
                     hcLiquidVelocity *= 1000.0 * SecondsPerYear;

                     liquidVelocityMagnitude = length ( hcLiquidVelocity );
                  } else {
                     hcLiquidVelocity.fill ( CAULDRONIBSNULLVALUE );
                     liquidVelocityMagnitude = CAULDRONIBSNULLVALUE;
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
                  hcVapourMaps [ 0 ]->setValue ( i, j, k, CAULDRONIBSNULLVALUE );
                  hcVapourMaps [ 1 ]->setValue ( i, j, k, CAULDRONIBSNULLVALUE );
                  hcVapourMaps [ 2 ]->setValue ( i, j, k, CAULDRONIBSNULLVALUE );
                  hcVapourMaps [ 3 ]->setValue ( i, j, k, CAULDRONIBSNULLVALUE );

                  hcLiquidMaps [ 0 ]->setValue ( i, j, k, CAULDRONIBSNULLVALUE );
                  hcLiquidMaps [ 1 ]->setValue ( i, j, k, CAULDRONIBSNULLVALUE );
                  hcLiquidMaps [ 2 ]->setValue ( i, j, k, CAULDRONIBSNULLVALUE );
                  hcLiquidMaps [ 3 ]->setValue ( i, j, k, CAULDRONIBSNULLVALUE );
               }
               
            }

         } else {

            for ( k = elements.firstK (); k <= elements.lastK (); ++k ) {
               hcVapourMaps [ 0 ]->setValue ( i, j, k, CAULDRONIBSNULLVALUE );
               hcVapourMaps [ 1 ]->setValue ( i, j, k, CAULDRONIBSNULLVALUE );
               hcVapourMaps [ 2 ]->setValue ( i, j, k, CAULDRONIBSNULLVALUE );
               hcVapourMaps [ 3 ]->setValue ( i, j, k, CAULDRONIBSNULLVALUE );

               hcLiquidMaps [ 0 ]->setValue ( i, j, k, CAULDRONIBSNULLVALUE );
               hcLiquidMaps [ 1 ]->setValue ( i, j, k, CAULDRONIBSNULLVALUE );
               hcLiquidMaps [ 2 ]->setValue ( i, j, k, CAULDRONIBSNULLVALUE );
               hcLiquidMaps [ 3 ]->setValue ( i, j, k, CAULDRONIBSNULLVALUE );
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
