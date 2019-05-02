#include <iostream>
using namespace std;

#include <algorithm>
#include "database.h"
#include "cauldronschema.h"
#include "cauldronschemafuncs.h"
using namespace database;

#include "GenexSimulator.h"
#include "SourceRock.h"
using namespace GenexSimulation;

#include "h5_parallel_file_types.h"

#include "Interface/Interface.h"
#include "Interface/ProjectHandle.h"
#include "Interface/ObjectFactory.h"
#include "Interface/SourceRock.h"
#include "Interface/Property.h"
#include "Interface/Snapshot.h"
#include "Interface/Formation.h"
#include "Interface/Surface.h"
#include "Interface/LithoType.h"
#include "Interface/PropertyValue.h"
#include "Interface/SimulationDetails.h"
using namespace DataAccess;

#include "ComponentManager.h"
#include "GenexResultManager.h"

#include "FilePath.h"

#ifndef _MSC_VER
#include "h5merge.h"
#endif

using namespace Genex6;

void displayTime ( const double timeToDisplay, const char * msgToDisplay ) {

   int hours   = (int)(  timeToDisplay / 3600.0 );
   int minutes = (int)(( timeToDisplay - (hours * 3600.0) ) / 60.0 );
   int seconds = (int)(  timeToDisplay - hours * 3600.0 - minutes * 60.0 );

   PetscPrintf ( PETSC_COMM_WORLD, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ \n" );
   PetscPrintf ( PETSC_COMM_WORLD, "%s: %d hours %d minutes %d seconds\n", msgToDisplay, hours, minutes, seconds );
   PetscPrintf ( PETSC_COMM_WORLD, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ \n" );

}

GenexSimulator::GenexSimulator (database::ProjectFileHandlerPtr database, const std::string & name, const std::string & accessMode, DataAccess::Interface::ObjectFactory* objectFactory)
   : GeoPhysics::ProjectHandle (database, name, accessMode, objectFactory)
{
  registerProperties();
  m_propertyManager = new DerivedProperties::DerivedPropertyManager ( this );
}

GenexSimulator::~GenexSimulator (void)
{
   m_registeredProperties.clear();
   m_requestedProperties.clear();
   m_expelledToCarrierBedProperties.clear ();
   m_expelledToSourceRockProperties.clear ();
   m_expelledToCarrierBedPropertiesS.clear ();
   m_expelledToSourceRockPropertiesS.clear ();
   delete m_propertyManager;

}

GenexSimulator *GenexSimulator::CreateFrom(const std::string & inputFileName, DataAccess::Interface::ObjectFactory* objectFactory)
{
	return (GenexSimulator *) Interface::OpenCauldronProject (inputFileName, "rw", objectFactory);
}

bool GenexSimulator::saveTo(const std::string & outputFileName)
{
   return saveToFile (outputFileName);
}

bool GenexSimulator::run()
{
   PetscBool isGenex7 = PETSC_FALSE;
   PetscOptionsHasName( PETSC_NULL, "-genex7", &isGenex7 );

   PetscBool isBG = PETSC_FALSE;
   PetscOptionsHasName( PETSC_NULL, "-bg", &isBG ); //compute biogenic gas

   PetscBool isTestAds = PETSC_FALSE;
   PetscOptionsHasName( PETSC_NULL, "-testads", &isTestAds );


   bool started = startActivity ( GenexActivityName, getLowResolutionOutputGrid () );
   
   if (!started) return false;
   
   const Interface::SimulationDetails* simulationDetails = getDetailsOfLastSimulation ( "fastcauldron" );

   bool coupledCalculation = simulationDetails != 0 and ( simulationDetails->getSimulatorMode () == "CoupledPressureAndTemperature" or
                                                          simulationDetails->getSimulatorMode () == "CoupledHighResDecompaction" or
                                                          simulationDetails->getSimulatorMode () == "LooselyCoupledTemperature" or
                                                          simulationDetails->getSimulatorMode () == "CoupledDarcy" );
   coupledCalculation = coupledCalculation and getModellingMode() == Interface::MODE3D;

   started =  GeoPhysics::ProjectHandle::initialise ( coupledCalculation );
   if (!started) return false;

   started = setFormationLithologies ( false, true );
   if (!started) return false;

   started = initialiseLayerThicknessHistory ( coupledCalculation );
   if (!started) return false;

   setRequestedOutputProperties( isBG );

   bool useFormationName = false;

   PetscBool minorSnapshots;

   PetscOptionsHasName (PETSC_NULL, "-minor", &minorSnapshots);

   PetscLogDouble run_Start_Time;
   PetscTime(&run_Start_Time);

#if 0
   std::vector<Interface::SourceRock*>::iterator sourceRockIter;
   Interface::Formation * theFormation;

   for (sourceRockIter = m_sourceRocks.begin(); sourceRockIter != m_sourceRocks.end(); ++ sourceRockIter) {
      SourceRock * sr = dynamic_cast<Genex6::SourceRock *>( *sourceRockIter );

      if( !sr->getLayerName().empty() ) {
         useFormationName = true;
         const Interface::Formation * theFormation = findFormation( sr->getLayerName() );

         if( theFormation != 0 && theFormation->isSourceRock() ) {
            if( !computeSourceRock ( sr, theFormation )) return false;
         }
      }
   }
#endif

   if( !useFormationName ) {

      // double depoAge = ( isBG ? findYoungestSR() : 0.0 );

      std::vector<Interface::Formation*>::iterator formationIter;

      for (formationIter = m_formations.begin(); formationIter != m_formations.end(); ++ formationIter) {

         if(started) {
            Interface::Formation * theFormation = dynamic_cast<Interface::Formation*>( *formationIter );

            bool computeGenex = false;

            if( isBG and not theFormation->isSourceRock() ) {
               computeGenex = createBiogenicGasSR( theFormation );
            }
            
            if( theFormation != 0 && ( theFormation->isSourceRock() or computeGenex )) {
               
               Genex6::SourceRock* sr = (Genex6::SourceRock *)( theFormation->getSourceRock1() ); 

               if( not sr->isSulphur() and isGenex7 ) {
                  sr->setGenex7 ( isGenex7 );
               }
               sr->setTestAds ( isTestAds );
               sr->setMinor (bool (minorSnapshots));
               if( isBG ) {
                  sr->setBiogenicGasOnly( computeGenex ); 

                  // if(( theFormation->isSourceRock() and depoAge == getDepositionTime( theFormation )) or computeGenex ) {
                  if(( theFormation->isSourceRock() and theFormation->isBiogenicGasLayer() ) or computeGenex ) {
                     setRequestedOutputPropertiesBG( true ); 
                  } else {
                     setRequestedOutputPropertiesBG( false ); 
                  }          
               }
     
               started = computeSourceRock( sr, theFormation );

               if( computeGenex ) {
                  removeBiogenicGasSR( theFormation );
               }
            }
         } else {
            finishActivity ();
            return false;
         }
      }
      if( !started ) {
         finishActivity ();
         return false;
      }
   }

   PetscLogDouble run_End_Time;
   PetscTime(&run_End_Time);

   if(started) {
      displayTime ( run_End_Time - run_Start_Time, "Simulation Time" );
      PetscPrintf ( PETSC_COMM_WORLD, "Saving Genex6results to disk. Please wait... \n" );
      PetscPrintf ( PETSC_COMM_WORLD, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ \n" );
   }

   finishActivity ();
   setSimulationDetails ( "fastgenex", "Default", "" );

   if( !mergeOutputFiles ()) {
      PetscPrintf ( PETSC_COMM_WORLD, "Basin_Error: Unable to merge output files\n");

      started = 0;
   }

   if(started) {
      PetscLogDouble sim_End_Time;
      PetscTime(&sim_End_Time);

      displayTime ( sim_End_Time - run_Start_Time, "Results saved sucessfully" );
   }

   return started;
}

double GenexSimulator::findYoungestSR() {

   double depoAge = 99999;
   std::vector<Interface::Formation*>::iterator formationIter;
   
   for (formationIter = m_formations.begin(); formationIter != m_formations.end(); ++ formationIter) {
      
      Interface::Formation * theFormation = dynamic_cast<Interface::Formation*>( *formationIter );
      if( theFormation != 0 and theFormation->isSourceRock() ) {
         const double formationDepoAge = getDepositionTime( theFormation );
         if( depoAge > formationDepoAge ) {
            depoAge = formationDepoAge;
         }
      }
   }
   return depoAge;
}

double GenexSimulator::getDepositionTime( Interface::Formation * formation ) {

   if( formation->kind() == Interface::SEDIMENT_FORMATION and formation->getTopSurface() ) {
      return formation->getTopSurface()->getSnapshot()->getTime();
   }
   return -1;

}

void GenexSimulator::removeBiogenicGasSR( Interface::Formation * bgFormation ) {

   database::Table* sourceRockLithoIoTbl = getTable( "SourceRockLithoIoTbl" );
 
   Record * sourceRockRecord = sourceRockLithoIoTbl->findRecord( "SourceRockType",  "Type_II_Mesozoic_MarineShale_kin",
                                                                 "LayerName", bgFormation->getName() );
   if( sourceRockRecord != 0 ) {
      sourceRockLithoIoTbl->eraseRecord( sourceRockRecord );
   }
   bgFormation->setSourceRock1( 0 );
}

bool GenexSimulator::createBiogenicGasSR( Interface::Formation * bgFormation ) {

   if( bgFormation->kind() == Interface::SEDIMENT_FORMATION and bgFormation->isBiogenicGasLayer() and 
       bgFormation->getTopSurface() and bgFormation->getTopSurface()->getSnapshot()->getTime() > 0 ) {

      database::Table* sourceRockLithoIoTbl = getTable( "SourceRockLithoIoTbl" );
      Interface::SourceRock * bgSourceRock = 0;
      database::Table::iterator tblIter;
      
      Record * sourceRockRecord = sourceRockLithoIoTbl->createRecord( true );
      
      database::setLayerName( sourceRockRecord, bgFormation->getName() );
      database::setSourceRockType( sourceRockRecord, "Type_II_Mesozoic_MarineShale_kin" );
	  // database::setBaseSourceRockType(sourceRockRecord, bgFormation->getSourceRockType());
      database::setHcVRe05( sourceRockRecord, 1.25 );
      database::setPreAsphaltStartAct( sourceRockRecord, 210.0 );
      database::setResinDiffusionEnergy( sourceRockRecord, 85.0 );
      database::setC15AroDiffusionEnergy( sourceRockRecord, 80.0 );
      database::setC15SatDiffusionEnergy( sourceRockRecord, 75.0 );
      database::setAsphalteneDiffusionEnergy( sourceRockRecord, 88.0 );
      database::setTocIni( sourceRockRecord, bgFormation->getToc() );
      bgSourceRock = getFactory()->produceSourceRock( this, sourceRockRecord );
      
      bgFormation->setSourceRock1( bgSourceRock );
      
      return true;
   }
   return false;
}

bool GenexSimulator::computeSourceRock ( Genex6::SourceRock * aSourceRock, const Interface::Formation * aFormation )
{
   if( aSourceRock != 0 ) {
      aSourceRock->clear();
      aSourceRock->setPropertyManager ( m_propertyManager );
      aSourceRock->setFormationData( aFormation ); // set layerName, formation, second SR type, mixing parameters, isSulphur

      bool isSulphur = aSourceRock->isSulphur();
      if ( aSourceRock->doOutputAdsorptionProperties ()) {
         if( isSulphur ) {
            aSourceRock->initializeSnapShotOutputMaps ( m_expelledToSourceRockPropertiesS, m_requestedProperties );
         } else {
            aSourceRock->initializeSnapShotOutputMaps ( m_expelledToSourceRockProperties, m_requestedProperties );
         }
      } else {
         if( isSulphur ) {
            aSourceRock->initializeSnapShotOutputMaps ( m_expelledToCarrierBedPropertiesS, m_requestedProperties );
         } else {
            aSourceRock->initializeSnapShotOutputMaps ( m_expelledToCarrierBedProperties, m_requestedProperties );
         }
      }

      return aSourceRock->compute();
   } else {
      cerr << "Error in the project file: cannot find Source Rock reference in formation " << aFormation->getName() << endl;
      finishActivity();
      return false;
   }
}

void GenexSimulator::setRequestedSpeciesOutputProperties()
{
   using namespace CBMGenerics;
   ComponentManager & theManager = ComponentManager::getInstance();

   for (int i = 0; i < ComponentManager::NUMBER_OF_SPECIES; ++i ) {
      if(!theManager.isSulphurComponent(i)) {
         m_expelledToCarrierBedProperties.push_back ( theManager.getSpeciesOutputPropertyName ( i, false ));
         m_expelledToSourceRockProperties.push_back ( theManager.getSpeciesOutputPropertyName ( i, true ));
      }
      m_expelledToCarrierBedPropertiesS.push_back ( theManager.getSpeciesOutputPropertyName ( i, false ));
      m_expelledToSourceRockPropertiesS.push_back ( theManager.getSpeciesOutputPropertyName ( i, true ));
   }

   sort ( m_expelledToSourceRockProperties.begin (), m_expelledToSourceRockProperties.end ());
   sort ( m_expelledToCarrierBedProperties.begin (), m_expelledToCarrierBedProperties.end ());
   sort ( m_expelledToSourceRockPropertiesS.begin (), m_expelledToSourceRockPropertiesS.end ());
   sort ( m_expelledToCarrierBedPropertiesS.begin (), m_expelledToCarrierBedPropertiesS.end ());
}

void GenexSimulator::setRequestedOutputPropertiesBG( const bool isBG )
{
   using namespace CBMGenerics;
   GenexResultManager & theResultManager = GenexResultManager::getInstance();
   theResultManager.SetResultToggleByResId( GenexResultManager::FluxOA1, isBG );
   theResultManager.SetResultToggleByResId( GenexResultManager::FluxOA2, isBG );
}

void GenexSimulator::setRequestedOutputProperties( const bool isBG )
{
   //cumullative expulsion mass for all species is required
   setRequestedSpeciesOutputProperties();

   using namespace CBMGenerics;
   GenexResultManager & theResultManager = GenexResultManager::getInstance();

   Interface::ModellingMode theMode = getModellingMode();
   string theModellingMode = "3d";

   if( Interface::MODE1D == theMode ) {
      theModellingMode = "1d";
   }
   bool doOutputGenexHistory = ( m_adsorptionPointHistoryList.size() != 0 ? true : false );

   Table * timeIoTbl = getTable ("FilterTimeIoTbl");
   Table::iterator tblIter;


   for (tblIter = timeIoTbl->begin (); tblIter != timeIoTbl->end (); ++ tblIter) {
      Record * filterTimeIoRecord = * tblIter;
      const string & outPutOption = database::getOutputOption(filterTimeIoRecord);
      const string & modellingMode = database::getModellingMode(filterTimeIoRecord);
      const string & propertyName = database::getPropertyName (filterTimeIoRecord);

      if(outPutOption != "None" && modellingMode == theModellingMode) {

         if(isPropertyRegistered(propertyName)) {
            if( theResultManager.getResultId ( propertyName ) != -1 ) {

               if( not (( propertyName == "FluxOA1" or propertyName == "FluxOA2" ) and not isBG )) {                  
                  m_requestedProperties.push_back(propertyName);
                  theResultManager.SetResultToggleByName(propertyName, true);
               }
            } else if( propertyName == "SourceRockEndMember1" or propertyName == "SourceRockEndMember2" or propertyName == "TOC" ) {
               m_requestedProperties.push_back(propertyName);
            }
         }

      } else if ((( propertyName == theResultManager.GetResultName(GenexResultManager::HcGasExpelledCum)) or
                  ( propertyName == theResultManager.GetResultName(GenexResultManager::OilExpelledCum))) and
                 modellingMode == theModellingMode) {

         if( isPropertyRegistered(propertyName)) {
            m_requestedProperties.push_back(propertyName);
            theResultManager.SetResultToggleByName(propertyName, true);
         }

      } else if ( doOutputGenexHistory && modellingMode == theModellingMode ) { // calculate (not output) all if output history

         if(isPropertyRegistered(propertyName) and theResultManager.getResultId ( propertyName ) != -1 ) {
            theResultManager.SetResultToggleByName(propertyName, true);
         }
      }
   }
   // Explicitly add biogenic gas output
   if( isBG ) {
      string pName = "FluxOA1";
      std::vector<string>::iterator it = std::find( m_requestedProperties.begin(), m_requestedProperties.end(), pName );
      if( it == m_requestedProperties.end() ) {
         m_requestedProperties.push_back(pName);
         theResultManager.SetResultToggleByName(pName, true);
      }
      pName = "FluxOA2";
      it = std::find( m_requestedProperties.begin(), m_requestedProperties.end(), pName );
      if( it == m_requestedProperties.end() ) {
         m_requestedProperties.push_back(pName);
         theResultManager.SetResultToggleByName(pName, true);
      }
   }
   sort(m_requestedProperties.begin(), m_requestedProperties.end());
}
void GenexSimulator::registerProperties()
{
   using namespace CBMGenerics;
   ComponentManager & theManager = ComponentManager::getInstance();
   GenexResultManager & theResultManager = GenexResultManager::getInstance();

   int i;

   for (i = 0; i < ComponentManager::NUMBER_OF_SPECIES; ++i ) {
      m_registeredProperties.push_back(theManager.getSpeciesOutputPropertyName ( i, false ));
   }

   for (i = 0; i < ComponentManager::NUMBER_OF_SPECIES; ++i ) {
      m_registeredProperties.push_back(theManager.getSpeciesOutputPropertyName ( i, true ));
   }

   for(i = 0; i < GenexResultManager::NumberOfResults; ++ i) {
      m_registeredProperties.push_back (theResultManager.GetResultName(i));
   }

   // Adding all possible species that can be used in the adsorption process.
   for (i = 0; i < ComponentManager::NUMBER_OF_SPECIES; ++i)
   {
      m_registeredProperties.push_back ( theManager.getSpeciesName ( i ) + "Retained" );
      m_registeredProperties.push_back ( theManager.getSpeciesName ( i ) + "Adsorped" );
      m_registeredProperties.push_back ( theManager.getSpeciesName ( i ) + "AdsorpedExpelled" );
      m_registeredProperties.push_back ( theManager.getSpeciesName ( i ) + "AdsorpedFree" );
   }

   // Should these properties be part of the genex-result manager?
   // They will always be output when the shale-gas simulator is run.
   m_registeredProperties.push_back ( "HcSaturation" );
   m_registeredProperties.push_back ( "ImmobileWaterSat" );
   m_registeredProperties.push_back ( "FractionOfAdsorptionCap" );
   m_registeredProperties.push_back ( "HcVapourSat" );
   m_registeredProperties.push_back ( "HcLiquidSat" );
   m_registeredProperties.push_back ( "AdsorptionCapacity" );
   m_registeredProperties.push_back ( "RetainedGasVolumeST" );
   m_registeredProperties.push_back ( "RetainedOilVolumeST" );
   m_registeredProperties.push_back ( "GasExpansionRatio_Bg" );
   m_registeredProperties.push_back ( "Oil2GasGeneratedCumulative" );
   m_registeredProperties.push_back ( "TotalGasGeneratedCumulative" );
   m_registeredProperties.push_back ( "RetainedOilApiSR" );
   m_registeredProperties.push_back ( "RetainedCondensateApiSR" );
   m_registeredProperties.push_back ( "RetainedGorSR" );
   m_registeredProperties.push_back ( "RetainedCgrSR" );
   m_registeredProperties.push_back ( "OverChargeFactor" );
   m_registeredProperties.push_back ( "PorosityLossFromPyroBitumen" );
   m_registeredProperties.push_back ( "H2SRisk" );
   m_registeredProperties.push_back ( "OrganoPorosity" );

   m_registeredProperties.push_back ( "SourceRockEndMember1" );
   m_registeredProperties.push_back ( "SourceRockEndMember2" );

   m_registeredProperties.push_back ( "TOC" );

   sort(m_registeredProperties.begin(), m_registeredProperties.end());
}

bool GenexSimulator::isPropertyRegistered(const string & propertyName)
{
   return std::binary_search(m_registeredProperties.begin(), m_registeredProperties.end(), propertyName);
}

void GenexSimulator::deleteSourceRockPropertyValues()
{
   Interface::MutablePropertyValueList::iterator propertyValueIter;

   propertyValueIter = m_propertyValues.begin ();

   while (propertyValueIter != m_propertyValues.end ()) {
      Interface::PropertyValue * propertyValue = * propertyValueIter;

      if(isPropertyRegistered (propertyValue->getProperty()->getName())) {

         propertyValueIter = m_propertyValues.erase (propertyValueIter);

         if(propertyValue->getRecord ()) {
            propertyValue->getRecord ()->getTable ()->deleteRecord (propertyValue->getRecord ());
         }

         delete propertyValue;
      } else {
         ++ propertyValueIter;
      }

   }

}
bool GenexSimulator::mergeOutputFiles ( ) {

   if( getModellingMode () == Interface::MODE1D ) return true;
#ifdef _MSC_VER
   return true;
#else
   ibs::FilePath localPath  ( getProjectPath () );
   localPath <<  getOutputDir ();
   const bool status = H5_Parallel_PropertyList ::mergeOutputFiles ( GenexActivityName, localPath.path() );

   return status;
#endif
}
