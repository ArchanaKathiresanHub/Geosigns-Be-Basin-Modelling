//
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "GeoPhysicsProjectHandle.h"

// std library
#include <iomanip>
#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif
#include <cmath>

// DataAccess library
#include "Interface.h"
#include "AllochthonousLithology.h"
#include "AllochthonousLithologyDistribution.h"
#include "CrustFormation.h"
#include "ContinentalCrustHistoryGenerator.h"
#include "Formation.h"
#include "GridMap.h"
#include "IgneousIntrusionEvent.h"
#include "MantleFormation.h"
#include "MobileLayer.h"
#include "OceanicCrustThicknessHistoryData.h"
#include "PaleoFormationProperty.h"
#include "PaleoSurfaceProperty.h"
#include "PaleoProperty.h"
#include "ProjectHandle.h"
#include "Property.h"
#include "PropertyValue.h"
#include "RunParameters.h"
#include "Snapshot.h"
#include "Surface.h"

// GeopPhysics library
#include "AllochthonousLithologyManager.h"
#include "BasementLithology.h"
#include "CompoundLithology.h"
#include "EffectiveCrustalThicknessCalculator.h"
#include "FracturePressureCalculator.h"
#include "GeoPhysicalConstants.h"
#include "GeoPhysicsCrustFormation.h"
#include "GeoPhysicsFluidType.h"
#include "GeoPhysicsFormation.h"
#include "GeoPhysicsMantleFormation.h"
#include "GeoPhysicsObjectFactory.h"
#include "LithologyManager.h"

// utilities library
#include "NumericFunctions.h"
#include "errorhandling.h"
#include "FilePath.h"
#include "ConstantsMathematics.h"
using Utilities::Maths::MilliWattsToWatts;
#include "ConstantsPhysics.h"
using Utilities::Physics::AccelerationDueToGravity;
#include "LogHandler.h"

using namespace DataAccess;
using namespace CBMGenerics;

GeoPhysics::ProjectHandle::ProjectHandle (database::ProjectFileHandlerPtr pfh, const std::string & name, const std::string & accessMode, const Interface::ObjectFactory* objectFactory ) :
   DataAccess::Interface::ProjectHandle ( pfh, name, accessMode, objectFactory ) {

   m_lithologyManager = new LithologyManager ( this );

   m_allochthonousLithologyManager = new AllochthonousLithologyManager ( this );

   m_isALCMode = isALC();

   m_constrainedBasaltTemperature = 1000;

   if( !loadALCConfigurationFile( "InterfaceData.cfg" ) ) {
      LogHandler( LogHandler::WARNING_SEVERITY ) << "Can't load ALC configuration file. Default values will be used";
   }

   // Now load anything that was not loaded by default in the constructor of the default project handle.
   ProjectHandle::loadFaults ();
   ProjectHandle::loadConstrainedOverpressureIntervals ();

   const bool status = ProjectHandle::loadCrustThinningHistory ();

   if( !status && m_isALCMode ) {
      throw RecordException ( "Error: The Crust thinning history table is empty!" );
   }

   ProjectHandle::loadMantleThicknessHistory ();
   correctCrustThicknessHistory ();
   ProjectHandle::loadHeatFlowHistory ();

   // Construct the fluid-property tables for each fluid.
   loadFluidPropertyTables ();

   // Now apply a correction to the constant density for "standard
   // pressure and temperature" of hydrostatic calculation mode.
   correctSimpleFluidDensities ();

   correctThermalConductivityTables ();
   addSimpleLithologiesToLithologyManager ();

   loadFracturePressureCalculator ();

}



//------------------------------------------------------------//

GeoPhysics::ProjectHandle::~ProjectHandle () {
   delete m_lithologyManager;
   delete m_allochthonousLithologyManager;
   delete m_basementLithoProps;
   delete m_fracturePressureCalculator;
}

//------------------------------------------------------------//

void GeoPhysics::ProjectHandle::loadFracturePressureCalculator () {
   m_fracturePressureCalculator = dynamic_cast<const GeoPhysics::ObjectFactory*>( getFactory ())->produceFracturePressureCalculator ( this );
}

//------------------------------------------------------------//
GeoPhysics::ConfigFileParameterAlc * GeoPhysics::ProjectHandle::getBasementLithologyProps() const {
   return m_basementLithoProps;
}

//------------------------------------------------------------//

bool GeoPhysics::ProjectHandle::startActivity ( const std::string& name, const DataAccess::Interface::Grid* grid, bool saveAsInputGrid, bool createResultsFile, bool append ) {
   const bool started = DataAccess::Interface::ProjectHandle::startActivity ( name, grid, saveAsInputGrid, createResultsFile, append );

   if ( started ) {
      // Now get the boundaries of the activity grid for this process.
      const Interface::Grid* activityGrid = getActivityOutputGrid ();

      m_firstI [ 0 ] = static_cast<unsigned int>(activityGrid->firstI(false));
      m_firstI [ 1 ] = static_cast<unsigned int>(activityGrid->firstI(true));

      m_firstJ [ 0 ] = static_cast<unsigned int>(activityGrid->firstJ(false));
      m_firstJ [ 1 ] = static_cast<unsigned int>(activityGrid->firstJ(true));

      m_lastI  [ 0 ] = static_cast<unsigned int>(activityGrid->lastI(false));
      m_lastI  [ 1 ] = static_cast<unsigned int>(activityGrid->lastI(true));

      m_lastJ  [ 0 ] = static_cast<unsigned int>(activityGrid->lastJ(false));
      m_lastJ  [ 1 ] = static_cast<unsigned int>(activityGrid->lastJ(true));

      m_cauldronGridDescription.nrI = activityGrid->numIGlobal ();
      m_cauldronGridDescription.nrJ = activityGrid->numJGlobal ();

      m_cauldronGridDescription.originI = activityGrid->minIGlobal ();
      m_cauldronGridDescription.originJ = activityGrid->minJGlobal ();

      m_cauldronGridDescription.deltaI = activityGrid->deltaIGlobal ();
      m_cauldronGridDescription.deltaJ = activityGrid->deltaJGlobal ();

   }

   return started;
}

//------------------------------------------------------------//

bool GeoPhysics::ProjectHandle::setFormationLithologies ( const bool canRunGeomorph,
                                                          const bool includeFaults ) {

   Interface::MutableFormationList::iterator formationIter;
   bool createdLithologies = true;
   bool errorOccurredInLayer;

   for ( formationIter = m_formations.begin (); formationIter != m_formations.end (); ++formationIter ) {

      if ((*formationIter)->kind () == Interface::SEDIMENT_FORMATION ) {
         GeoPhysics::GeoPhysicsFormation* formation = dynamic_cast<GeoPhysics::GeoPhysicsFormation*>(*formationIter);

         createdLithologies = createdLithologies and formation->setLithologiesFromStratTable ();
      }

   }

   createdLithologies = createdLithologies and dynamic_cast<GeoPhysics::GeoPhysicsCrustFormation*>( m_crustFormation )->setLithologiesFromStratTable ();
   createdLithologies = createdLithologies and dynamic_cast<GeoPhysics::GeoPhysicsMantleFormation*>( m_mantleFormation )->setLithologiesFromStratTable ();

   // Set the allochthonous lithologies.
   if ( canRunGeomorph ) {

      // Initialise the allochthonous lithology manager interpolators.
      m_allochthonousLithologyManager->initialiseInterpolators ( getOutputDir ());

      // No more interpolation data can be added after this point.
      //   1. The interpolators are sorted into age-order, oldest first.
      //   2. The lithologies are set in the lithology arrays, according to the interpolant.
      //      The number of morphing steps depends on the optimisation-level.
      m_allochthonousLithologyManager->freeze ( getRunParameters ()->getOptimisationLevel ());

      // Set the grid description for each of the interpolators.
      m_allochthonousLithologyManager->setGridDescription ( m_cauldronGridDescription );

      // Pass the interpolator to those layers that are involved in the allochthonous modelling.
      m_allochthonousLithologyManager->initialiseLayers ();
   }

   m_basinHasActiveFaults = false;

   // Set the fault lithologies.
   if ( includeFaults )
   {
      bool layerHasFaults;

      for ( formationIter = m_formations.begin (); formationIter != m_formations.end (); ++formationIter ) {

         // There are no faults in the basement formations.
         if ((*formationIter)->kind () == Interface::SEDIMENT_FORMATION ) {
            GeoPhysics::GeoPhysicsFormation* formation = dynamic_cast<GeoPhysics::GeoPhysicsFormation*>(*formationIter);

            formation->setFaultLithologies ( layerHasFaults, errorOccurredInLayer );
            m_basinHasActiveFaults = m_basinHasActiveFaults or layerHasFaults;
            createdLithologies = createdLithologies and not errorOccurredInLayer;
         }

      }
   }

   return createdLithologies;
}

//------------------------------------------------------------//

void GeoPhysics::ProjectHandle::loadFluidPropertyTables () {
   try
   {
      for ( Interface::MutableFluidTypeList::iterator fluidIter = m_fluidTypes.begin(); fluidIter != m_fluidTypes.end (); ++fluidIter )
      {
         GeoPhysics::FluidType* fluid = static_cast<GeoPhysics::FluidType*>(*fluidIter);
         fluid->loadPropertyTables ();
      }
   }
   catch ( formattingexception::GeneralException & ex )
   {
      if (getRank () == 0)
      {
         std::cerr << ex.what();
      }
      exit (1);
   }
   catch (...)
   {
      if (getRank () == 0)
      {
         std::cerr << "Fatal error when initialising log file(s).";
      }
      exit (1);
   }
}

