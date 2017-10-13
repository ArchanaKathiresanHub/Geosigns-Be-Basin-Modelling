//
// Copyright (C) 2015-2017 Shell International Exploration & Production.
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
#include <exception>
#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif
#include <cmath>

// DataAccess library
#include "Interface/Interface.h"
#include "Interface/AllochthonousLithology.h"
#include "Interface/AllochthonousLithologyDistribution.h"
#include "Interface/CrustFormation.h"
#include "Interface/Formation.h"
#include "Interface/GridMap.h"
#include "Interface/IgneousIntrusionEvent.h"
#include "Interface/MantleFormation.h"
#include "Interface/MobileLayer.h"
#include "Interface/OceanicCrustThicknessHistoryData.h"
#include "Interface/PaleoFormationProperty.h"
#include "Interface/PaleoSurfaceProperty.h"
#include "Interface/PaleoProperty.h"
#include "Interface/ProjectHandle.h"
#include "Interface/Property.h"
#include "Interface/PropertyValue.h"
#include "Interface/Reservoir.h"
#include "Interface/RunParameters.h"
#include "Interface/Snapshot.h"
#include "Interface/Surface.h"
#include "Interface/SourceRock.h"

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
#include "Validator.h"

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

GeoPhysics::ProjectHandle::ProjectHandle ( database::ProjectFileHandlerPtr pfh, const std::string & name, const std::string & accessMode, DataAccess::Interface::ObjectFactory* objectFactory ) :
   DataAccess::Interface::ProjectHandle ( pfh, name, accessMode, objectFactory ) {

   m_lithologyManager = new LithologyManager ( this );

   m_allochthonousLithologyManager = new AllochthonousLithologyManager ( this );

   m_isALCMode = ( getBottomBoundaryConditions () == Interface::ADVANCED_LITHOSPHERE_CALCULATOR );

   m_minimumLithosphereThickness = 100000;
   m_maximumNumberOfMantleElements = 100;
   m_constrainedBasaltTemperature = 1000;

   if( not loadALCConfigurationFile( "InterfaceData.cfg" ) ) {
      LogHandler( LogHandler::WARNING_SEVERITY ) << "Can't load ALC configuration file. Default values will be used";
   } 

   // Now load anything that was not loaded by default in the constructor of the default project handle.
   loadFaults ();
   loadConstrainedOverpressureIntervals ();

   bool status = loadCrustThinningHistory ();

   if( !status && m_isALCMode ) {
      throw RecordException ( "Error: The Crust thinning history table is empty!" );
   }

   loadMantleThicknessHistory ();
   correctCrustThicknessHistory ();
   loadHeatFlowHistory ();

   // Construct the fluid-property tables for each fluid.
   loadFluidPropertyTables ();

   // Now apply a correction to the constant density for "standard
   // pressure and temperature" of hydrostatic calculation mode.
   correctSimpleFluidDensities ();

   correctThermalConductivityTables ();
   addSimpleLithologiesToLithologyManager ();

   loadFracturePressureCalculator ();
#if 0
   if( getRunParameters ()->getHydrodynamicCapillaryPressure() ) {
     computeHCTables();
   }
#endif

}
//------------------------------------------------------------//

bool GeoPhysics::ProjectHandle::correctCrustThicknessHistory () {

   if( getBottomBoundaryConditions () == Interface::MANTLE_HEAT_FLOW ) {
      return true;
   }

   if ( m_crustPaleoThicknesses.size () == 1 ) {
      // Should check for bottom BCs.
      // No correction to the thickness history is necessary.
      return true;
   }

   const Interface::Snapshot* firstSimulationSnapshot = m_crustFormation->getTopSurface ()->getSnapshot ();
   assert ( firstSimulationSnapshot != 0 );

   size_t i;

   for ( i = 0; i < m_crustPaleoThicknesses.size (); ++i ) {

      if ( m_crustPaleoThicknesses [ i ]->getSnapshot () == firstSimulationSnapshot ) {
         // No correction to the thickness history is necessary.
         return true;
      }

   }

   const Interface::PaleoFormationProperty* beforeSimulation = 0;
   const Interface::PaleoFormationProperty* afterSimulation = 0;

   for ( i = 0; i < m_crustPaleoThicknesses.size (); ++i ) {

      if ( m_crustPaleoThicknesses [ i ]->getSnapshot ()->getTime () > firstSimulationSnapshot->getTime ()) {

         if ( beforeSimulation == 0 or beforeSimulation->getSnapshot ()->getTime () > m_crustPaleoThicknesses [ i ]->getSnapshot ()->getTime ()) {
            beforeSimulation = m_crustPaleoThicknesses [ i ];
         }

      }

      if ( m_crustPaleoThicknesses [ i ]->getSnapshot ()->getTime () < firstSimulationSnapshot->getTime ()) {

         if ( afterSimulation == 0 or afterSimulation->getSnapshot ()->getTime () < m_crustPaleoThicknesses [ i ]->getSnapshot ()->getTime ()) {
            afterSimulation = m_crustPaleoThicknesses [ i ];
         }

      }

   }

   assert ( beforeSimulation != 0 or afterSimulation != 0 );

   if ( beforeSimulation != 0 and afterSimulation != 0 ) {

      double beforeTime = beforeSimulation->getSnapshot ()->getTime ();
      double afterTime  = afterSimulation->getSnapshot ()->getTime ();
      double firstTime  = firstSimulationSnapshot->getTime ();

      m_crustPaleoThicknesses.push_back (getFactory ()->producePaleoFormationProperty ( this, m_crustFormation, beforeSimulation, afterSimulation, firstSimulationSnapshot ));

      m_mantlePaleoThicknesses.push_back (getFactory ()->producePaleoFormationProperty ( this, m_crustFormation, beforeSimulation, afterSimulation, firstSimulationSnapshot ));

      std::sort ( m_crustPaleoThicknesses.begin (), m_crustPaleoThicknesses.end (), Interface::PaleoPropertyTimeLessThan ());
      std::sort ( m_mantlePaleoThicknesses.begin (), m_mantlePaleoThicknesses.end (), Interface::PaleoPropertyTimeLessThan ());
   } else if ( beforeSimulation == 0 and afterSimulation != 0 ) {
   } else {
   }

   return true;
}


//------------------------------------------------------------//

GeoPhysics::ProjectHandle::~ProjectHandle () {
   delete m_lithologyManager;
   delete m_allochthonousLithologyManager;

   if( m_basementLithoProps != 0 ) {
      delete m_basementLithoProps;
   }

   deleteFracturePressureCalculator ();

}

//------------------------------------------------------------//

void GeoPhysics::ProjectHandle::loadFracturePressureCalculator () {
   m_fracturePressureCalculator = dynamic_cast<GeoPhysics::ObjectFactory*>( getFactory ())->produceFracturePressureCalculator ( this );
}

//------------------------------------------------------------//

void GeoPhysics::ProjectHandle::deleteFracturePressureCalculator () {
   delete m_fracturePressureCalculator;
}

//------------------------------------------------------------//
GeoPhysics::ConfigFileParameterAlc * GeoPhysics::ProjectHandle::getBasementLithologyProps() const {
   return m_basementLithoProps;
}

//------------------------------------------------------------//

