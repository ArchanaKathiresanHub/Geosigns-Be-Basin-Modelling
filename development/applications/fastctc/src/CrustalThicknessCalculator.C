//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "CrustalThicknessCalculator.h"


// petsc library
#include <petsc.h>

// TableIO library
#include "database.h"
#include "cauldronschema.h"
#include "cauldronschemafuncs.h"
using namespace database;

// DataAccess library
#include "Interface/CrustalThicknessData.h"
#include "Interface/DistributedGridMap.h"
#include "Interface/Formation.h"
#include "Interface/Interface.h"
#include "Interface/PaleoProperty.h"
#include "Interface/Property.h"
#include "Interface/PropertyValue.h"
#include "Interface/Snapshot.h"
#include "Interface/Surface.h"

// Crustal Thickness library
#include "DensityCalculator.h"
#include "LinearFunction.h"
#include "MapSmoother.h"
#include "McKenzieCrustCalculator.h"
#include "TotalTectonicSubsidenceCalculator.h"
#include "PaleowaterdepthCalculator.h"
#include "PaleowaterdepthResidualCalculator.h"
#include "Validator.h"

// Parallel _Hdf5 library
#include "h5_parallel_file_types.h"
#include "h5merge.h"

// utilitites
#include "LogHandler.h"

#include <typeinfo>       // operator typeid

//------------------------------------------------------------//

CrustalThicknessCalculator* CrustalThicknessCalculator::m_crustalThicknessCalculator = 0;
string CrustalThicknessCalculator::m_projectFileName = "";
string CrustalThicknessCalculator::m_outputFileName = "";
const string CrustalThicknessCalculatorActivityName = "CrustalThicknessCalculator";