//------------------------------------------------------------//

void GeoPhysics::ProjectHandle::correctThermalConductivityTables () {
   for ( Interface::MutableLithoTypeList::iterator simpleLithologyIter = m_lithoTypes.begin(); simpleLithologyIter != m_lithoTypes.end (); ++simpleLithologyIter ) {

     if( (*simpleLithologyIter)->getName() == DataAccess::Interface::ALCBasalt ) {
        continue;
     }
     SimpleLithology* simpleLithology = dynamic_cast<SimpleLithology*>(*simpleLithologyIter);

     const double stdThCondVal = simpleLithology->getThCondVal();
     const double correctionFactor = stdThCondVal / simpleLithology->thermalconductivity ( GeoPhysics::FluidType::DefaultThermalConductivityCorrectionTemperature );
     simpleLithology->correctThermCondPoint ( correctionFactor );

  }

}

//------------------------------------------------------------//

void GeoPhysics::ProjectHandle::addSimpleLithologiesToLithologyManager () {
   for ( Interface::MutableLithoTypeList::iterator lithoIter = m_lithoTypes.begin (); lithoIter != m_lithoTypes.end (); ++lithoIter ) {

      Interface::LithoType* lithoType = *lithoIter;
      if( m_isALCMode && ( lithoType->getName() == getCrustLithoName() || lithoType->getName() == getMantleLithoName() || lithoType->getName() == DataAccess::Interface::ALCBasalt )) {
         BasementLithology * litho = dynamic_cast<BasementLithology*>( lithoType );
         assert( litho );
         litho->setBasementLithoProperties( * m_basementLithoProps );
         getLithologyManager ().addSimpleLithology ( litho );
      } else {
         SimpleLithology* litho = static_cast<SimpleLithology*>(lithoType);
         getLithologyManager ().addSimpleLithology ( litho );
      }
   }

}

//------------------------------------------------------------//

void GeoPhysics::ProjectHandle::correctSimpleFluidDensities () {
   // The temperature gradient in the project file is in C/Km and not C/m and so needs to be converted.
   const double temperatureGradient = 0.001 * getRunParameters ()->getTemperatureGradient ();

   for ( Interface::MutableFluidTypeList::iterator fluidIter = m_fluidTypes.begin(); fluidIter != m_fluidTypes.end (); ++fluidIter ) {
      auto fluid = static_cast<GeoPhysics::FluidType*>(*fluidIter);

      fluid->correctSimpleDensity ( GeoPhysics::FluidType::DefaultStandardDepth,
                                    GeoPhysics::FluidType::DefaultHydrostaticPressureGradient,
                                    GeoPhysics::FluidType::StandardSurfaceTemperature,
                                    temperatureGradient );
   }

}

//------------------------------------------------------------//

GeoPhysics::LithologyManager& GeoPhysics::ProjectHandle::getLithologyManager () const {
   return *m_lithologyManager;
}

//------------------------------------------------------------//

const GeoPhysics::FracturePressureCalculator& GeoPhysics::ProjectHandle::getFracturePressureCalculator () const {
   return *m_fracturePressureCalculator;
}

//------------------------------------------------------------//

GeoPhysics::AllochthonousLithologyManager& GeoPhysics::ProjectHandle::getAllochthonousLithologyManager () const {
   return *m_allochthonousLithologyManager;
}

//------------------------------------------------------------//

bool GeoPhysics::ProjectHandle::initialise ( const bool readSizeFromVolumeData,
                                             const bool printTable ) {
   // Since the input depends on some results, these need to be
   // taken into account when setting the valid node array.
   bool result = initialiseValidNodes ( readSizeFromVolumeData );

   // Sets the age of basin field.
   setBasinAge ();

   // The reason for the order of operands in the expressions below is so
   // that the functions are executed on all processes even if result is false.
   // Not doing so may result in a dead-lock situation.
   // Under the assumption that in the expression: a and b, a is always evaluated
   // first regardless.
   if( m_isALCMode ) {
      // check inputs before they are modified by addCrustThinningHistoryMaps method
      /// @todo The inputs should never be modified, only outputs should be interpolated in this case
      checkAlcCrustHistoryInput();
      addCrustThinningHistoryMaps();
   }

   result = createSeaBottomTemperature    () and result;
   result = createPaleoBathymetry         () and result;
   result = createMantleHeatFlow          () and result;
   createCrustThickness();
   if( m_isALCMode ){
     result = createBasaltThicknessAndECT () and result;
   }
   result = determineLayerMinMaxThickness () and result;
   result = determineCrustThinningRatio   () and result;
   result = determineMaximumNumberOfSegmentsPerLayer ( readSizeFromVolumeData, printTable ) and result;

   return result;
}

//------------------------------------------------------------//

void GeoPhysics::ProjectHandle::createCrustThickness() const
{
   ContinentalCrustHistoryGenerator generator(*getActivityOutputGrid(), *m_crustFormation, m_validator, getGlobalOperations());
   generator.createCrustThickness(getBottomBoundaryConditions());
   m_crustFormation->setInitialCrustalThickness(generator.getInitialCrustalThickness());
   if( m_isALCMode ){
      m_contCrustThicknessHistory = generator.getcrustThicknessHistory();
   }
   else{
      m_crustThicknessHistory = generator.getcrustThicknessHistory();
   }
}

//------------------------------------------------------------//

void GeoPhysics::ProjectHandle::checkAlcCrustHistoryInput() const
{
   Interface::PaleoFormationPropertyList* crustThicknesses = getCrustFormation()->getPaleoThicknessHistory();
   for (auto continentalCrustReverseIter = crustThicknesses->rbegin(); continentalCrustReverseIter != crustThicknesses->rend(); ++continentalCrustReverseIter) {
      const PaleoFormationProperty* contCrustThicknessInstance = *continentalCrustReverseIter;
      const double age = contCrustThicknessInstance->getSnapshot()->getTime();
      const auto oceanicCrustThicknessIt = std::find_if( m_tableOceanicCrustThicknessHistory.data().begin(), m_tableOceanicCrustThicknessHistory.data().end(),
         [&age]( std::shared_ptr<const OceanicCrustThicknessHistoryData> obj ) { return obj->getAge() == age; } );
      if (oceanicCrustThicknessIt == m_tableOceanicCrustThicknessHistory.data().end()) {
         throw std::invalid_argument( "There is no oceanic crustal thickness corresponding to the continental crustal thickness defined at " + std::to_string( age ) + "Ma" );
      }
   }
}

//------------------------------------------------------------//

void GeoPhysics::ProjectHandle::switchLithologies ( const double age ) {

   // Would it be better here have the lithology-manager manage this?
   // Since it can loop over all compound lithologies where as in the
   // here and in the layer it is necessary to iterate over all pointers
   // to compound lithologies, thereby performing the action multiple times
   // in some (most?) lithologies.

   for ( Interface::MutableFormationList::iterator formationIter = m_formations.begin (); formationIter != m_formations.end (); ++formationIter ) {
      GeoPhysics::GeoPhysicsFormation* formation = dynamic_cast<GeoPhysics::GeoPhysicsFormation*>(*formationIter);
      formation->switchLithologies ( age );
   }

}

//------------------------------------------------------------//

void GeoPhysics::ProjectHandle::setBasinAge () {

   m_basinAge = getCrustFormation ()->getTopSurface ()->getSnapshot ()->getTime ();

}

//------------------------------------------------------------//

bool GeoPhysics::ProjectHandle::createSeaBottomTemperature () const
{

   bool status;

   //Create 2D Array of Polyfunction for Sea Bottom Temperature
   m_seaBottomTemperature.reallocate ( getActivityOutputGrid ());

   Interface::PaleoPropertyList* surfaceTemperatureHistory = getSurfaceTemperatureHistory ();

   if ( surfaceTemperatureHistory != nullptr && surfaceTemperatureHistory->size() != 0 ) {

      for ( Interface::PaleoPropertyList::const_iterator surfaceTemperatureIter = surfaceTemperatureHistory->begin (); surfaceTemperatureIter != surfaceTemperatureHistory->end (); ++surfaceTemperatureIter ) {

         const Interface::PaleoProperty* surfaceTemperatureInstance = *surfaceTemperatureIter;
         const Interface::GridMap* surfaceTemperatureMap = surfaceTemperatureInstance->getMap ( Interface::SurfaceTemperatureHistoryInstanceMap );
         const double age = surfaceTemperatureInstance->getSnapshot ()->getTime ();

         // This should be with ghost nodes.
         surfaceTemperatureMap->retrieveGhostedData ();

         for ( unsigned int i = surfaceTemperatureMap->getGrid()->firstI ( true ); i <= surfaceTemperatureMap->getGrid()->lastI ( true ); ++i ) {

            for ( unsigned int j = surfaceTemperatureMap->getGrid()->firstJ ( true ); j <= surfaceTemperatureMap->getGrid()->lastJ ( true ); ++j ) {

               if ( m_validator.isValid( i, j )) {
                  m_seaBottomTemperature ( i, j ).AddPoint ( age, surfaceTemperatureMap->getValue ( i, j ));
               }

            }

         }

         surfaceTemperatureMap->restoreData ( false, true );
         surfaceTemperatureMap->release();
      }

      delete surfaceTemperatureHistory;
      status = true;
   } else {
      status = false;

      getMessageHandler ().printLine (  "Basin_Error:  Surface temperature is not defined." );
   }

   return status;
}

//------------------------------------------------------------//

