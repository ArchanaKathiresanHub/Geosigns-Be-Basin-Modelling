#include "PropertiesIoTbl.h"

#include <iostream>
#include <string>

#include <assert.h>

#include "utils.h"
#include "globaldefs.h"

using namespace std;

#include "database.h"
#include "cauldronschemafuncs.h"
using namespace database;

#include "layer_iterators.h"
using namespace Basin_Modelling;

#include "CrustFormation.h"
#include "MantleFormation.h"
#include "HydraulicFracturingManager.h"

// Data access library
#include "Interface/BasementSurface.h"
#include "Interface/ConstrainedOverpressureInterval.h"
#include "Interface/CrustFormation.h"
#include "Interface/FluidType.h"
#include "Interface/Formation.h"
#include "Interface/FracturePressureFunctionParameters.h"
#include "Interface/GridMap.h"
#include "Interface/MantleFormation.h"
#include "Interface/MobileLayer.h"
#include "Interface/OutputProperty.h"
#include "Interface/PaleoFormationProperty.h"
#include "Interface/PaleoProperty.h"
#include "Interface/PaleoSurfaceProperty.h"
#include "Interface/ProjectData.h"
#include "Interface/RunParameters.h"
#include "Interface/Snapshot.h"
#include "Interface/Surface.h"

#include "Interface/Interface.h"

using namespace DataAccess;


bool StratIoTbl::writeToContext( AppCtx* Basin_Model )
{


   LayerProps *pNewLayer;
   Interface::FormationList* formations = FastcauldronSimulator::getInstance ().getFormations ();

   if ( formations != 0 ) {

      Interface::FormationList::const_iterator formationIter;
      int count;

      for ( count = 0, formationIter = formations->begin (); formationIter != formations->end (); ++formationIter, ++count ) {

         const Interface::Formation* formation = *formationIter;

         if ( formation->kind () == Interface::SEDIMENT_FORMATION ) {

            pNewLayer = const_cast<LayerProps*>(dynamic_cast<const LayerProps*>(formation));

            // Create and Initialise Current Day Thickness Map
            pNewLayer->fluid = Basin_Model->findFluid ( formation->getFluidType ()->getName ());

            Basin_Model->layers.push_back (pNewLayer);
         }

      }

   }

   const Interface::CrustFormation* crustFormation = FastcauldronSimulator::getInstance ().getCrustFormation ();

   // Create and Initialise Current Day Thickness Map
   CrustFormation* crust = const_cast<CrustFormation*>(dynamic_cast<const CrustFormation*>(crustFormation));
   Basin_Model->layers.push_back ( crust );
   Basin_Model->Set_Crust_Layer ( crust );

   delete formations;
   return true;
}

bool RunOptionsIoTbl::writeToContext( AppCtx* Basin_Model )
{

   const Interface::RunParameters* runParameters = FastcauldronSimulator::getInstance ().getRunParameters ();

   Basin_Model->timestepsize = runParameters->getPrefReconstep ();
   Basin_Model->optimalpressdiff = runParameters->getOptimalTotalPresDiff ();
   Basin_Model->optimaltempdiff = runParameters->getOptimalTotalTempDiff ();
   Basin_Model->optimalsrtempdiff = runParameters->getOptimalSourceRockTempDiff ();
   Basin_Model->timestepincr = runParameters->getMaxTimeStepIncreaseFactor ();
   Basin_Model->timestepdecr = runParameters->getMinTimeStepDecreaseFactor ();
   Basin_Model->MaxNumberOfRunOverpressure = runParameters->getMaxNumberOfRunOverpressure ();
   Basin_Model->Temperature_Gradient = runParameters->getTemperatureGradient () * 0.001;

   Basin_Model->includeAdvectiveTerm = runParameters->getConvectiveTerm ();
   Basin_Model->Do_Chemical_Compaction  = runParameters->getChemicalCompaction ();

   // Notice the NOT (!) here. The field in the project file is for the NON Geometric loop.
   // but the field in the Basin Model object is Geometric Loop.
   Basin_Model->Use_Geometric_Loop = not runParameters->getNonGeometricLoop ();
   Basin_Model->Optimisation_Level = runParameters->getOptimisationLevel ();


   return true;
}