//------------------------------------------------------------//
void displayTime( const double timeToDisplay, const char * msgToDisplay ) {

   int hours   = (int)(timeToDisplay / 3600.0);
   int minutes = (int)((timeToDisplay - (hours * 3600.0)) / 60.0);
   int seconds = (int)(timeToDisplay - hours * 3600.0 - minutes * 60.0);

   PetscPrintf( PETSC_COMM_WORLD, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ \n" );
   PetscPrintf( PETSC_COMM_WORLD, "%s: %d hours %d minutes %d seconds\n", msgToDisplay, hours, minutes, seconds );

}

CrustalThicknessCalculator::CrustalThicknessCalculator( database::Database * database, const std::string & name, const std::string & accessMode, ObjectFactory* objectFactory )
   : DataAccess::Mining::ProjectHandle( database, name, accessMode, objectFactory ) {

   m_outputOptions  = 0;
   m_debug          = false;
   m_applySmoothing = true;
   m_previousTTS    = nullptr;

}
 
//------------------------------------------------------------//

CrustalThicknessCalculator::~CrustalThicknessCalculator () {
}

//------------------------------------------------------------//

CrustalThicknessCalculator* CrustalThicknessCalculator::CreateFrom( const string& inputFileName, ObjectFactory* factory ) {


   if ( m_crustalThicknessCalculator == 0 ) {
      m_crustalThicknessCalculator = (CrustalThicknessCalculator*)Interface::OpenCauldronProject( inputFileName, "rw", factory );

   }
   m_projectFileName = inputFileName;

   return m_crustalThicknessCalculator;
}

//------------------------------------------------------------//

void CrustalThicknessCalculator::loadSnapshots( ) {

   LogHandler( LogHandler::DEBUG_SEVERITY ) << "Loading snpashots from stratigraphy:";
   Interface::FormationList* formations = getFormations();
   Interface::FormationList::const_iterator formationIter;

   m_snapshots.push_back( 0.0 ); // add present day
   LogHandler( LogHandler::DEBUG_SEVERITY ) << "   #time 0.0Ma loaded";

   for (formationIter = formations->begin(); formationIter != formations->end(); ++formationIter) {
      const Interface::Formation * formation = (*formationIter);

      if (formation != 0) {
         const Interface::Surface * topSurface = formation->getBottomSurface();
         m_snapshots.push_back( topSurface->getSnapshot()->getTime() );
         LogHandler( LogHandler::DEBUG_SEVERITY ) << "   #time " << topSurface->getSnapshot()->getTime() << "Ma loaded";
      }
   }
   delete formations;
}

//------------------------------------------------------------//

void CrustalThicknessCalculator::initialise() {

   ///1. Initialise CTC instance
   bool started = CrustalThicknessCalculator::getInstance().startActivity( CrustalThicknessCalculatorActivityName,
      CrustalThicknessCalculator::getInstance().getHighResolutionOutputGrid(),
      true );
   if (!started) {
      throw CtcException() << "Can not start CrustalThicknessCalculator activity.";
   }

   ///2. Initialise GeoPhysics ProjectHandle
   started = GeoPhysics::ProjectHandle::initialise();
   if (!started) {
      throw CtcException() << "Can not start CrustalThicknessCalculator because geophysics project handle cannot be initialised.";
   }

   ///3. Initialise InterfaceInput
   setFormationLithologies( false, true );
   if (m_crustalThicknessData.size() != 1) {
      if (m_crustalThicknessData.size() == 0) {
         throw CtcException() << "The CrustalThicknessData table in the ProjectFile is empty.";
      }
      else {
         throw CtcException() << "Too many records for CrustalThicknessData table in the ProjectFile.";
      }
   }

   m_inputData = dynamic_cast<InterfaceInput *>(m_crustalThicknessData[0]);
   if (!m_inputData)
   {
      throw CtcException() << "Could not create the input interface from crustal thickness data.";
   }
   m_inputData->loadInputDataAndConfigurationFile( "InterfaceData.cfg" );

   updateValidNodes( m_inputData );

   ///4. Load smoothing radius
   m_applySmoothing = (m_inputData->getSmoothRadius() > 0);
   setAdditionalOptionsFromCommandLine();

   ///5. Initialise InterfaceOutput
   setRequestedOutputProperties( m_outputData );
   if (m_debug) {
      m_outputData.setAllMapsToOutput( true );
   }

   ///6. Load snapshots from stratigraphy
   loadSnapshots();
   std::sort( m_snapshots.begin(), m_snapshots.end(), std::greater<int>() );
   // we insert a 0 snapshot at the beginning because we need first to compute TTS at 0Ma
   // then we compute everything else in the reverse order
   m_snapshots.insert( m_snapshots.begin(), 0.0 );
}

//------------------------------------------------------------//

void CrustalThicknessCalculator::finalise ( const bool saveResults ) {

   m_crustalThicknessCalculator->setSimulationDetails ( "fastctc", "Default", "" );
   m_crustalThicknessCalculator->finishActivity ();

   if( saveResults ) {
      if( ! CrustalThicknessCalculator::getInstance ().mergeOutputFiles ()) {
         PetscPrintf ( PETSC_COMM_WORLD, "  MeSsAgE ERROR Unable to merge output files\n");
      }
   }
   if ( saveResults && m_crustalThicknessCalculator->getRank() == 0 ) {
      if( m_outputFileName.length() == 0 ) {
         m_crustalThicknessCalculator->saveToFile(m_projectFileName);
      } else {
         m_crustalThicknessCalculator->saveToFile(m_outputFileName);
      }
   }
   delete m_crustalThicknessCalculator;
   m_crustalThicknessCalculator = 0;
}

//------------------------------------------------------------//

void CrustalThicknessCalculator::setRequestedOutputProperties( InterfaceOutput & theOutput )
{
   Interface::ModellingMode theMode = getModellingMode();
   string theModellingMode = "3d";

   if( Interface::MODE1D == theMode ) {
      theModellingMode = "1d";
   }
   
   Table * timeIoTbl = getTable ("FilterTimeIoTbl");
   Table::iterator tblIter;

   for (tblIter = timeIoTbl->begin (); tblIter != timeIoTbl->end (); ++ tblIter) {
      Record * filterTimeIoRecord = * tblIter;
      const string & outPutOption = database::getOutputOption(filterTimeIoRecord);
      const string & modellingMode = database::getModellingMode(filterTimeIoRecord);
      const string & propertyName = database::getPropertyName (filterTimeIoRecord);

      if(outPutOption != "None" && modellingMode == theModellingMode) {

         outputMaps index = CrustalThicknessInterface::getPropertyId ( propertyName );
         if( index != numberOfOutputMaps ) {
            theOutput.setMapToOutput( index );
         }

      }
   }

   // set default output properties
   theOutput.setMapsToOutput( mohoMap, topBasaltMap,
      thicknessBasaltMap, thicknessCrustMap, thicknessCrustMeltOnset, WLSadjustedMap, RDAadjustedMap,
      cumSedimentBackstrip, WLSMap, isostaticBathymetry, PaleowaterdepthResidual,
      numberOfOutputMaps );
}

//------------------------------------------------------------//

void CrustalThicknessCalculator::deleteCTCPropertyValues()
{
   Interface::MutablePropertyValueList::iterator propertyValueIter;

   propertyValueIter = m_propertyValues.begin ();

   while (propertyValueIter != m_propertyValues.end ()) {
      Interface::PropertyValue * propertyValue = * propertyValueIter;

      if(CrustalThicknessInterface::getPropertyId (propertyValue->getProperty()->getName()) != numberOfOutputMaps ) {

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

//------------------------------------------------------------//

void CrustalThicknessCalculator::run() {

   Interface::IdentityFunctor identity;
   GridMap* currentPressureTTS = nullptr;
   std::shared_ptr<GridMap> prensentDayPressureTTS;
   std::shared_ptr<GridMap> presentDayTTS;
   Validator validator( *this );

   for (unsigned int k = 0; k < m_snapshots.size(); ++k) {

      const double age = m_snapshots[k];
      if (age >= m_inputData->getFlexuralAge() or k == 0){
         const Snapshot * theSnapshot = (const Snapshot *)findSnapshot( age );

         /// 1. Load P/T data fot this snapshot
         m_inputData->loadTopAndBottomOfSediments( m_crustalThicknessCalculator, age, m_inputData->getBaseRiftSurfaceName() );
         const DataModel::AbstractProperty* depthProperty = m_inputData->loadDepthProperty();
         m_inputData->loadDepthData( m_crustalThicknessCalculator, depthProperty, age );
         const DataModel::AbstractProperty* pressureProperty = m_inputData->loadPressureProperty();
         m_inputData->loadPressureData( m_crustalThicknessCalculator, pressureProperty, age );

         /// 2. Create the maps for this snapshot
         if (!m_outputData.createSnapShotOutputMaps( m_crustalThicknessCalculator, theSnapshot, m_inputData->getTopOfSedimentSurface() )) {
            throw CtcException() << "Cannot allocate output maps.";
         }

         retrieveData();

         /// 3. Compute the backstripped density and thickness, the backtrip and the compensation
         DensityCalculator densityCalculator( *m_inputData, m_outputData, validator );
         densityCalculator.compute();
         if (!m_debug) m_outputData.disableBackstripOutput( m_crustalThicknessCalculator, m_inputData->getBotOfSedimentSurface(), theSnapshot );

         /// 4. Compute the Total Tectonic Subsidence (only if we have a SDH at this snapshot)
         const Interface::Property* pressureInterfaceProperty = findProperty( "Pressure" );
         if (asSurfaceDepthHistory( age )){
            TotalTectonicSubsidenceCalculator TTScalculator( *m_inputData, m_outputData, validator,
               age, densityCalculator.getAirCorrection(), m_previousTTS, m_seaBottomDepth );
            TTScalculator.compute();
            if (k > 0) m_previousTTS = m_outputData.getMap( WLSMap );
            currentPressureTTS = m_inputData->loadPropertyDataFromDepthMap( this, m_outputData.getMap( WLSMap ), pressureInterfaceProperty, theSnapshot );
            // This is just for the first step when we compute the TTS at 0Ma, then we go in the reverse order
            if (age == 0.0 and k == 0){
               presentDayTTS          = std::shared_ptr<GridMap>( this->getFactory()->produceGridMap( nullptr, 0, m_outputData.getMap( WLSMap ), identity ) );
               prensentDayPressureTTS = std::shared_ptr<GridMap>( this->getFactory()->produceGridMap( nullptr, 0, currentPressureTTS,            identity ) );
               // delete the record so it will not be save in TimeIoTbl
               m_recordLessMapPropertyValues.clear();
               restoreData();
               continue;
            }
         }
         else{
            currentPressureTTS = nullptr;
         }

         /// 5. Compute the Paleowaterdepth
         PaleowaterdepthCalculator PWDcalculator( *m_inputData, m_outputData, validator,
            presentDayTTS.get(), prensentDayPressureTTS.get(), currentPressureTTS );
         PWDcalculator.compute();

         // 6. Compute the PaleowaterdepthResidual (only if we have a SDH at this snapshot and if we are not at present day)
         if (asSurfaceDepthHistory( age ) and age!=0.0){
            PaleowaterdepthResidualCalculator PWDRcalculator( *m_inputData, m_outputData, validator,
               age, m_seaBottomDepth );
            PWDRcalculator.compute();
         }

         ///7. Smooth the TTS and PWD map
         if (m_applySmoothing) smoothOutputs();

         ///8. Computes the thinning factor and crusltal thicknesse
         McKenzieCrustCalculator mcKenzieCalculator( *m_inputData, m_outputData, validator );
         mcKenzieCalculator.compute();

         restoreData();
         m_outputData.saveOutput( m_crustalThicknessCalculator, m_debug, m_outputOptions, theSnapshot );

         // Save properties to disk.
         m_crustalThicknessCalculator->continueActivity();
      }
   }

}

//------------------------------------------------------------//

void CrustalThicknessCalculator::updateValidNodes( const InterfaceInput* theInterfaceData ) {

   addUndefinedAreas( &theInterfaceData->getT0Map     () );
   addUndefinedAreas( &theInterfaceData->getTRMap     () );
   addUndefinedAreas( &theInterfaceData->getHCuMap    () );
   addUndefinedAreas( &theInterfaceData->getHLMuMap   () );
   addUndefinedAreas( &theInterfaceData->getDeltaSLMap() );
}

//------------------------------------------------------------//

bool CrustalThicknessCalculator::mergeOutputFiles ( ) {

#ifdef _MSC_VER
	return true;
#else
	if( ! H5_Parallel_PropertyList::isOneFilePerProcessEnabled() ) return true;

   PetscBool noFileCopy = PETSC_FALSE;

   PetscOptionsHasName( PETSC_NULL, "-nocopy", &noFileCopy );

   PetscLogDouble merge_Start_Time;
   PetscTime( &merge_Start_Time );

   string fileName = CrustalThicknessCalculatorActivityName + "_Results.HDF";
   string filePathName = getProjectPath() + "/" + getOutputDir() + "/" + fileName;

   bool status = mergeFiles( allocateFileHandler( PETSC_COMM_WORLD, filePathName, H5_Parallel_PropertyList::getTempDirName(), (noFileCopy ? CREATE : REUSE) ) );

   if (status) {
      status = H5_Parallel_PropertyList::copyMergedFile( filePathName );
   }
   if (status) {
      PetscLogDouble merge_End_Time;
      PetscTime( &merge_End_Time );

      displayTime( merge_End_Time - merge_Start_Time, "Merging of output files" );
   }
   else {
      PetscPrintf( PETSC_COMM_WORLD, "  MeSsAgE ERROR Could not copy the file %s.\n", filePathName.c_str() );
   }
   return status;
#endif
}

//------------------------------------------------------------//

bool CrustalThicknessCalculator::parseCommandLine() {

   PetscBool isDefined = PETSC_FALSE;
   
   H5_Parallel_PropertyList::setOneFilePerProcessOption();
   PetscOptionsHasName (PETSC_NULL, "-xyz", &isDefined);
   if (isDefined) {
      m_outputOptions |= XYZ;
   }

   PetscOptionsHasName (PETSC_NULL, "-sur", &isDefined);
   if (isDefined) {
      m_outputOptions |= SUR;
   }

   PetscOptionsHasName (PETSC_NULL, "-debug", &isDefined);
   if (isDefined) {
      m_debug = true;
   }

   PetscOptionsHasName (PETSC_NULL, "-hdf", &isDefined);
   if (isDefined) {
      m_outputOptions |= HDF;
   }
   char outputFileName[128];
   outputFileName[0] = '\0';

   PetscOptionsGetString (PETSC_NULL, "-save", outputFileName, 128, &isDefined);
   if(isDefined) {
      m_outputFileName = outputFileName;
   }
   return true;
}

//------------------------------------------------------------//

void CrustalThicknessCalculator::setAdditionalOptionsFromCommandLine() {

   PetscBool isDefined = PETSC_FALSE;
   int radius;

   PetscOptionsHasName (PETSC_NULL, "-nosmooth", &isDefined);
   if (isDefined) {
      m_applySmoothing = false;
      m_inputData->setSmoothingRadius( 0 );
   }

   PetscOptionsGetInt ( PETSC_NULL, "-smooth", &radius, &isDefined );
   if (isDefined) {
      m_applySmoothing = ( radius > 0 );
      m_inputData->setSmoothingRadius( static_cast<unsigned int>(radius) );
   }

}

//------------------------------------------------------------//

void CrustalThicknessCalculator::smoothOutputs() {
   //Smooth the TTS and PWD map if requested
   if (m_applySmoothing) {
      std::vector<outputMaps> mapsToSmooth = { WLSMap, isostaticBathymetry };
      MapSmoother mapSmoother( m_inputData->getSmoothRadius() );
      LogHandler( LogHandler::INFO_SEVERITY ) << "Applying spatial smoothing with radius set to " << m_inputData->getSmoothRadius() << " for maps:";

      for (size_t i = 0; i < mapsToSmooth.size(); i++){
         LogHandler( LogHandler::INFO_SEVERITY ) << "   #" << outputMapsNames[ mapsToSmooth[i] ];
         bool status = mapSmoother.averageSmoothing( m_outputData.getMap( mapsToSmooth[i] ) );
         if (!status) {
            throw CtcException() << "Failed to smooth " << outputMapsNames[ mapsToSmooth[i] ] << ".";
         }
          
      }
   }
}

//------------------------------------------------------------//

void CrustalThicknessCalculator::retrieveData(){
   m_outputData.retrieveData();
   if (m_previousTTS != nullptr) {
      m_previousTTS->retrieveData();
   }
}

//------------------------------------------------------------//

void CrustalThicknessCalculator::restoreData(){
   m_outputData.restoreData();
   if (m_previousTTS != nullptr) {
      m_previousTTS->restoreData();
   }
}