bool GeoPhysics::ProjectHandle::createPaleoBathymetry () {

   bool status;

   //Create 2D Array of Polyfunction for sea bottom depth.
   m_seaBottomDepth.reallocate ( getActivityOutputGrid ());

   Interface::PaleoPropertyList* surfaceDepthHistory = getSurfaceDepthHistory ();

   if ( surfaceDepthHistory != nullptr ) {

      for ( Interface::PaleoPropertyList::const_iterator surfaceDepthIter = surfaceDepthHistory->begin (); surfaceDepthIter != surfaceDepthHistory->end (); ++surfaceDepthIter ) {

         const Interface::PaleoProperty* surfaceDepthInstance = *surfaceDepthIter;
         const double age = surfaceDepthInstance->getSnapshot ()->getTime ();
         const Interface::GridMap* surfaceDepthMap;

         if ( age > 0.0 ) {
            surfaceDepthMap = surfaceDepthInstance->getMap ( Interface::SurfaceDepthHistoryInstanceMap );
         } else {
            // The present-day depth-map is obtained from the top surface of the sediments.
            surfaceDepthMap = dynamic_cast<const Interface::GridMap*>((*m_surfaces.begin ())->getInputDepthMap ());
         }

         // This should be with ghost nodes.
         surfaceDepthMap->retrieveGhostedData ();

         for ( unsigned int i = surfaceDepthMap->firstI (true); i <= surfaceDepthMap->lastI (true); ++i ) {

            for ( unsigned int j = surfaceDepthMap->firstJ (true); j <= surfaceDepthMap->lastJ (true); ++j ) {

               if (m_validator.isValid( i, j )) {
                  m_seaBottomDepth ( i, j ).AddPoint ( age, surfaceDepthMap->getValue ( i, j ));
               }

            }

         }

         surfaceDepthMap->restoreData ( false, true );
         surfaceDepthMap->release();
      }

      delete surfaceDepthHistory;
      status = true;
   } else {
      status = false;

      getMessageHandler ().printLine (  "Basin_Error:  Surface depth is not defined." );
   }

   return status;
}

//------------------------------------------------------------//

bool GeoPhysics::ProjectHandle::createMantleHeatFlow () const
{

   if ( getBottomBoundaryConditions () == Interface::MANTLE_HEAT_FLOW ) {
      //Create 2D Array of Polyfunction for Mantle HeatFlow
      m_mantleHeatFlow.reallocate ( getActivityOutputGrid ());

      Interface::PaleoSurfacePropertyList* heatFlowHistory = getHeatFlowHistory ();

      for ( Interface::PaleoSurfacePropertyList::const_iterator heatFlowIter = heatFlowHistory->begin (); heatFlowIter != heatFlowHistory->end (); ++heatFlowIter ) {

         const Interface::PaleoSurfaceProperty* heatFlowInstance = dynamic_cast<const Interface::PaleoSurfaceProperty*>(*heatFlowIter);
         const Interface::GridMap* heatFlowMap = heatFlowInstance->getMap ( Interface::HeatFlowHistoryInstanceHeatFlowMap );
         const double age = heatFlowInstance->getSnapshot ()->getTime ();

         heatFlowMap->retrieveGhostedData ();

         for ( unsigned int i = heatFlowMap->getGrid()->firstI ( true ); i <= heatFlowMap->getGrid()->lastI ( true ); ++i ) {

            for ( unsigned int j = heatFlowMap->getGrid()->firstJ ( true ); j <= heatFlowMap->getGrid()->lastJ ( true ); ++j ) {

               if (m_validator.isValid( i, j )) {
                  m_mantleHeatFlow ( i, j ).AddPoint( age, heatFlowMap->getValue ( i, j ) * MilliWattsToWatts );
               }

            }

         }

         heatFlowMap->restoreData ( false, true );
         heatFlowMap->release();
      }

      delete heatFlowHistory;
   }

   return true;
}

//------------------------------------------------------------//
bool GeoPhysics::ProjectHandle::createBasaltThicknessAndECT () const
{

   if (not m_isALCMode) return false;
   bool status = false;
   //Create 2D Array of Polyfunction for Crust Thickness
   m_crustThicknessHistory.reallocate ( getActivityOutputGrid() );
   m_basaltThicknessHistory.reallocate( getActivityOutputGrid() );
   m_endOfRiftEvent.reallocate        ( getActivityOutputGrid() );

   Interface::PaleoFormationPropertyList* crustThicknesses = getCrustFormation()->getPaleoThicknessHistory();
   auto*  crust = dynamic_cast<GeoPhysics::GeoPhysicsCrustFormation*>(m_crustFormation);
   const double initialLithosphericMantleThickness = getMantleFormation()->getInitialLithosphericMantleThickness();
   const double initialCrustalThickness            = crust->getInitialCrustalThickness();

   EffectiveCrustalThicknessCalculator ectCalculator( crustThicknesses,
                                                      m_tableOceanicCrustThicknessHistory,
                                                      initialLithosphericMantleThickness,
                                                      initialCrustalThickness,
                                                      m_validator );
   try{
      ectCalculator.compute( m_crustThicknessHistory,
                             m_basaltThicknessHistory,
                             m_endOfRiftEvent );
      status = true;
   }
   catch ( std::invalid_argument& ex ) {
      LogHandler( LogHandler::ERROR_SEVERITY ) << "One of the Advanced Lithosphere Calculator (ALC) input is invalid (see details bellow)";
      LogHandler( LogHandler::ERROR_SEVERITY ) << ex.what();
   }
   catch (...) {
      LogHandler( LogHandler::ERROR_SEVERITY ) << "The Advanced Lithosphere Calculator encounterred a fatal error (unkown details)";
   }

   delete crustThicknesses;
   return status;
}

//------------------------------------------------------------//

bool GeoPhysics::ProjectHandle::determineLayerMinMaxThickness () {
   for ( Interface::MutableFormationList::iterator formationIter = m_formations.begin (); formationIter != m_formations.end (); ++formationIter ) {

      if ((*formationIter)->kind () == Interface::SEDIMENT_FORMATION ) {
         GeoPhysics::GeoPhysicsFormation* formation = dynamic_cast<GeoPhysics::GeoPhysicsFormation*>(*formationIter);

         formation->determineMinMaxThickness ();
      }

   }

   dynamic_cast<GeoPhysics::GeoPhysicsCrustFormation*>( m_crustFormation )->determineMinMaxThickness ();
   dynamic_cast<GeoPhysics::GeoPhysicsMantleFormation*>( m_mantleFormation )->determineMinMaxThickness ();

   return true;
}

//------------------------------------------------------------//

bool GeoPhysics::ProjectHandle::determineCrustThinningRatio () const
{

   // Should this function be in the Crust class?

   auto*  crust  = dynamic_cast<GeoPhysics::GeoPhysicsCrustFormation*>( m_crustFormation );
   auto* mantle = dynamic_cast<GeoPhysics::GeoPhysicsMantleFormation*>( m_mantleFormation );

   bool status = true;
   const double mantleElementHeight = getRunParameters ()->getBrickHeightMantle ();

   status = status and crust->determineCrustThinningRatio ();
   const double minimumCrustThickness = crust->getMinimumThickness ();
   const double maximumBasementThickness = ( m_isALCMode ? mantle->getInitialLithosphericMantleThickness() + crust->getInitialCrustalThickness() :
                                          mantle->getPresentDayThickness () + crust->getCrustMaximumThicknessHistory ( 0.0 ) );
   const double maximumCrustThinningRatio = crust->getCrustThinningRatio ();


   if ( not status ) {
      getMessageHandler ().printLine ( " Basin_Error:  Crust has some non-positive thickness values." );
      getMessageHandler ().printLine ( " Basin_Error:  For correct execution all crust thickness values must be positive" );
   } else if ( status and maximumCrustThinningRatio > MaximumReasonableCrustThinningRatio and not m_isALCMode ) {
      getMessageHandler ().printLine ( " Basin_Warning:  Crust has very large ratio of crust thicknesses." );
      getMessageHandler ().printLine ( " Basin_Warning:  This can result in an unreasonable number of elements in the basement." );
      getMessageHandler ().printLine ( " Basin_Warning:  Possibly, resulting in extended execution-times." );
      getMessageHandler ().print ( " Basin_Warning:  The mantle will have approximately " );
      getMessageHandler ().print ( int ( std::ceil ((( maximumBasementThickness - minimumCrustThickness ) / mantleElementHeight ) * maximumCrustThinningRatio )));
      getMessageHandler ().printLine ( " elements in the depth-direction." );
   }

   return status;
}

//------------------------------------------------------------//