bool SnapshotIoTbl::writeToContext( AppCtx* Basin_Model )
{

   const Interface::Snapshot* oldestSnapshot = FastcauldronSimulator::getInstance ().getCrustFormation ()->getTopSurface ()->getSnapshot ();
   assert ( oldestSnapshot != 0 );

   Interface::SnapshotList* snapshots = FastcauldronSimulator::getInstance ().getSnapshots ( Interface::MAJOR | Interface::MINOR );
   Interface::SnapshotList::const_iterator snapshotIter;

   for ( snapshotIter = snapshots->begin (); snapshotIter != snapshots->end (); ++snapshotIter ) {
      const Interface::Snapshot* snapshot = *snapshotIter;

      if ( snapshot->getTime () <= oldestSnapshot->getTime ()) {
         // only add snapsthots to fastcauldron if they are the same age or younger than the bottom-most surface age.
         const std::string& snapshotFileName = snapshot->getFileName ();
         const std::string& snapshotType     = snapshot->getKind ();
         bool isMinorSnapshot = snapshot->getType () == Interface::MINOR;

         Basin_Model->projectSnapshots.addSnapshotEntry ( snapshot->getTime (), isMinorSnapshot, snapshotType, snapshotFileName );
      }

   }

   delete snapshots;

   return true;
}

bool BasementIoTbl::writeToContext( AppCtx* Basin_Model )
{
   // Create Mantle in layers Vector 
   //->Will be added to the Layers List later


   const Interface::MantleFormation* mantleFormation = FastcauldronSimulator::getInstance ().getMantleFormation ();

   // Create and Initialise Current Day Thickness Map
   MantleFormation* pNewLayer = const_cast<MantleFormation*>(dynamic_cast<const MantleFormation*>(mantleFormation));
   
   Basin_Model->Set_Mantle_Layer ( pNewLayer );

#if 0
   cout << " getMantleFormation ()->getPresentDayThickness " 
        << FastcauldronSimulator::getInstance ().getMantleFormation ()->getPresentDayThickness () << endl;
#endif

   // Add Basement at the bottom of the Layers List
   Basin_Model->layers.push_back ( Basin_Model->Mantle() );
   return true;
}

bool FilterTimeIoTbl::writeToContext( AppCtx* Basin_Model )
{

   int i;

   for ( i = 0; i < FastcauldronSimulator::getInstance ().getSize (); ++i ) {

      if ( i == FastcauldronSimulator::getInstance ().getRank ()) {

         Interface::OutputPropertyList* outputProperties = FastcauldronSimulator::getInstance ().getTimeOutputProperties ();
         Interface::OutputPropertyList::const_iterator propertyIter;

         for ( propertyIter = outputProperties->begin (); propertyIter != outputProperties->end (); ++propertyIter ) {

#if 0
            cout << " output property: " << (*propertyIter)->getMode () << "  " 
                 << (*propertyIter)->getName () << "  " << (*propertyIter)->getOption () << endl;
#endif

            if (((*propertyIter)->getMode () == Interface::MODE3D and Basin_Model->isModellingMode3D ()) or
                ((*propertyIter)->getMode () == Interface::MODE1D and Basin_Model->isModellingMode1D ())) {
               Basin_Model->timefilter.setFilter ( (*propertyIter)->getName (), (*propertyIter)->getOption ());
            }

         }

         delete outputProperties;
      }

   }

   return true;
}

bool RelatedProjectsIoTbl::writeToContext( AppCtx* Basin_Model )
{

   Interface::RelatedProjectList* relatedProjects = FastcauldronSimulator::getInstance ().getRelatedProjectList ();
   Interface::RelatedProjectList::const_iterator relatedProjectIter;

   for ( relatedProjectIter = relatedProjects->begin (); relatedProjectIter != relatedProjects->end (); ++relatedProjectIter ) {
      const Interface::RelatedProject* relatedProject = *relatedProjectIter;

      Basin_Model->setRelatedProject ( relatedProject );
   }
   
   delete relatedProjects;
   return true;
}

