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

// File system library
#include "FilePath.h"

// utilitites
#include "LogHandler.h"

//------------------------------------------------------------//

CrustalThicknessCalculator* CrustalThicknessCalculator::m_crustalThicknessCalculator = 0;
string CrustalThicknessCalculator::m_projectFileName = "";
string CrustalThicknessCalculator::m_outputFileName = "";
const string CrustalThicknessCalculatorActivityName = "CrustalThicknessCalculator";

//------------------------------------------------------------//

CrustalThicknessCalculator::CrustalThicknessCalculator( database::Database * database, const std::string & name, const std::string & accessMode, ObjectFactory* objectFactory )
   : DataAccess::Mining::ProjectHandle( database, name, accessMode, objectFactory ),
     m_outputOptions                      (0      ),
     m_debug                              (false  ),
     m_applySmoothing                     (true   ),
     m_inputData                          (nullptr),
     m_previousTTS                        (nullptr),
     m_previousContinentalCrustalThickness(nullptr),
     m_previousOceanicCrustalThickness    (nullptr)
{}
 
//------------------------------------------------------------//

CrustalThicknessCalculator::~CrustalThicknessCalculator () {
   // No need to delete m_inputData as it is a shared pointer
   // No need to delete maps as they will be automatically deleted by the DataAccess library
}

//------------------------------------------------------------//

CrustalThicknessCalculator* CrustalThicknessCalculator::createFrom( const string& inputFileName, ObjectFactory* factory ) {


   if ( m_crustalThicknessCalculator == 0 ) {
      m_crustalThicknessCalculator = (CrustalThicknessCalculator*)Interface::OpenCauldronProject( inputFileName, "rw", factory );

   }
   m_projectFileName = inputFileName;

   return m_crustalThicknessCalculator;
}

//------------------------------------------------------------//

void CrustalThicknessCalculator::initialiseCTC() {

   ///1. Initialise CTC instance
   LogHandler( LogHandler::INFO_SEVERITY, LogHandler::SECTION ) << "Starting CTC activity";

   bool started = CrustalThicknessCalculator::getInstance().startActivity( CrustalThicknessCalculatorActivityName,
      CrustalThicknessCalculator::getInstance().getHighResolutionOutputGrid(),
      true );
   if (!started) {
      throw CtcException() << "Can not start CrustalThicknessCalculator activity.";
   }

   ///2. Initialise GeoPhysics ProjectHandle
   LogHandler( LogHandler::INFO_SEVERITY, LogHandler::SECTION ) << "Reading project file";
   started = GeoPhysics::ProjectHandle::initialise();
   if (!started) {
      throw CtcException() << "Can not start CrustalThicknessCalculator because geophysics project handle cannot be initialised.";
   }

   ///3. Initialise InterfaceInput
   LogHandler( LogHandler::INFO_SEVERITY, LogHandler::SECTION ) << "Setting CTC input data";
   setFormationLithologies( false, true );

   m_inputData = std::shared_ptr<InterfaceInput>(new InterfaceInput( m_tableCTC.data(), m_tableCTCRiftingHistory.data() ));
   m_inputData->loadSnapshots();
   m_inputData->loadSurfaceDepthHistoryMask( this );
   // loads configuration file, CTCIoTbl and CTCRiftingHistoryIoTbl
   m_inputData->loadInputData();

   ///4. Load smoothing radius
   m_applySmoothing = (m_inputData->getSmoothRadius() > 0);
   setAdditionalOptionsFromCommandLine();

   ///5. Initialise InterfaceOutput
   setRequestedOutputProperties( m_outputData );
   if (m_debug) {
      m_outputData.setAllMapsToOutput( true );
   }

}

//------------------------------------------------------------//