bool GeoPhysics::ProjectHandle::determineMaximumNumberOfSegmentsPerLayer ( const bool readSizeFromVolumeData,
                                                                           const bool printTable ) {

   if ( printTable and getRank () == 0 ) {
      cout << endl
           << "------------------------- Number of Segments --------------------------" << endl;
      cout << "        LayerName    (Depo)Age  Min.Thickness  Max.Thickness    Effective Max. Elem. Hgt.   Nb.Seg " << endl << endl;
   }

   unsigned int totalSegmentCount = 0;

   for ( Interface::MutableFormationList::iterator formationIter = m_formations.begin (); formationIter != m_formations.end (); ++formationIter ) {

      GeoPhysics::GeoPhysicsFormation* formation = dynamic_cast<GeoPhysics::GeoPhysicsFormation*>(*formationIter);

      const unsigned int numberOfSegments = formation->setMaximumNumberOfElements ( readSizeFromVolumeData );
      totalSegmentCount += numberOfSegments;

      if ( printTable and getRank () == 0 ) {

         if ( formation->isMantle ()) {
            std::cout << std::setw ( 20 ) << formation->getName ()
                      << std::setw ( 10 ) << -1
                      << std::setw ( 15 ) << formation->getMinimumThickness ()
                      << std::setw ( 15 ) << formation->getMaximumThickness ()
                      << std::setw ( 24 ) << getRunParameters ()->getBrickHeightMantle ()
                      << std::setw ( 10 ) << formation->getMaximumNumberOfElements ()
                      << std::endl;
         } else if ( formation->isCrust ()) {
            std::cout << std::setw ( 20 ) << formation->getName ()
                      << std::setw ( 10 ) << formation->getTopSurface ()->getSnapshot ()->getTime ()
                      << std::setw ( 15 ) << formation->getMinimumThickness ()
                      << std::setw ( 15 ) << formation->getMaximumThickness ()
                      << std::setw ( 24 ) << getRunParameters ()->getBrickHeightCrust ()
                      << std::setw ( 10 ) << formation->getMaximumNumberOfElements ()
                      << std::endl;
         } else {
            // Sediments
            std::cout << std::setw ( 20 ) << formation->getName ()
                      << std::setw ( 10 ) << formation->getTopSurface ()->getSnapshot ()->getTime ()
                      << std::setw ( 15 ) << formation->getMinimumThickness ()
                      << std::setw ( 15 ) << formation->getMaximumThickness ()
                      << std::setw ( 24 ) << getRunParameters ()->getBrickHeightSediment () / formation->getDepthRefinementFactor ()
                      << std::setw ( 10 ) << formation->getMaximumNumberOfElements ()
                      << std::endl;
         }

      }

   }

   if ( printTable and getRank () == 0 ) {
      cout << "                                                                           -------------------" << endl;
      std::cout << std::setw ( 84 ) << "Total"
                << std::setw ( 10 ) << totalSegmentCount
                << std::endl;
      cout << "----------------------------------------------------------------------------------------------" << endl;
   }


   return true;
}


//------------------------------------------------------------//

bool GeoPhysics::ProjectHandle::determinePermafrost ( std::vector<double>& timeSteps, std::vector<double>& permafrostAges ) {

   if( m_permafrost ) {
      bool isPermafrost = false;
      permafrostAges.clear();
      timeSteps.clear();

      // m_surfaceTemperatureHistory is sorted - youngest first on the list.

      if( m_surfaceTemperatureHistory.size() > 1 ) {

    // find the negative temperature in the Surface Temperature history table and calculate the time interval.
         double min, max;

         Interface::PaleoPropertyList* surfaceTemperatureHistory = getSurfaceTemperatureHistory ();
         Interface::PaleoPropertyList::const_reverse_iterator surfaceTemperatureIter = surfaceTemperatureHistory->rbegin();

         double age = ( * surfaceTemperatureIter )->getSnapshot ()->getTime ();

         ++ surfaceTemperatureIter;

         for ( ; surfaceTemperatureIter != surfaceTemperatureHistory->rend (); ++ surfaceTemperatureIter ) {
            const Interface::PaleoProperty* surfaceTemperatureInstance = *surfaceTemperatureIter;
            const Interface::GridMap* surfaceTemperatureMap = surfaceTemperatureInstance->getMap ( Interface::SurfaceTemperatureHistoryInstanceMap );

            const double currentAge = surfaceTemperatureInstance->getSnapshot ()->getTime ();
            surfaceTemperatureMap->retrieveData ();
            surfaceTemperatureMap->getMinMaxValue ( min, max );

            if( min < 0.0 ) {
               isPermafrost = true;
            }
            if( isPermafrost ) {
               if( age != currentAge ) {
                  permafrostAges.push_back( age );
                  timeSteps.push_back ( ( age - currentAge ) * 0.125 );
               }
            }

            age = currentAge;
            surfaceTemperatureMap->restoreData ();
         }
         if( permafrostAges.empty() ) {
            isPermafrost = false;
         }
         if( isPermafrost && permafrostAges.back() != 0.0  ) {
            permafrostAges.push_back( 0.0 );
            timeSteps.push_back ( 0.0 );
         }
         if( permafrostAges.size() != 0 ) {
            std::reverse( permafrostAges.begin(), permafrostAges.end() );
            std::reverse( timeSteps.begin(), timeSteps.end() );
         }
         delete surfaceTemperatureHistory;
      }
      setPermafrost( isPermafrost );
   }

   return m_permafrost;
}

//------------------------------------------------------------//

bool GeoPhysics::ProjectHandle::initialiseLayerThicknessHistory ( const bool overpressureCalculation ) {

   Interface::MutableFormationList::iterator formationIter;

   // Initially the check is only for negative thicknesses in mobile layers.
   IntegerArray numberOfErrorsPerLayer ( m_formations.size () - 2, 0 );
   unsigned int i;

   for ( formationIter = m_formations.begin (); formationIter != m_formations.end (); ++formationIter ) {

      GeoPhysics::GeoPhysicsFormation* formation = dynamic_cast<GeoPhysics::GeoPhysicsFormation*>(*formationIter);

      formation->m_solidThickness.reallocate ( getActivityOutputGrid (), formation->getMaximumNumberOfElements ());
      formation->m_realThickness.reallocate  ( getActivityOutputGrid (), formation->getMaximumNumberOfElements ());
      formation->m_presentDayErodedThickness.reallocate ( getActivityOutputGrid ());

   }

   for ( formationIter = m_formations.begin (); formationIter != m_formations.end (); ++formationIter ) {
      GeoPhysics::GeoPhysicsFormation* formation = dynamic_cast<GeoPhysics::GeoPhysicsFormation*>(*formationIter);
      formation->retrieveAllThicknessMaps ();
   }

   FloatStack uncMaxVes;
   FloatStack uncThickness;
   bool errorFound = false;

   for ( i = firstI ( true ); i <= lastI ( true ); ++i ) {

      for ( unsigned int j = firstJ ( true ); j <= lastJ ( true ); ++j ) {

         if ( getNodeIsValid ( i, j )) {

            for ( int formCount = int (m_formations.size ()) - 1; formCount >= 0; --formCount ) {

               auto* formation = dynamic_cast<GeoPhysics::GeoPhysicsFormation*>( m_formations [ static_cast<unsigned int>(formCount)]);

               if ( not computeThicknessHistories ( i, j, formation, numberOfErrorsPerLayer )) {
                  errorFound = true;
               }

            }

         // Iterate over all sediment-layers.
            for ( formationIter = m_formations.begin (); formationIter != m_formations.end (); ++formationIter )
            {
               GeoPhysics::GeoPhysicsFormation* formation = dynamic_cast<GeoPhysics::GeoPhysicsFormation*>( *formationIter );

               if ( formation->kind () == Interface::SEDIMENT_FORMATION ) {
                  storePresentDayThickness ( i, j, formation );
               }

            }



            // Start accounting for unconformities
            int nrActUnc = 0;
            uncMaxVes.push_front (0);
            uncThickness.push_front (100000);

            // And now from top to bottom
            // Do not include crust or mantle !!
            for ( formationIter = m_formations.begin (); formationIter != m_formations.end (); ++formationIter )
            {
               GeoPhysics::GeoPhysicsFormation* formation = dynamic_cast<GeoPhysics::GeoPhysicsFormation*>( *formationIter );

               if ( formation->kind () == Interface::SEDIMENT_FORMATION ) {
                  compFCThicknessHistories ( i, j, overpressureCalculation, formation, nrActUnc, uncMaxVes, uncThickness );
               }

            }

            // Finish accounting for unconformities
            uncMaxVes.pop_front ();
            uncThickness.pop_front ();

            if (0 != uncMaxVes.size ())
            {
               uncMaxVes.clear ();
            }

         }

      }

   }

   for ( formationIter = m_formations.begin (); formationIter != m_formations.end (); ++formationIter ) {
      GeoPhysics::GeoPhysicsFormation* formation = dynamic_cast<GeoPhysics::GeoPhysicsFormation*>(*formationIter);
      formation->restoreAllThicknessMaps ();
   }

   for ( i = 0; i < numberOfErrorsPerLayer.size (); ++i ) {

      if ( numberOfErrorsPerLayer [ i ] > MaximumNumberOfErrorsPerLayer ) {
         cout << " Basin_Error: formation '" << m_formations [ numberOfErrorsPerLayer.size () - i - 1 ]->getName () << "' has multiple errors, more than is indicated." << endl;
      }

   }


   const int errorFoundInt = ( errorFound ? 1 : 0 );
   int globalErrorFoundInt;

   getMaxValue ( errorFoundInt, globalErrorFoundInt );
   errorFound = ( globalErrorFoundInt == 1 );

   return not errorFound;
}

//------------------------------------------------------------//

void GeoPhysics::ProjectHandle::storePresentDayThickness ( const unsigned int i,
                                                           const unsigned int j,
                                                           GeoPhysics::GeoPhysicsFormation* formation )
{
   double presentDayThickness = 0.0;

   for ( unsigned int zCount = 0; zCount < formation->getMaximumNumberOfElements (); ++zCount )
   {
      const double segmentThickness = formation->getSolidThickness (i, j, zCount ).F (0.0);

      if ( segmentThickness != Interface::DefaultUndefinedScalarValue ) {
         presentDayThickness = presentDayThickness + segmentThickness;
      }

   }

   formation->m_presentDayErodedThickness ( i, j ) = presentDayThickness;
}