bool GeoPhysics::ProjectHandle::startActivity ( const std::string& name, const DataAccess::Interface::Grid* grid, bool saveAsInputGrid, bool createResultsFile, bool append ) {

   bool started;

   started = DataAccess::Interface::ProjectHandle::startActivity ( name, grid, saveAsInputGrid, createResultsFile, append );

   if ( started ) {
      // Now get the boundaries of the activity grid for this process.
      const Interface::Grid* activityGrid = getActivityOutputGrid ();

      m_firstI [ 0 ] = (unsigned int)activityGrid->firstI ( false );
      m_firstI [ 1 ] = (unsigned int)activityGrid->firstI ( true );

      m_firstJ [ 0 ] = (unsigned int)activityGrid->firstJ ( false );
      m_firstJ [ 1 ] = (unsigned int)activityGrid->firstJ ( true );

      m_lastI  [ 0 ] = (unsigned int)activityGrid->lastI ( false );
      m_lastI  [ 1 ] = (unsigned int)activityGrid->lastI ( true );

      m_lastJ  [ 0 ] = (unsigned int)activityGrid->lastJ ( false );
      m_lastJ  [ 1 ] = (unsigned int)activityGrid->lastJ ( true );

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
         GeoPhysics::Formation* formation = dynamic_cast<GeoPhysics::Formation*>(*formationIter);

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
            GeoPhysics::Formation* formation = dynamic_cast<GeoPhysics::Formation*>(*formationIter);

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

   Interface::MutableFluidTypeList::iterator fluidIter;

   try
   {
      for ( fluidIter = m_fluidTypes.begin(); fluidIter != m_fluidTypes.end (); ++fluidIter )
      {
         GeoPhysics::FluidType* fluid = (GeoPhysics::FluidType*)(*fluidIter);
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

  Interface::MutableLithoTypeList::iterator simpleLithologyIter;
  double correctionFactor;
  double stdThCondVal;

  for ( simpleLithologyIter = m_lithoTypes.begin(); simpleLithologyIter != m_lithoTypes.end (); ++simpleLithologyIter ) {

     if( (*simpleLithologyIter)->getName() == DataAccess::Interface::ALCBasalt ) {
        continue;
     }
     SimpleLithology* simpleLithology = dynamic_cast<SimpleLithology*>(*simpleLithologyIter);

     stdThCondVal = simpleLithology->getThCondVal();
     correctionFactor = stdThCondVal / simpleLithology->thermalconductivity ( GeoPhysics::FluidType::DefaultThermalConductivityCorrectionTemperature );
     simpleLithology->correctThermCondPoint ( correctionFactor );

  }

}

//------------------------------------------------------------//

void GeoPhysics::ProjectHandle::addSimpleLithologiesToLithologyManager () {

   Interface::MutableLithoTypeList::iterator lithoIter;


   for ( lithoIter = m_lithoTypes.begin (); lithoIter != m_lithoTypes.end (); ++lithoIter ) {

      Interface::LithoType* lithoType = *lithoIter;
      if( m_isALCMode && ( lithoType->getName() == DataAccess::Interface::CrustLithologyName || lithoType->getName() == DataAccess::Interface::MantleLithologyName || lithoType->getName() == DataAccess::Interface::ALCBasalt )) {
         BasementLithology * litho = dynamic_cast<BasementLithology*>( lithoType );
         assert( litho );
         litho->setBasementLithoProperties( * m_basementLithoProps );
         getLithologyManager ().addSimpleLithology ( litho );
      } else {
         SimpleLithology* litho = (SimpleLithology*)( lithoType );
         getLithologyManager ().addSimpleLithology ( litho );
      }
   }

}

//------------------------------------------------------------//

void GeoPhysics::ProjectHandle::correctSimpleFluidDensities () {

   Interface::MutableFluidTypeList::iterator fluidIter;

   // The temperature gradient in the project file is in C/Km and not C/m and so needs to be converted.
   const double temperatureGradient = 0.001 * getRunParameters ()->getTemperatureGradient ();

   for ( fluidIter = m_fluidTypes.begin(); fluidIter != m_fluidTypes.end (); ++fluidIter ) {
      GeoPhysics::FluidType* fluid = (GeoPhysics::FluidType*)(*fluidIter);

      fluid->correctSimpleDensity ( GeoPhysics::FluidType::DefaultStandardDepth,
                                    GeoPhysics::FluidType::DefaultHydrostaticPressureGradient,
                                    GeoPhysics::FluidType::StandardSurfaceTemperature,
                                    temperatureGradient );
   }

}

//------------------------------------------------------------//

// const GeoPhysics::CompoundLithology* GeoPhysics::ProjectHandle::getCompoundLithology ( const CompoundLithologyComposition& composition ) {
//    return m_lithologyManager->getCompoundLithology ( composition );
// }

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

void GeoPhysics::ProjectHandle::addUndefinedAreas( const DataAccess::Interface::GridMap* theMap ) {

   if ( theMap == nullptr ) {
      return;
   }

   unsigned int i;
   unsigned int j;

   bool dataIsRetrieved = theMap->retrieved ();

   if ( not dataIsRetrieved ) {
      theMap->retrieveGhostedData ();
   }

   for ( i = m_validNodes.first ( 0 ); i <= m_validNodes.last ( 0 ); ++i ) {

      for ( j = m_validNodes.first ( 1 ); j <= m_validNodes.last ( 1 ); ++j ) {

         if ( theMap->getValue ( i, j ) == theMap->getUndefinedValue ()) {
            m_validNodes ( i, j ) = false;
         }

      }

   }

   if ( not dataIsRetrieved ) {
      // If the data was not retrived then restore the map back to its original state.
      theMap->restoreData ( false, true );
   }

}

//------------------------------------------------------------//

void GeoPhysics::ProjectHandle::addCrustUndefinedAreas ( const Interface::CrustFormation* crust ) {

   if ( crust != 0 ) {

      // Add undefined areas from crust thickness maps.
      Interface::PaleoFormationPropertyList::const_iterator thicknessIter;
      Interface::PaleoFormationPropertyList* thicknesses = crust->getPaleoThicknessHistory ();

      for ( thicknessIter = thicknesses->begin (); thicknessIter != thicknesses->end (); ++thicknessIter ) {
         addUndefinedAreas ( dynamic_cast<const Interface::GridMap*>((*thicknessIter)->getMap ( Interface::CrustThinningHistoryInstanceThicknessMap )));
      }

      if( m_isALCMode ) {
         // legacy ALC
         addUndefinedAreas ( dynamic_cast<const Interface::GridMap*>(crust->getBasaltThicknessMap ()));
         addUndefinedAreas ( dynamic_cast<const Interface::GridMap*>(crust->getCrustThicknessMeltOnsetMap ()));
         // v2017.05 ALC
         auto oceaData = m_tableOceanicCrustThicknessHistory.data();
         std::for_each( oceaData.begin(), oceaData.end(), [&]( std::shared_ptr<const OceanicCrustThicknessHistoryData> oceanicCrust)
         {
            addUndefinedAreas( oceanicCrust->getMap() );
         } );
      }
      addUndefinedAreas ( dynamic_cast<const Interface::GridMap*>(crust->getCrustHeatProductionMap ()));

      delete thicknesses;
   }

}

//------------------------------------------------------------//

void GeoPhysics::ProjectHandle::addUndefinedAreas ( const DataAccess::Interface::PropertyValue* theProperty ) {

   if ( theProperty != 0 ) {
      const Interface::GridMap* theMap = dynamic_cast<const Interface::GridMap*>(theProperty->getGridMap ());

      addUndefinedAreas ( theMap );
   }

}

//------------------------------------------------------------//

void GeoPhysics::ProjectHandle::addMantleUndefinedAreas ( const Interface::MantleFormation* mantle ) {

   if ( mantle != 0 ) {
      if( mantle->getPaleoThicknessHistory () != 0 ) {
         Interface::PaleoFormationPropertyList::const_iterator thicknessIter;
         Interface::PaleoFormationPropertyList* thicknesses = mantle->getPaleoThicknessHistory ();

         for ( thicknessIter = thicknesses->begin (); thicknessIter != thicknesses->end (); ++thicknessIter ) {
            addUndefinedAreas ( dynamic_cast<const Interface::GridMap*>((*thicknessIter)->getMap ( Interface::MantleThicknessHistoryInstanceThicknessMap )));
         }
         delete thicknesses;
      }

      // Add undefined areas from heat flow maps.
      // Not explicitly dependent on the mantle.
      Interface::PaleoSurfacePropertyList* heatFlowMaps = getHeatFlowHistory ();
      Interface::PaleoSurfacePropertyList::const_iterator heatFlowIter;

      for ( heatFlowIter = heatFlowMaps->begin (); heatFlowIter != heatFlowMaps->end (); ++heatFlowIter ) {
         addUndefinedAreas ( dynamic_cast<const Interface::GridMap*>((*heatFlowIter)->getMap ( Interface::HeatFlowHistoryInstanceHeatFlowMap )));
      }
      delete heatFlowMaps;
   }

}

//------------------------------------------------------------//

void GeoPhysics::ProjectHandle::addFormationUndefinedAreas ( const Interface::Formation* formation ) {

   if ( formation != 0 and formation->kind () == Interface::SEDIMENT_FORMATION ) {

      const Interface::GridMap * LithoType1PercentageMap = dynamic_cast<const Interface::GridMap*>( formation->getLithoType1PercentageMap( ) );
      const Interface::GridMap * LithoType2PercentageMap = dynamic_cast<const Interface::GridMap*>( formation->getLithoType2PercentageMap( ) );
      const Interface::GridMap * LithoType3PercentageMap = dynamic_cast<const Interface::GridMap*>( formation->getLithoType3PercentageMap( ) );
      addUndefinedAreas( LithoType1PercentageMap );
      addUndefinedAreas( LithoType2PercentageMap );
      addUndefinedAreas( LithoType3PercentageMap );


      const Interface::GridMap * InputThicknessMap = dynamic_cast<const Interface::GridMap*> ( formation->getInputThicknessMap( ) );
      addUndefinedAreas( InputThicknessMap );

      if ( formation->isSourceRock ())
      {
         const Interface::GridMap* SourceRock1 = dynamic_cast<const Interface::GridMap*>( formation->getSourceRock1( )->getMap( Interface::TocIni )  );
         addUndefinedAreas( SourceRock1);
         if ( SourceRock1 ) SourceRock1->release( );
      }

      if ( formation->isMobileLayer ()) {
         // Add undefined areas from mobile-layer maps.
         Interface::MobileLayerList::const_iterator mobIt;
         Interface::MobileLayerList* mobileLayers(formation->getMobileLayers( ));

         for ( mobIt = mobileLayers->begin (); mobIt != mobileLayers->end (); ++mobIt )
         {
            const Interface::GridMap* mobLayerThickness = dynamic_cast<const Interface::GridMap*>( ( *mobIt )->getMap( Interface::MobileLayerThicknessMap ) );
            addUndefinedAreas( mobLayerThickness );
            if ( mobLayerThickness ) mobLayerThickness->release( );
         }
         if ( mobileLayers ) delete mobileLayers;
      }

      if ( formation->hasAllochthonousLithology ()) {
         // Add undefined areas from allochthonous-distributions.
         Interface::AllochthonousLithologyDistributionList::const_iterator allochIt;
         Interface::AllochthonousLithologyDistributionList * allochthonousDistributions( formation->getAllochthonousLithology()->getAllochthonousLithologyDistributions() );

         for ( allochIt = allochthonousDistributions->begin (); allochIt != allochthonousDistributions->end (); ++ allochIt )
         {
            const Interface::GridMap * allochthonousLithologyDistributionMap = dynamic_cast<const Interface::GridMap*> ( ( *allochIt )->getMap( Interface::AllochthonousLithologyDistributionMap ) );
            addUndefinedAreas( allochthonousLithologyDistributionMap );
            if ( allochthonousLithologyDistributionMap ) allochthonousLithologyDistributionMap->release( );
         }
         if ( allochthonousDistributions ) delete allochthonousDistributions;
      }

      // Add undefined areas from reservoirs maps.
      Interface::ReservoirList::const_iterator resIt;
      Interface::ReservoirList * reservoirs(formation->getReservoirs( ));

      for ( resIt = reservoirs->begin (); resIt != reservoirs->end (); ++resIt )
      {
         const Interface::GridMap * netToGrossMap         = dynamic_cast<const Interface::GridMap*>( ( *resIt )->getMap( Interface::NetToGross ) );
         addUndefinedAreas( netToGrossMap);
         if ( netToGrossMap         ) netToGrossMap->release( );
      }
      delete reservoirs;


      if ( LithoType1PercentageMap ) LithoType1PercentageMap->release();
      if ( LithoType2PercentageMap ) LithoType2PercentageMap->release();
      if ( LithoType3PercentageMap ) LithoType3PercentageMap->release();
   }

}

//------------------------------------------------------------//

void GeoPhysics::ProjectHandle::filterValidNodesByValidElements()
{
   /// An active element is a 4-points element with valid nodes ONLY.
   /// It is identified by its leftmost-bottomost node, the other nodes
   /// can be accessed as shown in the following scheme.
   ///
   /// (i+1,j)  (i+1,j+1)
   ///   O---------O
   ///   |         |
   ///   |         |
   ///   |         |
   ///   O---------O
   /// (i,j)     (i,j+1)
   ///
   /// NB: This is a temporary local struct
   struct ActiveElem
   {
      ActiveElem() : m_i(0),m_j(0) {}
      ActiveElem( const int i, const int j ) : m_i(i),m_j(j) {}
      const int m_i, m_j; // leftmost-bottommost element node indeces
   };

   const DataAccess::Interface::Grid * grid = getActivityOutputGrid();
   const bool useGhosts = true;
   const unsigned int firstIgh = grid->firstI( useGhosts );
   const unsigned int lastIgh  = grid->lastI ( useGhosts );
   const unsigned int firstJgh = grid->firstJ( useGhosts );
   const unsigned int lastJgh  = grid->lastJ ( useGhosts );

   // Create vector of all local active elements (including ghost nodes)
   // The criteria is that all the 4 nodes belonging to the element must be valid
   unsigned int activeCounter = 0;
   std::vector<ActiveElem> activeElemVec;
   activeElemVec.reserve( (lastIgh-firstIgh-1) * (lastJgh-firstJgh-1) );
   for( unsigned int i = firstIgh; i < lastIgh; ++i )
   {
      for( unsigned int j = firstJgh; j < lastJgh; ++j )
      {
         if( m_validNodes( i, j ) and m_validNodes( i+1, j ) and m_validNodes( i, j+1 ) and m_validNodes( i+1, j+1 ) )
         {
            activeElemVec.push_back( ActiveElem(i, j) );
            ++activeCounter;
         }
      }
   }
   activeElemVec.resize( activeCounter );

   // Now that we have stored all the information about the active elements we can
   // reset the valid node 2D array to false e fill it in again using the above information
   m_validNodes.fill( false );
   for( unsigned int elemIdx = 0; elemIdx < activeCounter; ++elemIdx )
   {
      const ActiveElem & elem = activeElemVec[elemIdx];
      m_validNodes( elem.m_i,   elem.m_j   ) = true;
      m_validNodes( elem.m_i+1, elem.m_j   ) = true;
      m_validNodes( elem.m_i,   elem.m_j+1 ) = true;
      m_validNodes( elem.m_i+1, elem.m_j+1 ) = true;
   }

   // Now the information about the ghost nodes has to be communicated to the other processors (if any)
   // Create a GridMap for the nodes validity, initialised to 0 (not valid)
   DataAccess::Interface::GridMap * validGridMap = getFactory()->produceGridMap( 0, 0, grid, 0.0 );
   validGridMap->retrieveData( useGhosts );
   for( unsigned int i = firstIgh; i <= lastIgh; ++i )
   {
      for( unsigned int j = firstJgh; j <= lastJgh; ++j )
      {
         if( m_validNodes( i, j ) ) validGridMap->setValue( i, j, 1.0 );
         else validGridMap->setValue( i, j, 0.0 ); // Temporary workaround for a bug in DistributedGridMap::restoreData
      }
   }
   validGridMap->restoreData( true, useGhosts );

   // Check if the current local grid has ghost nodes in the 4 directions
   // If in one direction there are no ghost nodes it means that it's a real boundary
   const bool hasLeftGhost   = firstIgh != grid->firstI( !useGhosts );
   const bool hasRightGhost  = lastIgh  != grid->lastI ( !useGhosts );
   const bool hasBottomGhost = firstJgh != grid->firstJ( !useGhosts );
   const bool hasTopGhost    = lastJgh  != grid->lastJ ( !useGhosts );

   // Now loop over ghost nodes only:
   // The ghost node for the local grid will be set valid only if the gridMap has
   // a value greater than zero, this means that at least one processor
   // has set to 1 (eg valid) its local node
   validGridMap->retrieveData( useGhosts );
   if( hasLeftGhost )
   {
      for( unsigned int j = firstJgh; j <= lastJgh; ++j )
      {
         m_validNodes( firstIgh, j ) = (validGridMap->getValue( firstIgh, j ) > 0.0);
      }
   }
   if( hasRightGhost )
   {
      for( unsigned int j = firstJgh; j <= lastJgh; ++j )
      {
         m_validNodes( lastIgh, j ) = (validGridMap->getValue( lastIgh, j ) > 0.0);
      }
   }
   if( hasBottomGhost )
   {
      for( unsigned int i = firstIgh; i <= lastIgh; ++i )
      {
         m_validNodes( i, firstJgh ) = (validGridMap->getValue( i, firstJgh ) > 0.0);
      }
   }
   if( hasTopGhost )
   {
      for( unsigned int i = firstIgh; i <= lastIgh; ++i )
      {
         m_validNodes( i, lastJgh ) = (validGridMap->getValue( i, lastJgh ) > 0.0);
      }
   }
   validGridMap->restoreData( false, useGhosts );

   delete validGridMap;
}

//------------------------------------------------------------//

bool GeoPhysics::ProjectHandle::initialiseValidNodes ( const bool readSizeFromVolumeData ) {

   m_validNodes.reallocate ( getActivityOutputGrid ());
   m_validNodes.fill ( true );

   Interface::MutableFormationList::iterator formationIter;

   // Add sediment-formation undefined areas:
   //
   //     o depth/thickness maps;
   //     o lithology maps;
   //     o mobile-layer maps;
   //     o allochthonous-lithology distribution maps;
   //     o all reservoir maps, if reservoir layer;
   for ( formationIter = m_formations.begin (); formationIter != m_formations.end (); ++formationIter ) {

      if ((*formationIter)->kind () == Interface::SEDIMENT_FORMATION ) {
         addFormationUndefinedAreas ( *formationIter );
      }

   }

   // Add crust-formation undefined areas.
   addCrustUndefinedAreas ( getCrustFormation ());

   // Add mantle-formation undefined areas.
   addMantleUndefinedAreas ( getMantleFormation ());

   // Add undefined areas from surface-depth maps.
   Interface::PaleoPropertyList * surfaceDepthMaps = getSurfaceDepthHistory ();
   Interface::PaleoPropertyList::const_iterator surfaceDepthIter;

   for ( surfaceDepthIter = surfaceDepthMaps->begin (); surfaceDepthIter != surfaceDepthMaps->end (); ++surfaceDepthIter )
   {
      const Interface::GridMap * surfaceDepthHistoryMap = dynamic_cast<const Interface::GridMap *>(( *surfaceDepthIter )->getMap( Interface::SurfaceDepthHistoryInstanceMap ) );
      addUndefinedAreas( surfaceDepthHistoryMap);
      if ( surfaceDepthHistoryMap ) surfaceDepthHistoryMap->release();
   }

   if ( surfaceDepthMaps ) delete surfaceDepthMaps;


   // Add undefined areas from surface-temperature maps.
   Interface::PaleoPropertyList * surfaceTemperatureMaps(getSurfaceTemperatureHistory( ));
   Interface::PaleoPropertyList::const_iterator surfaceTemperatureIter;

   for ( surfaceTemperatureIter = surfaceTemperatureMaps->begin (); surfaceTemperatureIter != surfaceTemperatureMaps->end (); ++surfaceTemperatureIter )
   {
      const Interface::GridMap * surfaceTemperatureHistoryMap = dynamic_cast<const Interface::GridMap *>(( *surfaceTemperatureIter )->getMap( Interface::SurfaceTemperatureHistoryInstanceMap ));
      addUndefinedAreas( surfaceTemperatureHistoryMap );
      if ( surfaceTemperatureHistoryMap ) surfaceTemperatureHistoryMap->release();
   }
   if ( surfaceTemperatureMaps ) delete surfaceTemperatureMaps;


   if ( readSizeFromVolumeData ) {
      // Since the input depends on some results, these need to be
      // taken into account when setting the valid node array.
      // And, since the number of element depends on the results volume-file
      // there must be some results already.

      // Any property could be used here that is always output, Ves is always output.
      const Interface::Property* vesProperty = findProperty ( "Ves" );

      // Any sediment-formation will do, so use the first on the list.
      Interface::PropertyValueList* vesValueList = getPropertyValues ( Interface::FORMATION,
                                                                       vesProperty,
                                                                       findSnapshot ( 0.0, Interface::MAJOR ),
                                                                       0,
                                                                       *m_formations.begin (),
                                                                       0,
                                                                       Interface::VOLUME );

      assert ( vesValueList->size () == 1 );

      const Interface::PropertyValue* ves = *vesValueList->begin ();


      if ( ves != 0 and ves->getGridMap () != 0 )
      {
         const Interface::GridMap* vesGridMap = ves->getGridMap( );
         if ( vesGridMap )
         {
            addUndefinedAreas( dynamic_cast<const Interface::GridMap*>( ves->getGridMap() ) );
            vesGridMap->release();
         }
      }

      delete vesValueList;

   }

   filterValidNodesByValidElements();

   return true;
}

//------------------------------------------------------------//

bool GeoPhysics::ProjectHandle::initialise ( const bool readSizeFromVolumeData,
                                             const bool printTable ) {

   bool result = true;

   // Since the input depends on some results, these need to be
   // taken into account when setting the valid node array.
   result = initialiseValidNodes ( readSizeFromVolumeData );


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
   result = createCrustThickness          () and result;
   result = determineLayerMinMaxThickness () and result;
   result = determineCrustThinningRatio   () and result;
   result = determineMaximumNumberOfSegmentsPerLayer ( readSizeFromVolumeData, printTable ) and result;

   return result;

}

//------------------------------------------------------------//

void GeoPhysics::ProjectHandle::checkAlcCrustHistoryInput() {
   Interface::PaleoFormationPropertyList* crustThicknesses = getCrustFormation()->getPaleoThicknessHistory();
   GeoPhysics::GeoPhysicsCrustFormation*  crust = dynamic_cast<GeoPhysics::GeoPhysicsCrustFormation*>(m_crustFormation);
   const Interface::GridMap* presentDayBasaltThickness = crust->getBasaltThicknessMap();
   const Interface::GridMap* crustMeltOnsetMap         = crust->getCrustThicknessMeltOnsetMap();
   // If the legacy alc is used do not check anything
   if (presentDayBasaltThickness == nullptr and crustMeltOnsetMap == nullptr) {
      for (auto continentalCrustReverseIter = crustThicknesses->rbegin(); continentalCrustReverseIter != crustThicknesses->rend(); ++continentalCrustReverseIter) {
         const PaleoFormationProperty* contCrustThicknessInstance = *continentalCrustReverseIter;
         const GridMap* contCrustThicknessMap = contCrustThicknessInstance->getMap( CrustThinningHistoryInstanceThicknessMap );
         const double age = contCrustThicknessInstance->getSnapshot()->getTime();
         auto oceanicCrustThicknessIt = std::find_if( m_tableOceanicCrustThicknessHistory.data().begin(), m_tableOceanicCrustThicknessHistory.data().end(),
            [&age]( std::shared_ptr<const OceanicCrustThicknessHistoryData> obj ) { return obj->getAge() == age; } );
         if (oceanicCrustThicknessIt == m_tableOceanicCrustThicknessHistory.data().end()) {
            throw std::invalid_argument( "There is no oceanic crustal thickness corresponding to the contiental crustal thickness defined at " + std::to_string( age ) + "Ma" );
         }
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

   Interface::MutableFormationList::iterator formationIter;

   for ( formationIter = m_formations.begin (); formationIter != m_formations.end (); ++formationIter ) {
      GeoPhysics::Formation* formation = dynamic_cast<GeoPhysics::Formation*>(*formationIter);
      formation->switchLithologies ( age );
   }

}

//------------------------------------------------------------//

void GeoPhysics::ProjectHandle::setBasinAge () {

   m_basinAge = getCrustFormation ()->getTopSurface ()->getSnapshot ()->getTime ();

}

//------------------------------------------------------------//

bool GeoPhysics::ProjectHandle::createSeaBottomTemperature () {

   bool status = true;

   unsigned int i;
   unsigned int j;

   //Create 2D Array of Polyfunction for Sea Bottom Temperature
   m_seaBottomTemperature.reallocate ( getActivityOutputGrid ());

   Interface::PaleoPropertyList* surfaceTemperatureHistory = getSurfaceTemperatureHistory ();
   Interface::PaleoPropertyList::const_iterator surfaceTemperatureIter;

   if ( surfaceTemperatureHistory != 0 && surfaceTemperatureHistory->size() != 0 ) {

      for ( surfaceTemperatureIter = surfaceTemperatureHistory->begin (); surfaceTemperatureIter != surfaceTemperatureHistory->end (); ++surfaceTemperatureIter ) {

         const Interface::PaleoProperty* surfaceTemperatureInstance = *surfaceTemperatureIter;
         const Interface::GridMap* surfaceTemperatureMap = dynamic_cast<const Interface::GridMap*>(surfaceTemperatureInstance->getMap ( Interface::SurfaceTemperatureHistoryInstanceMap ));
         const double age = surfaceTemperatureInstance->getSnapshot ()->getTime ();

         // This should be with ghost nodes.
         surfaceTemperatureMap->retrieveGhostedData ();

         for ( i = surfaceTemperatureMap->getGrid()->firstI ( true ); i <= surfaceTemperatureMap->getGrid()->lastI ( true ); ++i ) {

            for ( j = surfaceTemperatureMap->getGrid()->firstJ ( true ); j <= surfaceTemperatureMap->getGrid()->lastJ ( true ); ++j ) {

               if ( m_validNodes ( i, j )) {
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

      getMessageHandler ().printLine (  "MeSsAgE ERROR  Surface temperature is not defined." );
   }

   return status;
}

//------------------------------------------------------------//

bool GeoPhysics::ProjectHandle::createPaleoBathymetry () {

   bool status;

   unsigned int i;
   unsigned int j;

   //Create 2D Array of Polyfunction for sea bottom depth.
   m_seaBottomDepth.reallocate ( getActivityOutputGrid ());

   Interface::PaleoPropertyList* surfaceDepthHistory = getSurfaceDepthHistory ();
   Interface::PaleoPropertyList::const_iterator surfaceDepthIter;

   if ( surfaceDepthHistory != 0 ) {

      for ( surfaceDepthIter = surfaceDepthHistory->begin (); surfaceDepthIter != surfaceDepthHistory->end (); ++surfaceDepthIter ) {

         const Interface::PaleoProperty* surfaceDepthInstance = *surfaceDepthIter;
         const double age = surfaceDepthInstance->getSnapshot ()->getTime ();
         const Interface::GridMap* surfaceDepthMap;

         if ( age > 0.0 ) {
            surfaceDepthMap = dynamic_cast<const Interface::GridMap*>(surfaceDepthInstance->getMap ( Interface::SurfaceDepthHistoryInstanceMap ));
         } else {
            // The present-day depth-map is obtained from the top surface of the sediments.
            surfaceDepthMap = dynamic_cast<const Interface::GridMap*>((*m_surfaces.begin ())->getInputDepthMap ());
         }

         // This should be with ghost nodes.
         surfaceDepthMap->retrieveGhostedData ();

         for ( i = surfaceDepthMap->firstI (); i <= surfaceDepthMap->lastI (); ++i ) {

            for ( j = surfaceDepthMap->firstJ (); j <= surfaceDepthMap->lastJ (); ++j ) {

               if ( m_validNodes ( i, j )) {
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

      getMessageHandler ().printLine (  "MeSsAgE ERROR  Surface depth is not defined." );
   }

   return status;
}

//------------------------------------------------------------//

bool GeoPhysics::ProjectHandle::createMantleHeatFlow () {

   if ( getBottomBoundaryConditions () == Interface::MANTLE_HEAT_FLOW ) {

      unsigned int i;
      unsigned int j;

      //Create 2D Array of Polyfunction for Mantle HeatFlow
      m_mantleHeatFlow.reallocate ( getActivityOutputGrid ());

      Interface::PaleoSurfacePropertyList* heatFlowHistory = getHeatFlowHistory ();
      Interface::PaleoSurfacePropertyList::const_iterator heatFlowIter;

      for ( heatFlowIter = heatFlowHistory->begin (); heatFlowIter != heatFlowHistory->end (); ++heatFlowIter ) {

         const Interface::PaleoSurfaceProperty* heatFlowInstance = dynamic_cast<const Interface::PaleoSurfaceProperty*>(*heatFlowIter);
         const Interface::GridMap* heatFlowMap = dynamic_cast<const Interface::GridMap*>(heatFlowInstance->getMap ( Interface::HeatFlowHistoryInstanceHeatFlowMap ));
         const double age = heatFlowInstance->getSnapshot ()->getTime ();

         heatFlowMap->retrieveGhostedData ();

         for ( i = heatFlowMap->getGrid()->firstI ( true ); i <= heatFlowMap->getGrid()->lastI ( true ); ++i ) {

            for ( j = heatFlowMap->getGrid()->firstJ ( true ); j <= heatFlowMap->getGrid()->lastJ ( true ); ++j ) {

               if ( m_validNodes ( i, j )) {
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
bool GeoPhysics::ProjectHandle::createBasaltThicknessAndECT () {

   if (not m_isALCMode) return false;
   bool status = false;
   //Create 2D Array of Polyfunction for Crust Thickness
   m_crustThicknessHistory.reallocate ( getActivityOutputGrid() );
   m_basaltThicknessHistory.reallocate( getActivityOutputGrid() );
   m_endOfRiftEvent.reallocate        ( getActivityOutputGrid() );

   Interface::PaleoFormationPropertyList* crustThicknesses = getCrustFormation()->getPaleoThicknessHistory();
   GeoPhysics::GeoPhysicsCrustFormation*  crust = dynamic_cast<GeoPhysics::GeoPhysicsCrustFormation*>(m_crustFormation);
   const Interface::GridMap* presentDayBasaltThickness = crust->getBasaltThicknessMap();
   const Interface::GridMap* crustMeltOnsetMap         = crust->getCrustThicknessMeltOnsetMap();
   const double initialLithosphericMantleThickness = getMantleFormation()->getInitialLithosphericMantleThickness();
   const double initialCrustalThickness            = crust->getInitialCrustalThickness();
   Validator validator( *this );

   EffectiveCrustalThicknessCalculator ectCalculator( crustThicknesses,
                                                      m_tableOceanicCrustThicknessHistory,
                                                      m_contCrustThicknessHistory,
                                                      presentDayBasaltThickness,
                                                      crustMeltOnsetMap,
                                                      initialLithosphericMantleThickness,
                                                      initialCrustalThickness,
                                                      validator );
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
   catch( std::runtime_error& ex ) {
      LogHandler( LogHandler::ERROR_SEVERITY ) << "The Advanced Lithosphere Calculator (ALC) could not compute the Effective Crustal Thickness (see details bellow)";
      LogHandler( LogHandler::ERROR_SEVERITY ) << ex.what();
   }
   catch (...) {
      LogHandler( LogHandler::ERROR_SEVERITY ) << "The Advanced Lithosphere Calculator encounterred a fatal error (unkown details)";
   }

   delete crustThicknesses;
   return status;
}
//------------------------------------------------------------//

bool GeoPhysics::ProjectHandle::createCrustThickness () {

   unsigned int i;
   unsigned int j;

   LogHandler( LogHandler::INFO_SEVERITY, LogHandler::SECTION ) << "Bottom boundary conditions";
   //Create 2D Array of Polyfunction for Crust Thickness
   m_crustThicknessHistory.reallocate ( getActivityOutputGrid ());

   if ( getBottomBoundaryConditions () == Interface::FIXED_BASEMENT_TEMPERATURE ) {
      LogHandler( LogHandler::INFO_SEVERITY, LogHandler::SUBSECTION ) << "Basic Crust Thinning History";
      const Interface::Snapshot* firstSimulationSnapshot = m_crustFormation->getTopSurface ()->getSnapshot ();
      Interface::PaleoFormationPropertyList* crustThicknesses = getCrustFormation ()->getPaleoThicknessHistory ();
      Interface::PaleoFormationPropertyList::const_iterator crustThicknessIter;

      for ( crustThicknessIter = crustThicknesses->begin (); crustThicknessIter != crustThicknesses->end (); ++crustThicknessIter ) {

         const Interface::PaleoFormationProperty* thicknessInstance = dynamic_cast<const Interface::PaleoFormationProperty*>(*crustThicknessIter);
         const Interface::GridMap* thicknessMap = dynamic_cast<const Interface::GridMap*>(thicknessInstance->getMap ( Interface::CrustThinningHistoryInstanceThicknessMap ));
         const double age = thicknessInstance->getSnapshot ()->getTime ();
         double min, max;
         thicknessMap->retrieveData ( true );
         thicknessMap->getMinMaxValue ( min, max );
         if ( true or age <= firstSimulationSnapshot->getTime ()) {

         thicknessMap->retrieveData ( true );

         for ( i = thicknessMap->firstI (); i <= thicknessMap->lastI (); ++i ) {

            for ( j = thicknessMap->firstJ (); j <= thicknessMap->lastJ (); ++j ) {

               if ( m_validNodes ( i, j )) {
                  m_crustThicknessHistory ( i, j ).AddPoint( age, thicknessMap->getValue ( i, j ));
               }

            }

         }

         thicknessMap->restoreData ( false, true );
         }
      }

      delete crustThicknesses;

   } else if ( m_isALCMode ) {
      LogHandler( LogHandler::INFO_SEVERITY, LogHandler::SUBSECTION ) << "Advanced Lithospheric Calculator (ALC)";
      const Interface::Snapshot* firstSimulationSnapshot = m_crustFormation->getTopSurface ()->getSnapshot ();

      Interface::PaleoFormationPropertyList* crustThicknesses = getCrustFormation ()->getPaleoThicknessHistory ();
      Interface::PaleoFormationPropertyList::const_iterator crustThicknessIter;

      m_contCrustThicknessHistory.reallocate ( getActivityOutputGrid ());

      double localInitialCrustThickness =  -9999999999.9;
      double localMaximumCrustThickness =  -9999999999.9;
      double currentThickness;

      double oldestMapAge = dynamic_cast<const Interface::PaleoFormationProperty*>(*(crustThicknesses->rbegin ()))->getSnapshot ()->getTime ();

      bool flag = false;
      for ( crustThicknessIter = crustThicknesses->begin (); crustThicknessIter != crustThicknesses->end (); ++crustThicknessIter ) {

         const Interface::PaleoFormationProperty* thicknessInstance = dynamic_cast<const Interface::PaleoFormationProperty*>(*crustThicknessIter);
         const Interface::GridMap* thicknessMap = dynamic_cast<const Interface::GridMap*>(thicknessInstance->getMap ( Interface::CrustThinningHistoryInstanceThicknessMap ));
         const double age = thicknessInstance->getSnapshot ()->getTime ();

         if ( true or age <= firstSimulationSnapshot->getTime ()) {

            thicknessMap->retrieveData ( true );
            for ( i = thicknessMap->getGrid()->firstI ( true ); i <= thicknessMap->getGrid()->lastI ( true ); ++i ) {

               for ( j = thicknessMap->getGrid()->firstJ ( true ); j <= thicknessMap->getGrid()->lastJ ( true ); ++j ) {

                  if ( m_validNodes ( i, j )) {
                     currentThickness = thicknessMap->getValue ( i, j );
                     m_contCrustThicknessHistory ( i, j ).AddPoint( age, currentThickness );
                     if( age == m_basinAge ) {
                        flag = true;
                        localInitialCrustThickness = NumericFunctions::Maximum ( localInitialCrustThickness, currentThickness );  
                     } 
                     if( age == oldestMapAge ) {
                        localMaximumCrustThickness = NumericFunctions::Maximum ( localMaximumCrustThickness, currentThickness );
                     }
                  }
               }

            }

            thicknessMap->restoreData ( false, true );
         }
      }

      GeoPhysics::GeoPhysicsCrustFormation* crust = dynamic_cast<GeoPhysics::GeoPhysicsCrustFormation*>( m_crustFormation );
      double initialCrustalThickness = 0;

      getMaxValue ( &localInitialCrustThickness, &initialCrustalThickness );

      if( initialCrustalThickness < 0 ) {
         if( flag ) {
            LogHandler( LogHandler::WARNING_SEVERITY ) << "Initial crustal thickness is negative";
         } else {
            LogHandler( LogHandler::WARNING_SEVERITY ) << "Initial crustal thickness is not defined at the age of basin";
         }

         getMaxValue ( &localMaximumCrustThickness, &initialCrustalThickness );

         if( initialCrustalThickness < 0 ) {
            LogHandler( LogHandler::ERROR_SEVERITY ) << "Could not determine the initial crustal thickness";
            return false;
         } else {
            LogHandler( LogHandler::INFO_SEVERITY ,LogHandler::COMPUTATION_STEP ) << "setting InitialCrustalThickness to " << initialCrustalThickness;
         }
      } else {
           LogHandler( LogHandler::INFO_SEVERITY ,LogHandler::COMPUTATION_STEP ) << "InitialCrustalThickness = " << initialCrustalThickness;
      }
      crust->setInitialCrustalThickness( initialCrustalThickness );

      delete crustThicknesses;

      if( !createBasaltThicknessAndECT() ) {
         return false;
      }

   } else {
      LogHandler( LogHandler::INFO_SEVERITY, LogHandler::SUBSECTION ) << "Heat Flow History";
      const Interface::GridMap* thicknessMap = getCrustFormation ()->getInputThicknessMap ();

      bool retrieved = thicknessMap->retrieved ();

      if ( not retrieved ) {
         thicknessMap->retrieveData ( true );
      }

      for ( i = thicknessMap->firstI (); i <= thicknessMap->lastI (); ++i ) {

         for ( j = thicknessMap->firstJ (); j <= thicknessMap->lastJ (); ++j ) {

            if ( m_validNodes ( i, j )) {
               m_crustThicknessHistory ( i, j ).AddPoint( 0.0, thicknessMap->getValue ( i, j ));
            }

         }

      }

      if ( not retrieved ) {
         thicknessMap->restoreData ( false, true );
      }

   }

   return true;
}

//------------------------------------------------------------//

bool GeoPhysics::ProjectHandle::determineLayerMinMaxThickness () {

   Interface::MutableFormationList::iterator formationIter;

   for ( formationIter = m_formations.begin (); formationIter != m_formations.end (); ++formationIter ) {

      if ((*formationIter)->kind () == Interface::SEDIMENT_FORMATION ) {
         GeoPhysics::Formation* formation = dynamic_cast<GeoPhysics::Formation*>(*formationIter);

         formation->determineMinMaxThickness ();
      }

   }

   dynamic_cast<GeoPhysics::GeoPhysicsCrustFormation*>( m_crustFormation )->determineMinMaxThickness ();
   dynamic_cast<GeoPhysics::GeoPhysicsMantleFormation*>( m_mantleFormation )->determineMinMaxThickness ();

   return true;
}

//------------------------------------------------------------//

bool GeoPhysics::ProjectHandle::determineCrustThinningRatio () {

   // Should this function be in the Crust class?

   GeoPhysics::GeoPhysicsCrustFormation*  crust  = dynamic_cast<GeoPhysics::GeoPhysicsCrustFormation*>( m_crustFormation );
   GeoPhysics::GeoPhysicsMantleFormation* mantle = dynamic_cast<GeoPhysics::GeoPhysicsMantleFormation*>( m_mantleFormation );

   bool status = true;
   double minimumCrustThickness;
   double maximumBasementThickness;
   double maximumCrustThinningRatio;
   double mantleElementHeight = getRunParameters ()->getBrickHeightMantle ();

   status = status and crust->determineCrustThinningRatio ();
   minimumCrustThickness = crust->getMinimumThickness ();
   maximumBasementThickness = ( m_isALCMode ? mantle->getInitialLithosphericMantleThickness() + crust->getInitialCrustalThickness() :
                                mantle->getPresentDayThickness () + crust->getCrustMaximumThicknessHistory ( 0.0 ) );
   maximumCrustThinningRatio = crust->getCrustThinningRatio ();


   if ( not status ) {
      getMessageHandler ().printLine ( " MeSsAgE ERROR  Crust has some non-positive thickness values." );
      getMessageHandler ().printLine ( " MeSsAgE ERROR  For correct execution all crust thickness values must be positive" );
   } else if ( status and maximumCrustThinningRatio > MaximumReasonableCrustThinningRatio and not m_isALCMode ) {
      getMessageHandler ().printLine ( " MeSsAgE WARNING  Crust has very large ratio of crust thicknesses." );
      getMessageHandler ().printLine ( " MeSsAgE WARNING  This can result in an unreasonable number of elements in the basement." );
      getMessageHandler ().printLine ( " MeSsAgE WARNING  Possibly, resulting in extended execution-times." );
      getMessageHandler ().print ( " MeSsAgE WARNING  The mantle will have approximately " );
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

   Interface::MutableFormationList::iterator formationIter;
   unsigned int numberOfSegments;
   unsigned int totalSegmentCount = 0;

   for ( formationIter = m_formations.begin (); formationIter != m_formations.end (); ++formationIter ) {

      GeoPhysics::Formation* formation = dynamic_cast<GeoPhysics::Formation*>(*formationIter);

      numberOfSegments = formation->setMaximumNumberOfElements ( readSizeFromVolumeData );
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

         double currentAge, age = ( * surfaceTemperatureIter )->getSnapshot ()->getTime ();

         ++ surfaceTemperatureIter;

         for ( ; surfaceTemperatureIter != surfaceTemperatureHistory->rend (); ++ surfaceTemperatureIter ) {
            const Interface::PaleoProperty* surfaceTemperatureInstance = *surfaceTemperatureIter;
            const Interface::GridMap* surfaceTemperatureMap = dynamic_cast<const Interface::GridMap*>(surfaceTemperatureInstance->getMap ( Interface::SurfaceTemperatureHistoryInstanceMap ));

            currentAge = surfaceTemperatureInstance->getSnapshot ()->getTime ();
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

   // Intially the check is only for negative thicknesses in mobile layers.
   IntegerArray numberOfErrorsPerLayer ( m_formations.size () - 2, 0 );
   unsigned int i;
   unsigned int j;
   int formCount;

   for ( formationIter = m_formations.begin (); formationIter != m_formations.end (); ++formationIter ) {

      GeoPhysics::Formation* formation = dynamic_cast<GeoPhysics::Formation*>(*formationIter);

      formation->m_solidThickness.reallocate ( getActivityOutputGrid (), formation->getMaximumNumberOfElements ());
      formation->m_realThickness.reallocate  ( getActivityOutputGrid (), formation->getMaximumNumberOfElements ());
      formation->m_presentDayErodedThickness.reallocate ( getActivityOutputGrid ());

   }

   for ( formationIter = m_formations.begin (); formationIter != m_formations.end (); ++formationIter ) {
      GeoPhysics::Formation* formation = dynamic_cast<GeoPhysics::Formation*>(*formationIter);
      formation->retrieveAllThicknessMaps ();
   }

   FloatStack uncMaxVes;
   FloatStack uncThickness;
   int nrActUnc;
   bool errorFound = false;

   for ( i = firstI ( true ); i <= lastI ( true ); ++i ) {

      for ( j = firstJ ( true ); j <= lastJ ( true ); ++j ) {

         if ( getNodeIsValid ( i, j )) {

            for ( formCount = int (m_formations.size ()) - 1; formCount >= 0; --formCount ) {

               GeoPhysics::Formation* formation = dynamic_cast<GeoPhysics::Formation*>( m_formations [ (unsigned int)(formCount)]);

               if ( not computeThicknessHistories ( i, j, formation, numberOfErrorsPerLayer )) {
                  errorFound = true;
               }

            }

         // Iterate over all sediment-layers.
            for ( formationIter = m_formations.begin (); formationIter != m_formations.end (); ++formationIter )
            {
               GeoPhysics::Formation* formation = dynamic_cast<GeoPhysics::Formation*>( *formationIter );

               if ( formation->kind () == Interface::SEDIMENT_FORMATION ) {
                  storePresentDayThickness ( i, j, formation );
               }

            }



            // Start accounting for unconformities
            nrActUnc = 0;
            uncMaxVes.push_front (0);
            uncThickness.push_front (100000);

            // And now from top to bottom
            // Do not include crust or mantle !!
            for ( formationIter = m_formations.begin (); formationIter != m_formations.end (); ++formationIter )
            {
               GeoPhysics::Formation* formation = dynamic_cast<GeoPhysics::Formation*>( *formationIter );

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
      GeoPhysics::Formation* formation = dynamic_cast<GeoPhysics::Formation*>(*formationIter);
      formation->restoreAllThicknessMaps ();
   }

   for ( i = 0; i < numberOfErrorsPerLayer.size (); ++i ) {

      if ( numberOfErrorsPerLayer [ i ] > MaximumNumberOfErrorsPerLayer ) {
         cout << " MeSsAgE ERROR formation '" << m_formations [ numberOfErrorsPerLayer.size () - i - 1 ]->getName () << "' has multiple errors, more than is indicated." << endl;
      }

   }


   int errorFoundInt = ( errorFound ? 1 : 0 );
   int globalErrorFoundInt;

   getMaxValue ( errorFoundInt, globalErrorFoundInt );
   errorFound = ( globalErrorFoundInt == 1 );

   return not errorFound;
}

//------------------------------------------------------------//

void GeoPhysics::ProjectHandle::storePresentDayThickness ( const unsigned int i,
                                                           const unsigned int j,
                                                           GeoPhysics::Formation* formation ) {

   unsigned int zCount;
   double segmentThickness;
   double presentDayThickness = 0.0;

   for ( zCount = 0; zCount < formation->getMaximumNumberOfElements (); ++zCount )
   {
      segmentThickness = formation->getSolidThickness (i, j, zCount ).F (0.0);

      if ( segmentThickness != Interface::DefaultUndefinedScalarValue ) {
         presentDayThickness = presentDayThickness + segmentThickness;
      }

   }

   formation->m_presentDayErodedThickness ( i, j ) = presentDayThickness;
}

//------------------------------------------------------------//

bool GeoPhysics::ProjectHandle::computeThicknessHistories ( const unsigned int i,
                                                            const unsigned int j,
                                                                  GeoPhysics::Formation* formation,
                                                                  IntegerArray& numberOfErrorsPerLayer ) {

   if ( formation->isMobileLayer () or formation->kind () == Interface::BASEMENT_FORMATION ) {
      return setMobileLayerThicknessHistory ( i, j, formation, numberOfErrorsPerLayer );
   } else if ( formation->getIsIgneousIntrusion ()) {
      return setIgneousIntrusionThicknessHistory ( i, j, formation, numberOfErrorsPerLayer );
   } else {

      double thickness = formation->getInputThicknessMap ()->getValue ( i, j );

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
                                                       const double       thickness,
                                                             GeoPhysics::Formation* formation ) {

   double segmentThickness = thickness / double ( formation->getMaximumNumberOfElements ());

   double startDepositionAge = formation->getBottomSurface ()->getSnapshot ()->getTime ();
   double endDepositionAge = formation->getTopSurface ()->getSnapshot ()->getTime ();

   // Notice the sign of elementDepositionDurations is -ve,
   // because we are counting from the top of the element stack.
   double elementDepositionDuration = ( endDepositionAge - startDepositionAge ) / double ( formation->getMaximumNumberOfElements ());

   unsigned int segment;

   for ( segment = 0; segment < formation->getMaximumNumberOfElements (); ++segment ) {

      formation->getSolidThickness ( i, j, segment ).AddPoint ( startDepositionAge, 0.0 );
      formation->getRealThickness  ( i, j, segment ).AddPoint ( startDepositionAge, 0.0 );

#if 0
      formation->setSolidThickness ( i, j, segment, startDepositionAge, 0.0 );
      formation->setRealThickness  ( i, j, segment, startDepositionAge, 0.0 );
#endif

      endDepositionAge = startDepositionAge + elementDepositionDuration;

      formation->getSolidThickness ( i, j, segment ).AddPoint ( endDepositionAge, segmentThickness );
      formation->getRealThickness  ( i, j, segment ).AddPoint ( endDepositionAge, segmentThickness );

#if 0
      formation->setSolidThickness ( i, j, segment, endDepositionAge, segmentThickness );
      formation->setRealThickness  ( i, j, segment, endDepositionAge, segmentThickness );
#endif

      startDepositionAge = endDepositionAge;
   }

   return true;
}

//------------------------------------------------------------//

bool GeoPhysics::ProjectHandle::setHistoriesForUnconformity ( const unsigned int i,
                                                              const unsigned int j,
                                                              const double       thickness,
                                                                    GeoPhysics::Formation* formation ) {

   double endErosionAge   = formation->getTopSurface ()->getSnapshot ()->getTime ();
   double startErosionAge = formation->getBottomSurface ()->getSnapshot ()->getTime ();

   // calculate how much we have eroded
   double uncThickness = -thickness;

   bool result = true;

   GeoPhysics::Formation* currentFormation = formation;

   while ( result and uncThickness > ThicknessTolerance ) {

      currentFormation = const_cast<GeoPhysics::Formation*>(dynamic_cast<const GeoPhysics::Formation*>(currentFormation->getBottomSurface ()->getBottomFormation ()));

      if ( currentFormation->kind () == Interface::BASEMENT_FORMATION ) {
         // if we have reached the crust then exit with success
         return true;
      }

      if ( currentFormation->isMobileLayer ()) {

         if ( uncThickness <= MobileLayerNegativeThicknessTolerance ) {
            uncThickness = 0.0;
            continue;
         } else {
            cout << "MeSsAgE ERROR Erosion of mobile layer [" << currentFormation->getName () << "] at position (" << i << ", " << j << ") " << thickness <<  "  is not permitted " << endl;
            return false;
         }

      }

      if ( currentFormation->getIsIgneousIntrusion ()) {
         cout << "MeSsAgE ERROR Erosion of igneousIntrusion [" << currentFormation->getName () << "] is not permitted " << endl;
         return false;
      }

      if ( currentFormation->getInputThicknessMap ()->getValue ( i, j ) <= ThicknessTolerance ) {
         continue;
      }

      double layerThickness = 0.0;
      double segmentThickness = 0.0;

      unsigned int segment = 0;

      for ( segment = 0; segment < currentFormation->getMaximumNumberOfElements (); ++segment ) {
         segmentThickness = currentFormation->getSolidThickness ( i, j, segment ).F ( 0.0 );
//          segmentThickness = currentFormation->getSolidThickness ( i, j, segment, 0.0 );

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

      double erodedThickness = NumericFunctions::Minimum (layerThickness, uncThickness);

      double localEndErosionAge = startErosionAge +
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
                                                    GeoPhysics::Formation* formation,
                                                    const double startErosionAge,
                                                    const double endErosionAge,
                                                    const double erodedThickness ) {

   static const double ThicknessEpsilon = 1.0e-8;

   CBMGenerics::Polyfunction *pf;
   CBMGenerics::Polyfunction *real_thickness_pf;
   double polyfThickness;

   double actualErodedThickness = erodedThickness;
   double segmentThicknessAfterErosion;
   double segmentStartErosionAge = startErosionAge;

   // we are going to iterate through the segments in this layer and remove
   // the erosion
   assert (0 != formation->getMaximumNumberOfElements ());

   int segment = 0;

   for ( segment = formation->getMaximumNumberOfElements () - 1; segment >= 0; --segment ) {

      // if we have dealt with all of the eroded thickness
      if ( actualErodedThickness <= ThicknessTolerance ) {
         break;
      }

      pf = &(formation->getSolidThickness ( i, j, segment ));
      real_thickness_pf = &(formation->getRealThickness (i, j, segment ));

      pf->MinX (polyfThickness);
      assert (polyfThickness != Interface::DefaultUndefinedScalarValue );

      if (polyfThickness > 0)
      {
         double segErosionThickness = NumericFunctions::Minimum (polyfThickness, actualErodedThickness );

         assert (0 != actualErodedThickness );
         double segEndErosionAge = segmentStartErosionAge +
               (segErosionThickness / actualErodedThickness) * (endErosionAge - segmentStartErosionAge);

         bool added = pf->AddPoint ( segmentStartErosionAge, polyfThickness);

         added = real_thickness_pf->AddPoint ( segmentStartErosionAge, polyfThickness);

         if ( std::fabs ( polyfThickness - segErosionThickness ) < ThicknessEpsilon ) {
            // If the difference between the two thickness is very small
            // then set the eroded thickness to be zero.
            segmentThicknessAfterErosion = 0.0;
         } else {
            // otherwise set the actual eroded thickness.
            segmentThicknessAfterErosion = polyfThickness - segErosionThickness;
         }

         added = pf->AddPoint (segEndErosionAge, segmentThicknessAfterErosion );
         assert (added);

         added = real_thickness_pf->AddPoint (segEndErosionAge, segmentThicknessAfterErosion);

         segmentStartErosionAge = segEndErosionAge;
         actualErodedThickness -= segErosionThickness;
      }

   }

   return true;
}

//------------------------------------------------------------//

bool GeoPhysics::ProjectHandle::setMobileLayerThicknessHistory ( const unsigned int i,
                                                                 const unsigned int j,
                                                                       GeoPhysics::Formation* formation,
                                                                       IntegerArray& numberOfErrorsPerLayer ) {

   if ( formation->isMantle ()) {
      return true;
   } else if ( formation->isCrust ()) {

      if ( getBottomBoundaryConditions () == Interface::FIXED_BASEMENT_TEMPERATURE ||
           getBottomBoundaryConditions () == Interface::ADVANCED_LITHOSPHERE_CALCULATOR) {
         Interface::PaleoFormationPropertyList::const_iterator mapIter;
         Interface::PaleoFormationPropertyList* crustThicknesses = dynamic_cast<GeoPhysics::GeoPhysicsCrustFormation*>(formation)->getPaleoThicknessHistory ();

         unsigned int segment;
         double age;

         double segmentThickness;

         for ( mapIter = crustThicknesses->begin (); mapIter != crustThicknesses->end (); ++mapIter ) {
            age = (*mapIter)->getSnapshot ()->getTime ();

            if( m_isALCMode ) {
               segmentThickness = getCrustThickness( i, j, age ) / double ( formation->getMaximumNumberOfElements ());
             } else {
               segmentThickness = (*mapIter)->getMap ( Interface::CrustThinningHistoryInstanceThicknessMap )->getValue ( i, j ) / double ( formation->getMaximumNumberOfElements ());
            }
            for ( segment = 0; segment < formation->getMaximumNumberOfElements (); ++segment ) {
               formation->getSolidThickness ( i, j, segment ).AddPoint ( age, segmentThickness );
               formation->getRealThickness  ( i, j, segment ).AddPoint ( age, segmentThickness );
            }

         }

         delete crustThicknesses;
      } else {

         double segmentThickness = formation->getInputThicknessMap ()->getValue ( i, j ) / double ( formation->getMaximumNumberOfElements ());
         unsigned int segment;

         for ( segment = 0; segment < formation->getMaximumNumberOfElements (); ++segment ) {
            formation->getSolidThickness ( i, j, segment ).AddPoint ( 0.0, segmentThickness );
            formation->getRealThickness  ( i, j, segment ).AddPoint ( 0.0, segmentThickness );
         }

      }

      return true;
   } else {

      unsigned int segment;
      double age;
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
               cout << " MeSsAgE ERROR negative mobile layer thickness detected in formation '" << formation->getName () << "' at position (" << i << ", " << j  << ").  " << formation->getInputThicknessMap ()->getValue ( i, j ) << endl;
            }

         }

      }

      for ( segment = 0; segment < formation->getMaximumNumberOfElements (); ++segment ) {
         formation->getSolidThickness ( i, j, segment ).AddPoint ( 0.0, segmentThickness );
         formation->getRealThickness  ( i, j, segment ).AddPoint ( 0.0, segmentThickness );
      }

      if ( formation->isMobileLayer ()) {
         Interface::MobileLayerList::const_iterator mapIter;
         Interface::MobileLayerList* mobileLayerThicknesses = formation->getMobileLayers ();

         for ( mapIter = mobileLayerThicknesses->begin (); mapIter != mobileLayerThicknesses->end (); ++mapIter ) {
            age = (*mapIter)->getSnapshot ()->getTime ();

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
                                                                            GeoPhysics::Formation* formation,
                                                                            IntegerArray& numberOfErrorsPerLayer ) {

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
   // this can be found in DataAccess/src/Interface/interface/h
   //

   if ( not formation->getIsIgneousIntrusion ()) {
      // If the formation is not an igneous intrusion then should not be in this function.
      return false;
   }

   bool onlyPositiveThickness = true;

   double endOfIntrusionAge;
   double startOfIntrusionAge;
   // The age at which the igneous intrusion formation appears in the strat table.
   double formationInclusionAge;

   unsigned int segment;

   double segmentThickness = formation->getInputThicknessMap ()->getValue ( i, j ) / double ( formation->getMaximumNumberOfElements ());

   if ( segmentThickness < 0.0 ) {
      onlyPositiveThickness = false;

      if ( formation->getDepositionSequence () > 0 and formation->getDepositionSequence () != Interface::DefaultUndefinedScalarValue ) {
         ++numberOfErrorsPerLayer [ formation->getDepositionSequence () - 1 ];

         if ( numberOfErrorsPerLayer [ formation->getDepositionSequence () - 1 ] <= MaximumNumberOfErrorsPerLayer ) {
            cout << " MeSsAgE ERROR negative igneous intrusion thickness detected in formation '" << formation->getName () << "' at position (" << i << ", " << j  << ")." << endl;
         }

      }

   }

   const Interface::IgneousIntrusionEvent* intrusionEvent = formation->getIgneousIntrusionEvent ();

   endOfIntrusionAge = intrusionEvent->getEndOfIntrusion ()->getTime ();
   startOfIntrusionAge = intrusionEvent->getStartOfIntrusion ();
   formationInclusionAge = formation->getTopSurface ()->getSnapshot ()->getTime ();

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
      double thinningFactor = 1.0 - getContCrustThickness( i, j, age ) / getCrustFormation()->getInitialCrustalThickness();
      double initLithoThickness = getMantleFormation ()->getInitialLithosphericMantleThickness () +
                                  getCrustFormation()->getInitialCrustalThickness();

      const double HLmod = 0.5 * ( ( initLithoThickness + m_minimumLithosphereThickness ) + ( initLithoThickness - m_minimumLithosphereThickness ) * cos ( M_PI * thinningFactor ));
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
                                                           const bool     overpressureCalculation,
                                                           GeoPhysics::Formation* formation,
                                                           int& nrActUnc,
                                                           FloatStack &uncMaxVes,
                                                           FloatStack &uncThickness ) {

   // we have 3 possible types of layer: 1: a mobile layer, 2: a normal
   // sedimented layer or 3: an erosion (layer).

   if ( formation->isMobileLayer () or formation->getIsIgneousIntrusion ()) {
      // mobile layer or igneous intrusion!!
      return updateMobileLayerOrIgneousIntrusionMaxVes (i, j, formation, uncMaxVes.front ());
   } else  {
      // get the thickness of the layer at this point to
      // determine whether the layer is depositing or eroding.
      // for a non mobile layer there will only be one thickness

      double thickness = formation->getInputThicknessMap ()->getValue ( i, j );

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
                                                                            GeoPhysics::Formation* formation,
                                                                            double &maxVes ) {

  bool result = true;
  double segmentThickness;
  double dummy;
  double diffdensity = 0.0;

  int segment;

  const CompoundLithology* lithology = formation->getCompoundLithology ( i, j );
  const GeoPhysics::FluidType* fluid = dynamic_cast<const GeoPhysics::FluidType*>( formation->getFluidType ());

  for ( segment = formation->getMaximumNumberOfElements() - 1; segment >= 0; --segment )
  {

      segmentThickness = formation->getSolidThickness ( i, j, (unsigned int)(segment)).MaxY (dummy);
      assert( segmentThickness != Interface::DefaultUndefinedScalarValue );

      if ( fluid != 0  )
      {
         diffdensity = lithology->density () - fluid->getConstantDensity ();

         bool switchPermaFrost = fluid->isPermafrostEnabled();
         double surfacePorosity = lithology->surfacePorosity( );

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
                                                               const bool     overpressureCalculation,
                                                                     GeoPhysics::Formation* formation,
                                                                     FloatStack &uncMaxVes,
                                                                     FloatStack &uncThickness,
                                                                const int nrActUnc ) {

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
   double compThickness = 0.;
   double fullCompThickness = 0.;
   double totalFCT = 0.;
   double startDepoTime = 0.0, endDepoTime = 1.e10;

   int numberOfUnconformities = nrActUnc;

   // loop through all the segments in this layer
   int segmentNr = 0;

   for (segmentNr = formation->getMaximumNumberOfElements () - 1; segmentNr >= 0; segmentNr--)
   {

      // create an empty polynomial function
      CBMGenerics::Polyfunction *oldPolyf = new Polyfunction;
      CBMGenerics::Polyfunction *newPolyf = &(formation->getSolidThickness (i, j, segmentNr));

      // set the old polynomial function up and clear the new one
      (*newPolyf).swap (*oldPolyf);

      // set an iterator to point to the oldest point in the function
      Polyfunction::Polypoint::const_reverse_iterator oldPolyfIter = oldPolyf->getRBegin ();

      if (oldPolyf->getREnd () == oldPolyfIter)
      {
         cout << "poly function is empty" << endl;
      }

      // save the start of deposition point as we will add this
      // point to the function last.
      Polyfunction::Polypoint::const_reverse_iterator startOfDeposition = oldPolyfIter;

      assert ((*startOfDeposition)->getY () == 0);

      double thisStartDepoTime = (*startOfDeposition)->getX ();

      startDepoTime = NumericFunctions::Maximum (startDepoTime, thisStartDepoTime);
      Polyfunction::Polypoint::const_reverse_iterator endOfEvent = ++oldPolyfIter;
      Polyfunction::Polypoint::const_reverse_iterator endOfPrevEvent = endOfEvent;
      double thisEndDepoTime = (*endOfEvent)->getX ();

      endDepoTime = NumericFunctions::Minimum (endDepoTime, thisEndDepoTime);

      // this loop will only be executed when we have an erosion
      Polyfunction::Polypoint::const_reverse_iterator startOfErosion;
      while (true)
      {

         // move to the next point
         startOfErosion = ++oldPolyfIter;
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
            cout << "Could not compute FCT" << endl;
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
            double tmpThickness = NumericFunctions::Minimum (uncThickness.front (), remThickness);

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
      bool added = newPolyf->AddPoint ((*endOfEvent)->getX (), 0.0);

      compThickness = (*endOfEvent)->getY ();
      result &= calcFullCompactedThickness (i, j, overpressureCalculation, formation, compThickness, uncMaxVes.front (), fullCompThickness, (*endOfEvent)->getX() );
      if (!result)
      {
         cout << "Could not compute FCT (2)" << endl;
         delete oldPolyf;
         return false;
      }

      totalFCT += fullCompThickness;

      newPolyf->RaiseBy (fullCompThickness);
      added = newPolyf->AddPoint ((*startOfDeposition)->getX (), 0.0);

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
   for (segmentNr = formation->getMaximumNumberOfElements () - 1; segmentNr >= 0; segmentNr--)
   {

      Polyfunction *polyf;

      polyf = &(formation->getSolidThickness (i, j, segmentNr));
      Polyfunction::Polypoint::const_reverse_iterator polyfit = polyf->getRBegin ();

      Polyfunction::Polypoint::const_reverse_iterator startDepoPoint = polyfit;
      Polyfunction::Polypoint::const_reverse_iterator endDepoPoint = ++polyfit;

      double segmFCT = (*endDepoPoint)->getY ();

      assert (0 == (*startDepoPoint)->getY ());
      assert (0 != totalFCT);

      double segmStartDepoTime = endDepoTime - (endDepoTime - startDepoTime) * (segmFCT / totalFCT);

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
   GeoPhysics::Formation* formation,
   const double compThickness,
   double &uncMaxVes,
   double &fullCompThickness,
   double age ) {

   bool result = true;

   const CompoundLithology* lithology = formation->getCompoundLithologyArray()( i, j, age );
   const GeoPhysics::FluidType* fluid = dynamic_cast<const GeoPhysics::FluidType*>( formation->getFluidType() );

   double fluidDensity = 0.0;
   double lithologyDensity = lithology->density();
   double densityDifference = 0.0;

   if ( fluid != 0 )
   {
      fluidDensity = fluid->getConstantDensity( );
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

bool GeoPhysics::ProjectHandle::applyFctCorrections () {

   const Interface::Property* fctCorrectionProperty = findProperty ( "FCTCorrection" );
   assert ( fctCorrectionProperty != 0 );

   const Interface::Snapshot* presentDataSnapshot = findSnapshot ( 0.0, Interface::MAJOR );
   assert ( presentDataSnapshot != 0 );

   // Get the FCTCorrectionproperty from the results file.
   Interface::PropertyValueList* solidThicknessCorrections = getPropertyValues ( Interface::FORMATION,
                                                                                 fctCorrectionProperty,
                                                                                 presentDataSnapshot,
                                                                                 0, 0, 0,
                                                                                 Interface::MAP );

   Interface::PropertyValueList::const_iterator fctIter;
   unsigned int i;
   unsigned int j;
   unsigned int k;
   unsigned int numberOfElements;

   for ( fctIter = solidThicknessCorrections->begin (); fctIter != solidThicknessCorrections->end (); ++fctIter ) {

      const Interface::PropertyValue* fct = *fctIter;

      GeoPhysics::Formation* formation = const_cast<GeoPhysics::Formation*>(dynamic_cast<const GeoPhysics::Formation*>( fct->getFormation ()));

      if ( formation->kind () == Interface::SEDIMENT_FORMATION ) {
         const Interface::GridMap* fctMap = dynamic_cast<const Interface::GridMap*>(fct->getGridMap ());

         fctMap->retrieveGhostedData ();

         numberOfElements = formation->getMaximumNumberOfElements ();

         for ( i = firstI ( true ); i <= lastI ( true ); ++i ) {

            for ( j = firstJ ( true ); j <= lastJ ( true ); ++j ) {

               for ( k = 0; k < numberOfElements; ++k ) {
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

void GeoPhysics::ProjectHandle::printValidNeedles ( std::ostream& o  ) {

   unsigned int i;
   unsigned int j;
   int proc;

   std::ostringstream msg;
   msg << endl;
   msg << endl;
   msg << "--------------------------------" << endl;
   msg << endl;
   msg << endl;


   for ( proc = 0; proc < getSize (); ++proc ) {

      if ( proc == getRank () ) {
         msg << " process " << proc << endl;

         for ( i = firstI ( true ); i <= lastI ( true ); ++i ) {

            for ( j = firstJ ( true ); j <= lastJ ( true ); ++j ) {
               msg << ( m_validNodes ( i, j ) ? 'T' : 'F' );
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

   m_basementLithoProps = 0;
   if( m_isALCMode ) {
      char * ALC_ConfigurationFile     = getenv ( "CTCDIR" );
      char * ALC_UserConfigurationFile = getenv ( "MY_CTCDIR" );

      string fullpath;

      if( ALC_UserConfigurationFile != 0 ) {
         ibs::Path fp( ALC_UserConfigurationFile );
         fp << cfgFileName;
         fullpath = fp.path();
      } else if( ALC_ConfigurationFile != 0 ) {
         ibs::FilePath fp( ALC_ConfigurationFile );
         fp << cfgFileName;
         fullpath = fp.path();
      } else {
         std::cout<< "MeSsAgE WARNING: Environment Variable CTCDIR is not set." << std::endl;;
         return false;
      }

      ifstream  ConfigurationFile;
      ConfigurationFile.open( fullpath );

      if (!ConfigurationFile) {
         getMessageHandler().printLine( "MeSsAgE ERROR Attempting to open file : " + fullpath + "\nNo cfg file available in the $CTCDIR directory... Aborting..." );
         throw RecordException( "MeSsAgE ERROR Attempting to open file : " + fullpath + "\nNo cfg file available in the $CTCDIR directory... Aborting..." );
      }

      m_basementLithoProps = new ConfigFileParameterAlc();
      if( !m_basementLithoProps->loadConfigurationFileAlc ( ConfigurationFile )) {
         ConfigurationFile.close();
         return false;
      };
      
      m_minimumLithosphereThickness   = m_basementLithoProps->m_HLmin;
      m_maximumNumberOfMantleElements = m_basementLithoProps->m_NLMEmax;
      m_constrainedBasaltTemperature  = m_basementLithoProps->m_bT;

      ConfigurationFile.close();
      return true;
   }
   return true;
}

//------------------------------------------------------------//
bool GeoPhysics::ProjectHandle::hasSurfaceDepthHistory( const double age ) const{
   for (std::size_t i = 0; i < m_surfaceDepthHistory.size(); i++){
      if (m_surfaceDepthHistory[i]->getSnapshot()->getTime() == age){
         return true;
      }
   }
   return false;
}