void CrustalThicknessCalculator::finalise ( const bool saveResults ) {

   m_crustalThicknessCalculator->setSimulationDetails ( "fastctc", "Default", "" );
   m_crustalThicknessCalculator->finishActivity ();

   if( saveResults ) {
      if( ! CrustalThicknessCalculator::getInstance ().mergeOutputFiles ()) {
         LogHandler(LogHandler::ERROR_SEVERITY) << "Unable to merge output files";
      }
      if (m_crustalThicknessCalculator->getRank() == 0) {
         if ( m_outputFileName.empty() ) {
            m_crustalThicknessCalculator->saveToFile( m_outputFileName );
         }
         else {
            m_crustalThicknessCalculator->saveToFile( m_projectFileName );
         }
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
      thicknessBasaltMap, thicknessCrustMap, thicknessCrustMeltOnset, WLSadjustedMap, RDAadjustedMap, TFMap,
      cumSedimentBackstrip, WLSMap, incTectonicSubsidenceAdjusted, isostaticBathymetry, PaleowaterdepthResidual,
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
   std::shared_ptr<GridMap> presentDayTTS;
   Validator validator( *this );
   std::vector< double > snapshotForLoop = m_inputData->copySnapshots();
   // sort from start to end [250 220 ... 0]
   // we insert a 0 snapshot at the beginning because we need first to compute TTS at 0Ma
   // then we compute everything else in the reverse order [0 250 220 ... 0]
   std::sort( snapshotForLoop.begin(), snapshotForLoop.end(), std::greater<double>() );
   snapshotForLoop.insert( snapshotForLoop.begin(), 0.0 );

   for (unsigned int k = 0; k < snapshotForLoop.size(); ++k) {

      const double age = snapshotForLoop[k];
      if (k == 0 and age != 0.0){
         throw CtcException() << "Cannot compute initial total tectonic subsidence";
      }

      if ( (age >= m_inputData->getFlexuralAge() ) or k == 0){

         if (k == 0){
            LogHandler( LogHandler::INFO_SEVERITY, LogHandler::SECTION ) << "Precomputing present day Total Tectonic Subsidence";
         }
         else{
            if (k == 1){
               LogHandler( LogHandler::INFO_SEVERITY, LogHandler::SECTION ) << "Computing CTC output maps";
            }
            LogHandler( LogHandler::INFO_SEVERITY, LogHandler::SUBSECTION ) << "Snapshot: " << age << "Ma";
         }
         updateValidNodes( age );
         const Snapshot * theSnapshot = findSnapshot( age );

         /// 1. Load P/T data fot this snapshot
         m_inputData->loadTopAndBottomOfSediments( m_crustalThicknessCalculator, age, m_inputData->getBaseRiftSurfaceName() );
         const DataModel::AbstractProperty* depthProperty = m_inputData->loadDepthProperty();
         m_inputData->loadDepthData( m_crustalThicknessCalculator, depthProperty, age );
         const DataModel::AbstractProperty* pressureProperty = m_inputData->loadPressureProperty();
         m_inputData->loadPressureData( m_crustalThicknessCalculator, pressureProperty, age );

         /// 2. Create the maps for this snapshot
         m_outputData.createSnapShotOutputMaps( m_crustalThicknessCalculator, m_inputData, theSnapshot, m_inputData->getTopOfSedimentSurface(), m_debug );

         retrieveData();

         /// 3. Compute the backstripped density and thickness, the backtrip and the compensation
         LogHandler( LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_STEP ) << "computing Backstrip";
         CrustalThickness::DensityCalculator densityCalculator( *m_inputData, m_outputData, validator );
         densityCalculator.compute();
         if (not m_debug) m_outputData.disableDebugOutput( m_crustalThicknessCalculator, m_inputData->getBotOfSedimentSurface(), theSnapshot );

         /// 4. Compute the Total Tectonic Subsidence (only if we have a SDH at this snapshot)
         const Interface::Property* pressureInterfaceProperty = findProperty( "Pressure" );
         if (hasSurfaceDepthHistory( age )){
            LogHandler( LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_STEP ) << "computing Tectonic Subsidence";
            CrustalThickness::TotalTectonicSubsidenceCalculator TTScalculator( *m_inputData, m_outputData, validator,
               age, densityCalculator.getAirCorrection(), m_previousTTS, m_seaBottomDepth );
            TTScalculator.compute();
            // This is just for the first step when we compute the TTS at 0Ma, then we go in the reverse order
            if (k == 0){
               assert( age == 0.0 );
               presentDayTTS          = std::shared_ptr<GridMap>( this->getFactory()->produceGridMap( nullptr, 0, m_outputData.getMap( WLSMap ), identity ) );
               // delete the record so it will not be save in TimeIoTbl
               m_recordLessMapPropertyValues.clear();
               restoreData();
               continue;
            }
            else{
               m_previousTTS = m_outputData.getMap( WLSMap );
            }
         }

         /// 5. Compute the Paleowaterdepth
         LogHandler( LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_STEP ) << "computing Paleowaterdepth";
         CrustalThickness::PaleowaterdepthCalculator PWDcalculator( *m_inputData, m_outputData, validator, presentDayTTS.get() );
         PWDcalculator.compute();

         // 6. Compute the PaleowaterdepthResidual (only if we have a SDH at this snapshot and if we are not at present day)
         if (hasSurfaceDepthHistory( age ) and age!=0.0){
            LogHandler( LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_STEP ) << "computing PaleowaterdepthResidual";
            CrustalThickness::PaleowaterdepthResidualCalculator PWDRcalculator( *m_inputData, m_outputData, validator,
               age, m_seaBottomDepth );
            PWDRcalculator.compute();
         }

         ///7. Computes the thinning factor and crustal thicknesse
         if (m_inputData->getRiftingCalculationMask( age )){
            LogHandler( LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_STEP ) << "computing Crustal Thicknesses";
            CrustalThickness::McKenzieCrustCalculator mcKenzieCalculator( *m_inputData, m_outputData, validator, age, m_previousContinentalCrustalThickness, m_previousOceanicCrustalThickness );
            mcKenzieCalculator.compute();
            m_previousContinentalCrustalThickness = m_outputData.getMap( thicknessCrustMap  );
            m_previousOceanicCrustalThickness     = m_outputData.getMap( thicknessBasaltMap );
         }

         ///8. Smooth the PWD and Crustal Thicknesses maps
         smoothOutputs();

         restoreData();
         m_outputData.saveOutput( m_crustalThicknessCalculator, m_debug, m_outputOptions, theSnapshot );

         // Save properties to disk.
         m_crustalThicknessCalculator->continueActivity();
      }
   }

}

//------------------------------------------------------------//

void CrustalThicknessCalculator::updateValidNodes( const double age ) {
   LogHandler( LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_STEP ) << "setting area of interest";
   initialiseValidNodes(false);
   addUndefinedAreas( &(m_inputData->getHCuMap    ()    ) );
   addUndefinedAreas( &(m_inputData->getHLMuMap   ()    ) );
   addUndefinedAreas( &(m_inputData->getHBuMap    (age) ) );
   addUndefinedAreas( &(m_inputData->getDeltaSLMap(age) ) );
}

//------------------------------------------------------------//
bool CrustalThicknessCalculator::mergeOutputFiles() {

#ifdef _MSC_VER
   return true;
#else
   if (getModellingMode() == Interface::MODE1D) return true;

   ibs::FilePath localPath( getProjectPath() );
   localPath << getOutputDir();

   const bool status = H5_Parallel_PropertyList::mergeOutputFiles( CrustalThicknessCalculatorActivityName, localPath.path() );

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
   //Smooth the PWD and crustal thicknesses maps if requested
   if (m_applySmoothing) {
      std::vector<outputMaps> mapsToSmooth = { isostaticBathymetry, thicknessBasaltMap, thicknessCrustMap };
      MapSmoother mapSmoother( m_inputData->getSmoothRadius() );
      LogHandler( LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_STEP ) << "applying spatial smoothing with radius set to " << m_inputData->getSmoothRadius() << " for maps:";

      for (size_t i = 0; i < mapsToSmooth.size(); i++){
         if (m_outputData.getOutputMask( mapsToSmooth[i] )){
            LogHandler( LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP ) << outputMapsNames[mapsToSmooth[i]];
            bool status = mapSmoother.averageSmoothing( m_outputData.getMap( mapsToSmooth[i] ) );
            if (!status) {
               throw CtcException() << "Failed to smooth " << outputMapsNames[mapsToSmooth[i]];
            }
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