//------------------------------------------------------------//

bool GeoPhysics::ProjectHandle::computeThicknessHistories ( const unsigned int i,
                                                            const unsigned int j,
                                                                  GeoPhysics::GeoPhysicsFormation* formation,
                                                                  IntegerArray& numberOfErrorsPerLayer ) const
{

   if ( formation->isMobileLayer () or formation->kind () == Interface::BASEMENT_FORMATION ) {
      return setMobileLayerThicknessHistory ( i, j, formation, numberOfErrorsPerLayer );
   } else if ( formation->getIsIgneousIntrusion ()) {
      return setIgneousIntrusionThicknessHistory ( i, j, formation, numberOfErrorsPerLayer );
   } else {

      const double thickness = formation->getInputThicknessMap ()->getValue ( i, j );

      // If the thickness is positive (taking into account our minimum)
      if ( thickness > ThicknessTolerance ) {
         // depositing
         return setDepositionHistory ( i, j, thickness, formation );
      } else if ( thickness < -ThicknessTolerance ) {
         // otherwise we are eroding if it is negative
         return setHistoriesForUnconformity ( i, j, thickness, formation );
      } else {
         // the other else case here is when we consider the layer to be
         // already eroded at this point
         return true;
      }

   }

}

//------------------------------------------------------------//

bool GeoPhysics::ProjectHandle::setDepositionHistory ( const unsigned int i,
                                                       const unsigned int j,
                                                       const double thickness,
                                                       GeoPhysics::GeoPhysicsFormation* formation )
{

   const double segmentThickness = thickness / double ( formation->getMaximumNumberOfElements ());

   double startDepositionAge = formation->getBottomSurface ()->getSnapshot ()->getTime ();
   double endDepositionAge = formation->getTopSurface ()->getSnapshot ()->getTime ();

   // Notice the sign of elementDepositionDurations is -ve,
   // because we are counting from the top of the element stack.
   const double elementDepositionDuration = ( endDepositionAge - startDepositionAge ) / double ( formation->getMaximumNumberOfElements ());

   for ( unsigned int segment = 0; segment < formation->getMaximumNumberOfElements (); ++segment ) {

      formation->getSolidThickness ( i, j, segment ).AddPoint ( startDepositionAge, 0.0 );
      formation->getRealThickness  ( i, j, segment ).AddPoint ( startDepositionAge, 0.0 );

      endDepositionAge = startDepositionAge + elementDepositionDuration;

      formation->getSolidThickness ( i, j, segment ).AddPoint ( endDepositionAge, segmentThickness );
      formation->getRealThickness  ( i, j, segment ).AddPoint ( endDepositionAge, segmentThickness );

      startDepositionAge = endDepositionAge;
   }

   return true;
}

//------------------------------------------------------------//

bool GeoPhysics::ProjectHandle::setHistoriesForUnconformity ( const unsigned int i,
                                                              const unsigned int j,
                                                              const double       thickness,
                                                              GeoPhysicsFormation*         formation ) const
{

   const double endErosionAge   = formation->getTopSurface ()->getSnapshot ()->getTime ();
   double startErosionAge = formation->getBottomSurface ()->getSnapshot ()->getTime ();

   // calculate how much we have eroded
   double uncThickness = -thickness;

   bool result = true;

   GeoPhysics::GeoPhysicsFormation* currentFormation = formation;

   while ( result and uncThickness > ThicknessTolerance ) {

      currentFormation = const_cast<GeoPhysics::GeoPhysicsFormation*>(dynamic_cast<const GeoPhysics::GeoPhysicsFormation*>(currentFormation->getBottomSurface ()->getBottomFormation ()));

      if ( currentFormation->kind () == Interface::BASEMENT_FORMATION ) {
         // if we have reached the crust then exit with success
         return true;
      }

      if ( currentFormation->isMobileLayer ()) {

         if ( uncThickness <= MobileLayerNegativeThicknessTolerance ) {
            uncThickness = 0.0;
            continue;
         } else {
            cout << "Basin_Error: Erosion of mobile layer [" << currentFormation->getName () << "] at position (" << i << ", " << j << ") " << thickness <<  "  is not permitted " << endl;
            return false;
         }

      }

      if ( currentFormation->getIsIgneousIntrusion ()) {
         cout << "Basin_Error: Erosion of igneousIntrusion [" << currentFormation->getName () << "] is not permitted " << endl;
         return false;
      }

      if ( currentFormation->getInputThicknessMap ()->getValue ( i, j ) <= ThicknessTolerance ) {
         continue;
      }

      double layerThickness = 0.0;

      for ( unsigned int segment = 0; segment < currentFormation->getMaximumNumberOfElements (); ++segment ) {
         const double segmentThickness = currentFormation->getSolidThickness ( i, j, segment ).F ( 0.0 );

         if ( segmentThickness == Interface::DefaultUndefinedScalarValue ) {
            std::cout << "(setHistoriesForUnconformity) FullCompThickness not set on segment "
                      << currentFormation->getName () << "  " << i << "  " << j << "  " << segment << std::endl;
            return false;
         }

         layerThickness += segmentThickness;
      }

      assert ( layerThickness >= -ThicknessTolerance );

      // Layer is completely eroded
      if (layerThickness < ThicknessTolerance ) {
         continue;
      }

      const double erodedThickness = NumericFunctions::Minimum (layerThickness, uncThickness);

      const double localEndErosionAge = startErosionAge +
            (erodedThickness / uncThickness) * (endErosionAge - startErosionAge);

      result = setErosionHistory (i, j, currentFormation, startErosionAge, localEndErosionAge, erodedThickness);

      if (result)
      {
         // subtract this layers' thickness from the remaining unconf thickness
         uncThickness -= erodedThickness;
         // set the new start depo age at the end of the erosion of this layer
         //endErosionAge = Basin_Model -> layers[layerCnt]->depoage;
         startErosionAge = localEndErosionAge; //Basin_Model -> layers[layerCnt+1]->depoage;
      }

   }

   return result;
}

//------------------------------------------------------------//

bool GeoPhysics::ProjectHandle::setErosionHistory ( const unsigned int i,
                                                    const unsigned int j,
                                                    GeoPhysicsFormation* formation,
                                                    const double startErosionAge,
                                                    const double endErosionAge,
                                                    const double erodedThickness ) const
{

   static const double ThicknessEpsilon = 1.0e-8;

   double polyfThickness;

   double actualErodedThickness = erodedThickness;
   double segmentThicknessAfterErosion;
   double segmentStartErosionAge = startErosionAge;

   // we are going to iterate through the segments in this layer and remove
   // the erosion
   assert (0 != formation->getMaximumNumberOfElements ());

   for ( int segment = formation->getMaximumNumberOfElements () - 1; segment >= 0; --segment ) {

      // if we have dealt with all of the eroded thickness
      if ( actualErodedThickness <= ThicknessTolerance ) {
         break;
      }

      CBMGenerics::Polyfunction *pf = &(formation->getSolidThickness ( i, j, segment ));
      CBMGenerics::Polyfunction *realThicknessPf = &(formation->getRealThickness (i, j, segment ));

      pf->MinX (polyfThickness);
      assert (polyfThickness != Interface::DefaultUndefinedScalarValue );

      if (polyfThickness > 0)
      {
         const double segErosionThickness = NumericFunctions::Minimum (polyfThickness, actualErodedThickness );

         assert (0 != actualErodedThickness );
         const double segEndErosionAge = segmentStartErosionAge +
               (segErosionThickness / actualErodedThickness) * (endErosionAge - segmentStartErosionAge);

         pf->AddPoint ( segmentStartErosionAge, polyfThickness);
         realThicknessPf->AddPoint ( segmentStartErosionAge, polyfThickness);

         if ( std::fabs ( polyfThickness - segErosionThickness ) < ThicknessEpsilon ) {
            // If the difference between the two thickness is very small
            // then set the eroded thickness to be zero.
            segmentThicknessAfterErosion = 0.0;
         } else {
            // otherwise set the actual eroded thickness.
            segmentThicknessAfterErosion = polyfThickness - segErosionThickness;
         }

         pf->AddPoint (segEndErosionAge, segmentThicknessAfterErosion );
         realThicknessPf->AddPoint (segEndErosionAge, segmentThicknessAfterErosion);

         segmentStartErosionAge = segEndErosionAge;
         actualErodedThickness -= segErosionThickness;
      }

   }

   return true;
}

//------------------------------------------------------------//

bool GeoPhysics::ProjectHandle::setMobileLayerThicknessHistory ( const unsigned int i,
                                                                 const unsigned int j,
                                                                       GeoPhysics::GeoPhysicsFormation* formation,
                                                                       IntegerArray& numberOfErrorsPerLayer ) const
{

   if ( formation->isMantle ()) {
      return true;
   } else if ( formation->isCrust ()) {

      if ( getBottomBoundaryConditions () == Interface::FIXED_BASEMENT_TEMPERATURE || m_isALCMode) {
         Interface::PaleoFormationPropertyList* crustThicknesses = dynamic_cast<GeoPhysics::GeoPhysicsCrustFormation*>(formation)->getPaleoThicknessHistory ();

         double segmentThickness;

         for ( Interface::PaleoFormationPropertyList::const_iterator mapIter = crustThicknesses->begin (); mapIter != crustThicknesses->end (); ++mapIter ) {
            const double age = (*mapIter)->getSnapshot ()->getTime ();

            if( m_isALCMode ) {
               segmentThickness = getCrustThickness( i, j, age ) / double ( formation->getMaximumNumberOfElements ());
             } else {
               segmentThickness = (*mapIter)->getMap ( Interface::CrustThinningHistoryInstanceThicknessMap )->getValue ( i, j ) / double ( formation->getMaximumNumberOfElements ());
            }
            for ( unsigned int segment = 0; segment < formation->getMaximumNumberOfElements (); ++segment ) {
               formation->getSolidThickness ( i, j, segment ).AddPoint ( age, segmentThickness );
               formation->getRealThickness  ( i, j, segment ).AddPoint ( age, segmentThickness );
            }

         }

         delete crustThicknesses;
      } else {

         const double segmentThickness = formation->getInputThicknessMap ()->getValue ( i, j ) / double ( formation->getMaximumNumberOfElements ());

         for ( unsigned int segment = 0; segment < formation->getMaximumNumberOfElements (); ++segment ) {
            formation->getSolidThickness ( i, j, segment ).AddPoint ( 0.0, segmentThickness );
            formation->getRealThickness  ( i, j, segment ).AddPoint ( 0.0, segmentThickness );
         }

      }

      return true;
   } else {

      unsigned int segment;
      bool onlyPositiveThickness = true;

      double segmentThickness = formation->getInputThicknessMap ()->getValue ( i, j ) / double ( formation->getMaximumNumberOfElements ());

      if ( NumericFunctions::inRange<double>( segmentThickness, -MobileLayerNegativeThicknessTolerance, 0.0 )) {
        segmentThickness = 0.0;
      }

      if ( segmentThickness < 0.0 ) {
         onlyPositiveThickness = false;

         if ( formation->getDepositionSequence () > 0 and formation->getDepositionSequence () != Interface::DefaultUndefinedScalarValue ) {
            ++numberOfErrorsPerLayer [ formation->getDepositionSequence () - 1 ];

            if ( numberOfErrorsPerLayer [ formation->getDepositionSequence () - 1 ] <= MaximumNumberOfErrorsPerLayer ) {
               cout << " Basin_Error: negative mobile layer thickness detected in formation '" << formation->getName () << "' at position (" << i << ", " << j  << ").  " << formation->getInputThicknessMap ()->getValue ( i, j ) << endl;
            }

         }

      }

      for ( segment = 0; segment < formation->getMaximumNumberOfElements (); ++segment ) {
         formation->getSolidThickness ( i, j, segment ).AddPoint ( 0.0, segmentThickness );
         formation->getRealThickness  ( i, j, segment ).AddPoint ( 0.0, segmentThickness );
      }

      if ( formation->isMobileLayer ()) {
         Interface::MobileLayerList* mobileLayerThicknesses = formation->getMobileLayers ();

         for ( Interface::MobileLayerList::const_iterator mapIter = mobileLayerThicknesses->begin (); mapIter != mobileLayerThicknesses->end (); ++mapIter ) {
            const double age = (*mapIter)->getSnapshot ()->getTime ();

            if ( age != 0.0 ) {
               // The present day thicknesses are added above, the input-thickness map values.
               segmentThickness = (*mapIter)->getMap ( Interface::MobileLayerThicknessMap )->getValue ( i, j ) / double ( formation->getMaximumNumberOfElements ());

               for ( segment = 0; segment < formation->getMaximumNumberOfElements (); ++segment ) {
                  formation->getSolidThickness ( i, j, segment ).AddPoint ( age, segmentThickness );
                  formation->getRealThickness  ( i, j, segment ).AddPoint ( age, segmentThickness );
               }

            }

         }

         delete mobileLayerThicknesses;
      }

      return onlyPositiveThickness;
   }

}

//------------------------------------------------------------//

bool GeoPhysics::ProjectHandle::setIgneousIntrusionThicknessHistory ( const unsigned int i,
                                                                      const unsigned int j,
                                                                      GeoPhysics::GeoPhysicsFormation* formation,
                                                                      IntegerArray& numberOfErrorsPerLayer )
{

   //
   //
   //
   //
   //
   //
   //
   //
   //
   //
   //
   //
   //
   //                                             ^
   //                    +==================+     |
   //                   /                         |
   //                   |                         |
   //                   |                         | thickness
   //                   |                         |
   //                  /                          |
   //  +===============+-+-----------------+------+ 0
   //  t0             ts te               tp
   //
   //  t0 age of surrounding material
   //  ts start of inflation of the igneous intrusion
   //  te end of inflation of the igneous intrusion
   //  tp present day
   //
   // The difference between ts and te (duration of inflation = ts - te) is defined by IgneousIntrusionEventDuration
   // this can be found in DataAccess/src/interface/h
   //

   if ( not formation->getIsIgneousIntrusion ()) {
      // If the formation is not an igneous intrusion then should not be in this function.
      return false;
   }

   bool onlyPositiveThickness = true;

   unsigned int segment;

   const double segmentThickness = formation->getInputThicknessMap ()->getValue ( i, j ) / double ( formation->getMaximumNumberOfElements ());

   if ( segmentThickness < 0.0 ) {
      onlyPositiveThickness = false;

      if ( formation->getDepositionSequence () > 0 and formation->getDepositionSequence () != Interface::DefaultUndefinedScalarValue ) {
         ++numberOfErrorsPerLayer [ formation->getDepositionSequence () - 1 ];

         if ( numberOfErrorsPerLayer [ formation->getDepositionSequence () - 1 ] <= MaximumNumberOfErrorsPerLayer ) {
            cout << " Basin_Error: negative igneous intrusion thickness detected in formation '" << formation->getName () << "' at position (" << i << ", " << j  << ")." << endl;
         }

      }

   }

   const Interface::IgneousIntrusionEvent* intrusionEvent = formation->getIgneousIntrusionEvent ();

   const double endOfIntrusionAge = intrusionEvent->getEndOfIntrusion ()->getTime ();
   const double startOfIntrusionAge = intrusionEvent->getStartOfIntrusion ();
   const double formationInclusionAge = formation->getTopSurface ()->getSnapshot ()->getTime ();

   // Set present day thicknesses.
   for ( segment = 0; segment < formation->getMaximumNumberOfElements (); ++segment ) {
      formation->getSolidThickness ( i, j, segment ).AddPoint ( 0.0, segmentThickness );
      formation->getRealThickness  ( i, j, segment ).AddPoint ( 0.0, segmentThickness );
   }

   // Set thickness at end of intrusion event age.
   // If this is the same as present day then there is no need to add it because it has been added already.
   if ( endOfIntrusionAge != 0.0 ) {

      for ( segment = 0; segment < formation->getMaximumNumberOfElements (); ++segment ) {
         formation->getSolidThickness ( i, j, segment ).AddPoint ( endOfIntrusionAge, segmentThickness );
         formation->getRealThickness  ( i, j, segment ).AddPoint ( endOfIntrusionAge, segmentThickness );
      }

   }

   for ( segment = 0; segment < formation->getMaximumNumberOfElements (); ++segment ) {
      formation->getSolidThickness ( i, j, segment ).AddPoint ( startOfIntrusionAge, 0.0 );
      formation->getRealThickness  ( i, j, segment ).AddPoint ( startOfIntrusionAge, 0.0 );
   }


   // set the thickness at the top surface age.
   // This is the age the surrounding formation appears in the strat table.
   for ( segment = 0; segment < formation->getMaximumNumberOfElements (); ++segment ) {
      formation->getSolidThickness ( i, j, segment ).AddPoint ( formationInclusionAge, 0.0 );
      formation->getRealThickness  ( i, j, segment ).AddPoint ( formationInclusionAge, 0.0 );
   }


   return onlyPositiveThickness;
}

//------------------------------------------------------------//

double GeoPhysics::ProjectHandle::getSeaBottomDepth ( const unsigned int i,
                                                      const unsigned int j,
                                                      const double       age ) const {
   return m_seaBottomDepth ( i, j ).F ( age );
}

//------------------------------------------------------------//

double GeoPhysics::ProjectHandle::getSeaBottomTemperature ( const unsigned int i,
                                                            const unsigned int j,
                                                            const double       age ) const {
   return m_seaBottomTemperature ( i, j ).F ( age );
}

//------------------------------------------------------------//

double GeoPhysics::ProjectHandle::getMantleHeatFlow ( const unsigned int i,
                                                      const unsigned int j,
                                                      const double       age ) const {
   /// Return -mhf because the up-direction is negative (in the cauldron model, i.e. depth is positive).
   return -m_mantleHeatFlow ( i, j ).F ( age );
}

//------------------------------------------------------------//

double GeoPhysics::ProjectHandle::getEndOfRiftEvent ( const unsigned int i,
                                                      const unsigned int j ) const {
   return m_endOfRiftEvent ( i, j );
}
//------------------------------------------------------------//

double GeoPhysics::ProjectHandle::getCrustThickness ( const unsigned int i,
                                                      const unsigned int j,
                                                      const double       age ) const {
   return m_crustThicknessHistory ( i, j ).F ( age );
}
//------------------------------------------------------------//

double GeoPhysics::ProjectHandle::getContCrustThickness ( const unsigned int i,
                                                          const unsigned int j,
                                                          const double       age ) const {
  if( m_isALCMode ) {
     return m_contCrustThicknessHistory ( i, j ).F ( age );
  }
  return 0.0;
}
//------------------------------------------------------------//

double GeoPhysics::ProjectHandle::getBasaltThickness ( const unsigned int i,
                                                       const unsigned int j,
                                                       const double       age ) const {
   if( m_isALCMode ) {
      return m_basaltThicknessHistory ( i, j ).F ( age );
   }
   return 0.0;
}
//------------------------------------------------------------//

double GeoPhysics::ProjectHandle::getLithosphereThicknessMod ( const unsigned int i,
                                                               const unsigned int j,
                                                               const double       age ) const {
   if( m_isALCMode ) {
      const double thinningFactor = 1.0 - getContCrustThickness( i, j, age ) / getCrustFormation()->getInitialCrustalThickness();
      const double initLithoThickness = getMantleFormation ()->getInitialLithosphericMantleThickness () +
                                  getCrustFormation()->getInitialCrustalThickness();

      const double HLmod = 0.5 * ( ( initLithoThickness + m_equilibriumOceanicLithosphereThickness ) + ( initLithoThickness - m_equilibriumOceanicLithosphereThickness ) * cos ( M_PI * thinningFactor ));
      return HLmod;
   }
   return 0.0;
}
//------------------------------------------------------------//

double GeoPhysics::ProjectHandle::getBasaltInMantleThickness ( const unsigned int i,
                                                               const unsigned int j,
                                                               const double       age ) const {
   if( m_isALCMode ) {
      return m_contCrustThicknessHistory( i, j ).F( age ) + m_basaltThicknessHistory ( i, j ).F ( age ) - m_crustThicknessHistory( i, j ).F( age ) ;
   }
   return 0.0;
}
//------------------------------------------------------------//

bool GeoPhysics::ProjectHandle::compFCThicknessHistories ( const unsigned int i,
                                                           const unsigned int j,
                                                           const bool overpressureCalculation,
                                                           GeoPhysics::GeoPhysicsFormation* formation,
                                                           int& nrActUnc,
                                                           FloatStack &uncMaxVes,
                                                           FloatStack &uncThickness )
{

   // we have 3 possible types of layer: 1: a mobile layer, 2: a normal
   // sedimented layer or 3: an erosion (layer).

   if ( formation->isMobileLayer () or formation->getIsIgneousIntrusion ()) {
      // mobile layer or igneous intrusion!!
      return updateMobileLayerOrIgneousIntrusionMaxVes (i, j, formation, uncMaxVes.front ());
   } else  {
      // get the thickness of the layer at this point to
      // determine whether the layer is depositing or eroding.
      // for a non mobile layer there will only be one thickness

      const double thickness = formation->getInputThicknessMap ()->getValue ( i, j );

      if ( thickness > ThicknessTolerance ) {
         // Compact this layer
         return compactLayerThicknessHistory (i, j, overpressureCalculation, formation, uncMaxVes, uncThickness, nrActUnc);
      } else if (thickness < -ThicknessTolerance ) {

         // the layer we are processing is an erosion layer and
         // therefore we have no fct thickness functions for the layer

         // Put the erosion on the stack, necessary to compute maxVes
         nrActUnc++;
         uncMaxVes.push_front (0.0);
         uncThickness.push_front (-thickness);
      }
   }

   return false;
}

//------------------------------------------------------------//

bool GeoPhysics::ProjectHandle::updateMobileLayerOrIgneousIntrusionMaxVes ( const unsigned int i,
                                                                            const unsigned int j,
                                                                            GeoPhysics::GeoPhysicsFormation* formation,
                                                                            double &maxVes )
{
   const bool result = true;
   double dummy;
   double diffdensity;

   const CompoundLithology* lithology = formation->getCompoundLithology ( i, j );
   const auto* fluid = dynamic_cast<const GeoPhysics::FluidType*>( formation->getFluidType ());

   for ( int segment = formation->getMaximumNumberOfElements() - 1; segment >= 0; --segment )
   {
      const double segmentThickness = formation->getSolidThickness ( i, j, static_cast<unsigned int>(segment)).MaxY (dummy);
      assert( segmentThickness != Interface::DefaultUndefinedScalarValue );

      if ( fluid != nullptr  )
      {
         diffdensity = lithology->density () - fluid->getConstantDensity ();

         const bool switchPermaFrost = fluid->isPermafrostEnabled();
         const double surfacePorosity = lithology->surfacePorosity( );

         // Fluid is denser than rock and the permafrost switch is on
         if ( diffdensity <= 0 && switchPermaFrost )
         {
            maxVes += AccelerationDueToGravity *  lithology->density() * segmentThickness * ( 1.0 - surfacePorosity );
            return result;
         }
         else if ( diffdensity <= 0 ) diffdensity = 0.0;
      }
      else
      {
         diffdensity = lithology->density ();
      }

      maxVes += AccelerationDueToGravity * diffdensity * segmentThickness;
    }

  return result;
}

//------------------------------------------------------------//

bool GeoPhysics::ProjectHandle::compactLayerThicknessHistory ( const unsigned int i,
                                                               const unsigned int j,
                                                               const bool overpressureCalculation,
                                                               GeoPhysics::GeoPhysicsFormation* formation,
                                                               FloatStack &uncMaxVes,
                                                               FloatStack &uncThickness,
                                                               const int nrActUnc )
{

// Simple deposition                                                    ^
//               =======================================================| depth
//              /                                                       |
//             /                                                        |
//            /                                                         |
//           /                                                          |
//          /                                                           |
//         /                                                            |
// t <=====-------------------------------------------------------------
//        t0    t1

// Deposition followed by total erosion                                 ^
//               ================                                       | depth
//              /                \                                      |
//             /                  \                                     |
//            /                    \                                    |
//           /                      \                                   |
//          /                        \                                  |
//         /                          \                                 |
// t <=====----------------------------=================================
//        t0    t1             t2      t3

// Deposisiton followed by partial erosion                              ^
//               ================                                       | depth
//              /                \                                      |
//             /                  \                                     |
//            /                    =====================================|
//           /                                                          |
//          /                                                           |
//         /                                                            |
// t <=====-------------------------------------------------------------
//        t0    t1                       t2 t3

// Deposition followed by 2 (or more) partial erosions but not totally  ^
// eroding       ================                                       | depth
//              /                \                                      |
//             /                  \                                     |
//            /                    ================                     |
//           /                                     \                    |
//          /                                       ====================|
//         /                                                            |
// t <=====-------------------------------------------------------------
//        t0    t1             t2 t3             t4 t5

// Deposition followed by 2 (or more) partial erosions ending in        ^
// total         ================                                       | depth
// erosion      /                \                                      |
//             /                  \                                     |
//            /                    ================                     |
//           /                                     \                    |
//          /                                       =======             |
//         /                                               \            |
// t <=====-------------------------------------------------============
//        t0    t1             t2 t3             t4 t5   t6 t7

// Deposition followed by immediate total erosion                       ^
//               ^                                                      | depth
//              / \           This is important when setting            |
//             /   \          start of erosion and end of               |
//            /     \         erosion                                   |
//           /       \                                                  |
//          /         \                                                 |
//         /           \                                                |
// t <=====----------------------------=================================
//        t0    t1             t2      t3


   bool result = true;
   double compThickness;
   double fullCompThickness = 0.;
   double totalFCT = 0.;
   double startDepoTime = 0.0, endDepoTime = 1.e10;

   int numberOfUnconformities = nrActUnc;

   // loop through all the segments in this layer

   for (int segmentNr = formation->getMaximumNumberOfElements () - 1; segmentNr >= 0; segmentNr--)
   {

      // create an empty polynomial function
      auto *oldPolyf = new Polyfunction;
      CBMGenerics::Polyfunction *newPolyf = &(formation->getSolidThickness (i, j, segmentNr));

      // set the old polynomial function up and clear the new one
      (*newPolyf).swap (*oldPolyf);

      // set an iterator to point to the oldest point in the function
      auto oldPolyfIter = oldPolyf->getRBegin ();

      if (oldPolyf->getREnd () == oldPolyfIter)
      {
         cout << "poly function is empty" << endl;
      }

      // save the start of deposition point as we will add this
      // point to the function last.
      const auto startOfDeposition = oldPolyfIter;

      assert ((*startOfDeposition)->getY () == 0);

      double thisStartDepoTime = (*startOfDeposition)->getX ();

      startDepoTime = NumericFunctions::Maximum (startDepoTime, thisStartDepoTime);
      auto endOfEvent = ++oldPolyfIter;
      auto endOfPrevEvent = endOfEvent;
      double thisEndDepoTime = (*endOfEvent)->getX ();

      endDepoTime = NumericFunctions::Minimum (endDepoTime, thisEndDepoTime);

      while (true)
      {

         // move to the next point
         auto startOfErosion = ++oldPolyfIter;
         if (startOfErosion == oldPolyf->getREnd ())
         {
            // exit the loop if we have got to the end of the function
            break;
         }

         endOfPrevEvent = endOfEvent;
         if ((*endOfPrevEvent)->getY () == (*startOfErosion)->getY ())
         {
            endOfEvent = ++oldPolyfIter;
       if (endOfEvent == oldPolyf->getREnd ())
       {
          // There is a start of erosion but not and end of it.
          // exit the loop if we have got to the end of the function
          break;
       }
            assert ((*endOfEvent)->getX () <= (*startOfErosion)->getX ());
         }
         else
         {
            // if this is a deposition followed immediately by an erosion or
            // an erosion followed immediately by another erosion then we need
            // to set the end of event and start of erosion to the same time
            // note that the poly function will fail to add the second point.
            endOfEvent = startOfErosion;
            startOfErosion = endOfPrevEvent;
         }

         newPolyf->AddPoint ((*endOfPrevEvent)->getX (), 0.0);
         newPolyf->AddPoint ((*startOfErosion)->getX (), 0.0);

         compThickness = (*startOfErosion)->getY () - (*endOfEvent)->getY ();

         // if the end of event has iterated past the end of the polyfunction then
         // terminate the loop (we will not need to raise the function in this case)
         if (oldPolyf->getREnd () == endOfEvent)
            break;

         //cout << "calcFullCompactedThickness for " << Basin_Model -> layers[layerNr]->layername
         //   << "[" << segmentNr << "]" << endl;
         result &= calcFullCompactedThickness ( i, j, overpressureCalculation, formation, compThickness, uncMaxVes.front (), fullCompThickness, (*endOfEvent)->getX () );

         if (!result)
         {
            cout << "Basin_Warning: Could not compute FCT" << endl;
            delete oldPolyf;
            return false;
         }

         totalFCT += fullCompThickness;

         // we are adding points at y=0 and then raising the whole function
         newPolyf->RaiseBy (fullCompThickness);

         // compensate for the unconformity.
         double remThickness = compThickness;

         while (remThickness > 0 && uncMaxVes.size () > 0)
         {
            const double tmpThickness = NumericFunctions::Minimum (uncThickness.front (), remThickness);

            uncThickness.front () -= tmpThickness;
            remThickness -= tmpThickness;

            if ( uncThickness.front () < ThicknessTolerance )
            {
               numberOfUnconformities--;
               double currentMaxVes = uncMaxVes.front ();

               uncMaxVes.pop_front ();
               uncThickness.pop_front ();
               uncMaxVes.front () = NumericFunctions::Maximum (uncMaxVes.front (), currentMaxVes);
            }
         }
      }

      // add the start and end of the original deposition
      newPolyf->AddPoint ((*endOfEvent)->getX (), 0.0);

      compThickness = (*endOfEvent)->getY ();
      result &= calcFullCompactedThickness (i, j, overpressureCalculation, formation, compThickness, uncMaxVes.front (), fullCompThickness, (*endOfEvent)->getX() );
      if (!result)
      {
         cout << "Basin_Warning: Could not compute FCT (2)" << endl;
         delete oldPolyf;
         return false;
      }

      totalFCT += fullCompThickness;

      newPolyf->RaiseBy (fullCompThickness);
      newPolyf->AddPoint ((*startOfDeposition)->getX (), 0.0);

      delete oldPolyf;
   }

   // Start at the bottom of the layer and iterate through
   // the segments to the top.
   //
   // At each iteration we will modify the time axis of the FCT
   // polynomial function so that the deposition (not erosion) period of the
   // segment is proportional to the FCT of the deposited material.
   //
   // on entry to this loop the totalFCT will be that for the complete layer
   // startDepoTime will be the start time of deposition of the complete layer
   //
   for (int segmentNr = formation->getMaximumNumberOfElements () - 1; segmentNr >= 0; segmentNr--)
   {
      Polyfunction *polyf = &(formation->getSolidThickness (i, j, segmentNr));
      auto polyfit = polyf->getRBegin ();

      const auto startDepoPoint = polyfit;
      const auto endDepoPoint = ++polyfit;

      const double segmFCT = (*endDepoPoint)->getY ();

      assert (0 == (*startDepoPoint)->getY ());
      assert (0 != totalFCT);

      const double segmStartDepoTime = endDepoTime - (endDepoTime - startDepoTime) * (segmFCT / totalFCT);

      (*endDepoPoint)->setX (endDepoTime);
      (*startDepoPoint)->setX (segmStartDepoTime);

      endDepoTime = segmStartDepoTime;
      totalFCT -= segmFCT;
   }

   return result;
}

//------------------------------------------------------------//

bool GeoPhysics::ProjectHandle::calcFullCompactedThickness ( const unsigned int i,
   const unsigned int j,
   const bool     overpressureCalculation,
   GeoPhysics::GeoPhysicsFormation* formation,
   const double compThickness,
   double &uncMaxVes,
   double &fullCompThickness,
   const double age )
{

   const bool result = true;

   const CompoundLithology* lithology = formation->getCompoundLithologyArray()( i, j, age );
   const auto* fluid = dynamic_cast<const GeoPhysics::FluidType*>( formation->getFluidType() );

   const double lithologyDensity = lithology->density();
   double densityDifference = 0.0;

   if ( fluid != nullptr )
   {
      const double fluidDensity = fluid->getConstantDensity( );
      if ( lithologyDensity > fluidDensity ) densityDifference = lithologyDensity - fluidDensity;
   }
   else
   {
      densityDifference = lithologyDensity;
   }

  fullCompThickness = lithology->hydrostatFullCompThickness ( uncMaxVes,
                                                              compThickness,
                                                              densityDifference,
                                                              overpressureCalculation );

  uncMaxVes += AccelerationDueToGravity * densityDifference * fullCompThickness;

  return result;
}

//------------------------------------------------------------/it

bool GeoPhysics::ProjectHandle::applyFctCorrections () const
{

   const Interface::Property* fctCorrectionProperty = findProperty ( "FCTCorrection" );
   assert ( fctCorrectionProperty != nullptr );

   const Interface::Snapshot* presentDataSnapshot = findSnapshot ( 0.0, Interface::MAJOR );
   assert ( presentDataSnapshot != nullptr );

   // Get the FCTCorrectionproperty from the results file.
   Interface::PropertyValueList* solidThicknessCorrections = getPropertyValues ( Interface::FORMATION,
                                                                                 fctCorrectionProperty,
                                                                                 presentDataSnapshot,
                                                                                 nullptr, nullptr, nullptr,
                                                                                 Interface::MAP );

   for ( Interface::PropertyValueList::const_iterator fctIter = solidThicknessCorrections->begin (); fctIter != solidThicknessCorrections->end (); ++fctIter ) {

      const Interface::PropertyValue* fct = *fctIter;

      GeoPhysics::GeoPhysicsFormation* formation = const_cast<GeoPhysics::GeoPhysicsFormation*>(dynamic_cast<const GeoPhysics::GeoPhysicsFormation*>( fct->getFormation ()));

      if ( formation->kind () == Interface::SEDIMENT_FORMATION ) {
         const Interface::GridMap* fctMap = dynamic_cast<const Interface::GridMap*>(fct->getGridMap ());

         fctMap->retrieveGhostedData ();

         const unsigned int numberOfElements = formation->getMaximumNumberOfElements ();

         for ( unsigned int i = firstI ( true ); i <= lastI ( true ); ++i ) {

            for ( unsigned int j = firstJ ( true ); j <= lastJ ( true ); ++j ) {

               for ( unsigned int k = 0; k < numberOfElements; ++k ) {
                  formation->getSolidThickness ( i, j, k ).ScaleBy ( fctMap->getValue ( i, j ));
               }

            }

         }

         fctMap->restoreData ( false, true );
         fctMap->release();
      }
   }

   delete solidThicknessCorrections;

   return true;
}

//------------------------------------------------------------//

void GeoPhysics::ProjectHandle::printValidNeedles ( std::ostream& o  ) const
{
   std::ostringstream msg;
   msg << endl;
   msg << endl;
   msg << "--------------------------------" << endl;
   msg << endl;
   msg << endl;


   for ( int proc = 0; proc < getSize (); ++proc ) {

      if ( proc == getRank () ) {
         msg << " process " << proc << endl;

         for ( unsigned int i = firstI ( true ); i <= lastI ( true ); ++i ) {

            for ( unsigned int j = firstJ ( true ); j <= lastJ ( true ); ++j ) {
               msg << (m_validator.isValid( i, j ) ? 'T' : 'F' );
            }

            msg << endl;
         }

         o << msg.str();
         o << flush;
      }

      barrier ();
      getMessageHandler ().flush ();
   }

}

//------------------------------------------------------------//
bool GeoPhysics::ProjectHandle::loadALCConfigurationFile(const string & cfgFileName) {

   m_basementLithoProps = nullptr;
   if( m_isALCMode ) {
      char * alcConfigurationFile     = getenv ( "CTCDIR" );
      char * alcUserConfigurationFile = getenv ( "MY_CTCDIR" );

      string fullpath;

      if( alcUserConfigurationFile != nullptr ) {
         ibs::Path fp( alcUserConfigurationFile );
         fp << cfgFileName;
         fullpath = fp.path();
      } else if( alcConfigurationFile != nullptr ) {
         ibs::FilePath fp( alcConfigurationFile );
         fp << cfgFileName;
         fullpath = fp.path();
      } else {
         std::cout<< "Basin_Warning: Environment Variable CTCDIR is not set." << std::endl;;
         return false;
      }

      ifstream  configurationFile;
      configurationFile.open( fullpath );

      if (!configurationFile) {
         getMessageHandler().printLine( "Basin_Error: Attempting to open file : " + fullpath + "\nNo cfg file available in the $CTCDIR directory... Aborting..." );
         throw RecordException( "Basin_Error: Attempting to open file : " + fullpath + "\nNo cfg file available in the $CTCDIR directory... Aborting..." );
      }

      m_basementLithoProps = new ConfigFileParameterAlc();
      if( !m_basementLithoProps->loadConfigurationFileAlc ( configurationFile )) {
         configurationFile.close();
         return false;
      };

      m_constrainedBasaltTemperature  = m_basementLithoProps->m_bT;

      configurationFile.close();
      return true;
   }
   return true;
}

//------------------------------------------------------------//
bool GeoPhysics::ProjectHandle::hasSurfaceDepthHistory( const double age ) const{
   for (auto i : m_surfaceDepthHistory){
      if (i->getSnapshot()->getTime() == age){
         return true;
      }
   }
   return false;
}
