//
// Copyright (C) 2015-2018 Shell International Exploration & Production.
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
#include "cauldronschemafuncs.h"
using namespace database;

// DataAccess library
#include "CrustalThicknessData.h"
#include "Interface.h"
#include "Property.h"
#include "PropertyValue.h"
#include "Snapshot.h"

using namespace DataAccess::Interface;

// Crustal Thickness library
#include "DensityCalculator.h"
#include "CTCMapSmoother.h"
#include "McKenzieCrustCalculator.h"
#include "TotalTectonicSubsidenceCalculator.h"
#include "PaleowaterdepthCalculator.h"
#include "PaleowaterdepthResidualCalculator.h"

// Parallel _Hdf5 library
#include "h5_parallel_file_types.h"

// File system library
#include "FilePath.h"

// utilitites
#include "LogHandler.h"

#include <iomanip>
#include <sstream>
//------------------------------------------------------------//

string CrustalThicknessCalculator::m_projectFileName;
string CrustalThicknessCalculator::m_outputFileName;
const string CrustalThicknessCalculatorActivityName = "CrustalThicknessCalculator";

//------------------------------------------------------------//

CrustalThicknessCalculator::CrustalThicknessCalculator(const database::ProjectFileHandlerPtr database, const std::string & name, const ObjectFactory* objectFactory )
   : DataAccess::Mining::ProjectHandle( database, name, objectFactory ),
     m_outputOptions                      (0      ),
     m_debug                              (false  ),
     m_applySmoothing                     (true   ),
     m_inputData                          (nullptr),
     m_previousTTS                        (nullptr),
     m_previousRiftTTS                    (nullptr),
     m_previousContinentalCrustalThickness(nullptr),
     m_previousOceanicCrustalThickness    (nullptr),
     m_mergeHDF                           (false  )
{}

//------------------------------------------------------------//

CrustalThicknessCalculator* CrustalThicknessCalculator::createFrom( const string& inputFileName, ObjectFactory* factory ) {

   m_projectFileName = inputFileName;

   return dynamic_cast<CrustalThicknessCalculator*>(Interface::OpenCauldronProject(inputFileName, factory) );
}

//------------------------------------------------------------//

void CrustalThicknessCalculator::initialiseCTC() {

   ///1. Initialise CTC instance
   LogHandler( LogHandler::INFO_SEVERITY, LogHandler::SECTION ) << "Starting CTC activity";

   bool started = startActivity( CrustalThicknessCalculatorActivityName,
      getHighResolutionOutputGrid(),
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
   m_inputData->loadInputData( );

   //clean old crustal histories, if defined at non calculation ages
   LogHandler(LogHandler::DEBUG_SEVERITY, LogHandler::COMPUTATION_STEP) << "Clear input data from crustal History Tables, if defined at non-calculation ages of CTC";
   m_inputData->cleanOldCrustalHistoriesAtNonCalculationAges(this, "ContCrustalThicknessIoTbl");
   m_inputData->cleanOldCrustalHistoriesAtNonCalculationAges(this, "OceaCrustalThicknessIoTbl");
   m_inputData->cleanAllInputPWDMapsExceptPresentDay(this, "SurfaceDepthIoTbl");

   ///4. Load smoothing radius
   m_applySmoothing = (m_inputData->getSmoothRadius() > 0);
   setAdditionalOptionsFromCommandLine();

   ///5. Initialize InterfaceOutput
   setRequestedOutputProperties( m_outputData );
   if (m_debug) {
      m_outputData.setAllMapsToOutput( true );
   }

}

//------------------------------------------------------------//

void CrustalThicknessCalculator::finalise ( const bool saveResults ) {

   setSimulationDetails ( "fastctc", "Default", "" );
   finishActivity ();

   if( saveResults ) {
      if( ! mergeOutputFiles ()) {
         LogHandler(LogHandler::ERROR_SEVERITY) << "Unable to merge output files";
      }
      if (getRank() == 0) {
         if ( m_outputFileName.empty() ) {
            saveToFile( m_projectFileName );
         }
         else {
            saveToFile( m_outputFileName );
         }
      }
   }
}

//------------------------------------------------------------//

void CrustalThicknessCalculator::setRequestedOutputProperties( InterfaceOutput & theOutput ) const
{
   const Interface::ModellingMode theMode = getModellingMode();
   string theModellingMode = "3d";

   if( Interface::MODE1D == theMode ) {
      theModellingMode = "1d";
   }

   Table * timeIoTbl = getTable ("FilterTimeIoTbl");

   for (auto tblIter = timeIoTbl->begin (); tblIter != timeIoTbl->end (); ++ tblIter) {
      Record * filterTimeIoRecord = * tblIter;
      const string & outPutOption = database::getOutputOption(filterTimeIoRecord);
      const string & modellingMode = database::getModellingMode(filterTimeIoRecord);
      const string & propertyName = database::getPropertyName (filterTimeIoRecord);

      if(outPutOption != "None" && modellingMode == theModellingMode) {

         const outputMaps index = CrustalThicknessInterface::getPropertyId ( propertyName );
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

bool CrustalThicknessCalculator::deleteCTCPropertyValues()
{
   Interface::MutablePropertyValueList::iterator propertyValueIter = m_propertyValues.begin ();

   while (propertyValueIter != m_propertyValues.end ()) {
      Interface::PropertyValue * propertyValue = * propertyValueIter;

      if( getPropertyId (propertyValue->getProperty()->getName()) != numberOfOutputMaps ) {

         propertyValueIter = m_propertyValues.erase (propertyValueIter);

         if(propertyValue->getRecord ()) {
            propertyValue->getRecord ()->getTable ()->deleteRecord (propertyValue->getRecord ());
         }

         delete propertyValue;
         propertyValue = nullptr;
      } else {
         ++ propertyValueIter;
      }

   }
   return m_propertyValues.empty();
}

//------------------------------------------------------------//

void CrustalThicknessCalculator::run() {

    CleanGridMapIoTbl();

   Interface::IdentityFunctor identity;
   std::shared_ptr<GridMap> presentDayTTS;
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

         /// 1. Load P/T data for this snapshot
         m_inputData->loadTopAndBottomOfSediments( this, age, m_inputData->getBaseRiftSurfaceName() );
         const DataModel::AbstractProperty* depthProperty = m_inputData->loadDepthProperty();
         m_inputData->loadDepthData( this, depthProperty, age );
         const DataModel::AbstractProperty* pressureProperty = m_inputData->loadPressureProperty();
         m_inputData->loadPressureData( this, pressureProperty, age );

         /// 2. Create the maps for this snapshot
         m_outputData.createSnapShotOutputMaps( this, m_inputData, theSnapshot, m_inputData->getTopOfSedimentSurface(), m_debug );

         retrieveData();

         /// 3. Compute the backstripped density and thickness, the backtrip and the compensation
         LogHandler( LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_STEP ) << "computing Backstrip";
         CrustalThickness::DensityCalculator densityCalculator( *m_inputData, m_outputData, getValidator() );
         densityCalculator.compute();
         if (not m_debug) m_outputData.disableDebugOutput( this, m_inputData->getBotOfSedimentSurface(), theSnapshot );

         /// 4. Compute the Total Tectonic Subsidence (only if we have a SDH at this snapshot)
         if (hasSurfaceDepthHistory( age )){
            LogHandler( LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_STEP ) << "computing Tectonic Subsidence";
            CrustalThickness::TotalTectonicSubsidenceCalculator TTScalculator( *m_inputData, m_outputData, getValidator(),
               age, densityCalculator.getAirCorrection(), m_previousTTS, m_seaBottomDepth );
            TTScalculator.compute();
            // This is just for the first step when we compute the TTS at 0Ma, then we go in the reverse order
            if (k == 0){
               assert( age == 0.0 );
               presentDayTTS          = std::shared_ptr<GridMap>( getFactory()->produceGridMap( nullptr, 0, m_outputData.getMap( WLSMap ), identity ) );
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
         CrustalThickness::PaleowaterdepthCalculator PWDcalculator( *m_inputData, m_outputData, getValidator(), presentDayTTS.get() );
         PWDcalculator.compute();

         // 6. Compute the PaleowaterdepthResidual (only if we have a SDH at this snapshot and if we are not at present day)
         if (hasSurfaceDepthHistory( age ) and age!=0.0){
            LogHandler( LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_STEP ) << "computing PaleowaterdepthResidual";
            CrustalThickness::PaleowaterdepthResidualCalculator PWDRcalculator( *m_inputData, m_outputData, getValidator(),
               age, m_seaBottomDepth );
            PWDRcalculator.compute();
         }

         ///7. Computes the thinning factor and crustal thicknesses
         if (m_inputData->getRiftingCalculationMask( age )){
            LogHandler( LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_STEP ) << "computing Crustal Thicknesses";
            CrustalThickness::McKenzieCrustCalculator mcKenzieCalculator( *m_inputData, m_outputData, getValidator(), age,
               m_previousRiftTTS, m_previousContinentalCrustalThickness, m_previousOceanicCrustalThickness );
            mcKenzieCalculator.compute();
            m_previousRiftTTS = m_outputData.getMap( WLSadjustedMap );
            m_previousContinentalCrustalThickness = m_outputData.getMap( thicknessCrustMap  );
            m_previousOceanicCrustalThickness     = m_outputData.getMap( thicknessBasaltMap );
         }

         ///8. Smooth the PWD and Crustal Thicknesses maps
         smoothOutputs();

         restoreData();

         // 9. This is where all BC files from ctc are merged into Inputs.HDF for importing to BPA2-Basin
         m_outputData.saveOutput( this, m_debug, m_outputOptions, theSnapshot, m_mergeHDF );

         // Save properties to disk.
         continueActivity();
      }
   }
   
   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_STEP) << "CTC calculations done!";
   bool success = false;
   if (m_mergeHDF)
   {
       success = sortCTCOuputTbl();
       if (!success)
       {
           throw (std::invalid_argument("sorting CTC Table gone wrong!"));
       }
       // to write the reference in project3d file in GridMapIoTbl for the CTC output maps which are merged in Inputs.HDF file
       // done in CrustalThicknessCalculator::finalise
   }

   success = UpdateBPANameMappingTbl();
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
   PetscOptionsHasName (PETSC_IGNORE, PETSC_IGNORE, "-xyz", &isDefined);
   if (isDefined) {
      m_outputOptions |= XYZ;
   }

   PetscOptionsHasName (PETSC_IGNORE, PETSC_IGNORE, "-sur", &isDefined);
   if (isDefined) {
      m_outputOptions |= SUR;
   }

   PetscOptionsHasName (PETSC_IGNORE, PETSC_IGNORE, "-debug", &isDefined);
   if (isDefined) {
      m_debug = true;
   }

   PetscOptionsHasName (PETSC_IGNORE, PETSC_IGNORE, "-hdf", &isDefined);
   if (isDefined) {
      m_outputOptions |= HDF;
   }
   char outputFileName[Utilities::Numerical::MaxLineSize];
   outputFileName[0] = '\0';

   PetscOptionsGetString (PETSC_IGNORE, PETSC_IGNORE, "-save", outputFileName, Utilities::Numerical::MaxLineSize, &isDefined);
   if(isDefined) {
      m_outputFileName = outputFileName;
   }

   PetscOptionsHasName(PETSC_IGNORE, PETSC_IGNORE, "-merge", &isDefined);
   if (isDefined) {
       m_mergeHDF = true;
   }
   return true;
}

//------------------------------------------------------------//

void CrustalThicknessCalculator::setAdditionalOptionsFromCommandLine() {

   PetscBool isDefined = PETSC_FALSE;
   int radius;

   PetscOptionsHasName (PETSC_IGNORE, PETSC_IGNORE, "-nosmooth", &isDefined );
   if (isDefined) {
      m_applySmoothing = false;
      m_inputData->setSmoothingRadius( 0 );
   }

   PetscOptionsGetInt (PETSC_IGNORE, PETSC_IGNORE, "-smooth", &radius, &isDefined );
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
            const bool status = mapSmoother.averageSmoothing( m_outputData.getMap( mapsToSmooth[i] ) );
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
   if (m_previousRiftTTS != nullptr) {
      m_previousRiftTTS->retrieveData();
   }
}

//------------------------------------------------------------//

void CrustalThicknessCalculator::restoreData(){
   m_outputData.restoreData();
   if (m_previousTTS != nullptr) {
      m_previousTTS->restoreData();
   }
   if (m_previousRiftTTS != nullptr) {
      m_previousRiftTTS->restoreData();
   }
}

//---------------------------------------------------

//----------------------------------------------------
bool CrustalThicknessCalculator::sortCTCOuputTbl()
{
    bool success = true;
    string ctcRiftingHistoryTable;
    ctcRiftingHistoryTable = "CTCRiftingHistoryIoTbl";
    bool isFlexuralOtherThan_0Ma = false;
    double startOfFlexuralEvent = 0.;

    std::vector<std::string> listOfIoTbls { InterfaceOutput::s_SurfaceDepthTableName , InterfaceOutput::s_oceaCrustalThicknessTableName,
    InterfaceOutput::s_contCrustalThicknessTableName
    };

    // To add the crustal thickness at the start of passive events and to add the map calculated for a Flexural event at an age older than 0Ma to be used for 0 Ma
    database::Table* ctcRiftingHistoryIoTbl = this->getTable(ctcRiftingHistoryTable);
    size_t ctcRiftingTblSize = ctcRiftingHistoryIoTbl->size();
    std::vector<double> passiveRiftEventStartAges;
    std::string prevTectonicFlag = "Active Rifting"; //Always active for basement
    for (int k = (ctcRiftingTblSize - 2); k >= 0; k--)//Excluding the oldest record from this loop which always corresponds to basement age and 
    {
        database::Record* rec = ctcRiftingHistoryIoTbl->getRecord(k);
        std::string tectonicFlag = rec->getValue<std::string>("TectonicFlag");
        double age = rec->getValue<double>("Age");
        if (tectonicFlag == "Passive Margin" && prevTectonicFlag != "Passive Margin")
        {
            passiveRiftEventStartAges.push_back(age);

        }
        if (tectonicFlag == "Flexural Basin" && !NumericFunctions::isEqual(age, 0., 1e-5) && prevTectonicFlag != "Flexural Basin")
        {
            isFlexuralOtherThan_0Ma = true;
            startOfFlexuralEvent = age;
        }

        prevTectonicFlag = tectonicFlag;
    }

    //sort table with respect to current age to older age
    int i = 0;
    for (const auto &s_tblName : listOfIoTbls)
    {
        database::Table* ctcIoTbl = this->getTable(s_tblName);
        if (i != 0)//To create new records to specify the crustal thicknesses at the start of the passive events
        {
            for (const auto & newAge : passiveRiftEventStartAges)
            {
                database::Record* newRecord = ctcIoTbl->createRecord(true);
                database::setAge(newRecord, newAge);
                database::setThickness(newRecord, DataAccess::Interface::DefaultUndefinedScalarValue); //initialize thickness with default value. This will be updated below
                database::setThicknessGrid(newRecord, DataAccess::Interface::NullString); //initialize thicknessGrid with default value. This will be updated below
            }
            //Sort ctcIoTbl w.r.t age
            ctcIoTbl->sort([](database::Record* recordL, database::Record* recordR)
                {  bool isLess = database::getAge(recordL) < database::getAge(recordR);
                   return isLess;
                });

            for (int ii = 0; ii < (ctcIoTbl->size() - 1); ii++)
            {
                database::Record* record1 = ctcIoTbl->getRecord(ii);
                double age1 = record1->getValue<double>("Age");
                double thickness1 = record1->getValue<double>("Thickness");
                std::string thicknessGrid1 = record1->getValue<std::string>("ThicknessGrid");

                database::Record* record2 = ctcIoTbl->getRecord(ii + 1);
                double age2 = record2->getValue<double>("Age");
                double thickness2 = record2->getValue<double>("Thickness");
                std::string thicknessGrid2 = record2->getValue<std::string>("ThicknessGrid");

                if (NumericFunctions::isEqual(thickness2, DataAccess::Interface::DefaultUndefinedScalarValue, 1e-5) && thicknessGrid2 == "")
                {
                    if (age1 == age2)
                    {
                        ctcIoTbl->removeRecord(record1);
                        ii = ii-2;                 
                    }
                    else
                    {
                        database::setThickness(record2, thickness1);
                        database::setThicknessGrid(record2, thicknessGrid1);
                        ii++;
                    }          
                }
                if (age1 == startOfFlexuralEvent && isFlexuralOtherThan_0Ma == true)
                {
                    database::Record* newRecord1 = ctcIoTbl->createRecord(true);
                    database::setAge(newRecord1, 0);
                    database::setThickness(newRecord1, thickness1);
                    database::setThicknessGrid(newRecord1, thicknessGrid1);
                }            
            }
        }
        auto ictcTblSize = ctcIoTbl->size();
        for (int j = 0; j < (ictcTblSize - 1); ++j)
        {
            database::Record* record = ctcIoTbl->getRecord(j);
            int i_getIndex = 0;
            bool b_moveRecord = false;
            for (int k = (j + 1); k < ictcTblSize; ++k)
            {
                const database::Record* recordNext = ctcIoTbl->getRecord(k);
                if (record->getValue<double>(InterfaceOutput::s_ContOceaCrustalThicknessAgeFieldName) > 
                    recordNext->getValue<double>(InterfaceOutput::s_ContOceaCrustalThicknessAgeFieldName))
                {
                    i_getIndex = k;
                    b_moveRecord = true;
                }
            }
            if (b_moveRecord)
            {
                success = ctcIoTbl->moveRecord( ctcIoTbl->getRecord(i_getIndex) , (record) );
            }
        }
        ++i;
    }
    
    return success;
}

bool CrustalThicknessCalculator::UpdateBPANameMappingTbl() const
{
	std::vector<outputMaps> mapsToRename        =  { isostaticBathymetry, thicknessBasaltMap, thicknessCrustMap };
	std::vector<std::string> ctcTblsToChange    =  { "SurfaceDepthIoTbl","OceaCrustalThicknessIoTbl","ContCrustalThicknessIoTbl" };
    std::vector<std::string> GripMapColmnNm     =  { "DepthGrid", "ThicknessGrid" ,"ThicknessGrid" };

    database::Table* surfaceDepthIoTbl = this->getTable("SurfaceDepthIoTbl");
    surfaceDepthIoTbl->sort([](database::Record* recordL, database::Record* recordR)
        {  bool isLess = database::getAge(recordL) < database::getAge(recordR);
    return isLess;
        });

    database::Record* firstPWDRecord = surfaceDepthIoTbl->getRecord(0);
    std::string pwdMapNameFirstRecord = firstPWDRecord->getValue<std::string>("DepthGrid");

    database::Table* BPANameMappingIoTbl    = this->getTable("BPANameMapping");
    auto rcdNM                              = BPANameMappingIoTbl->begin();
    
    /// <summary>
    /// First, clean the BPANameMapping Table of old entries related to ctc output tables except for map references of PWD map @0Ma
    /// PWD map info at 0Ma is retained as this map is not required to be updated as per the naming conventions of ctc output maps
    /// </summary>
    /// <returns></returns>
    while (rcdNM != BPANameMappingIoTbl->end())
    {
        std::string val = (*rcdNM)->getValue<std::string>("TblIoMappingEncode");
        size_t found = val.find(ctcTblsToChange[0]);
        //Standard structure of TblIoMappingEncode field of BPANameMapping is "GridMapIoTbl:<Tablename>:<MapName>"
        size_t pos_SecondPipe = val.find_last_of(":");
        std::string mapName = val.substr(pos_SecondPipe+1);
		if (    val.find(ctcTblsToChange[0]) != std::string::npos  ||
                val.find(ctcTblsToChange[1]) != std::string::npos  ||
                val.find(ctcTblsToChange[2]) != std::string::npos 
            ) 
        {
            if (mapName.compare(pwdMapNameFirstRecord) )
            {
                LogHandler(LogHandler::DEBUG_SEVERITY) << "found old entry" << val << '\n';
                rcdNM = BPANameMappingIoTbl->removeRecord(rcdNM);
            }
            else
            {
                ++rcdNM;
            }
            
		}
        else
        {
            ++rcdNM;
        }
        
    }
    
    /// <summary>
    /// Make the map names in BPA2 for the CTC tbls, human readable in BPANameMapping
    /// </summary>
    /// <returns></returns>
    int i = 0;
    
	for (const auto& atbl : ctcTblsToChange)
    {
        database::Table* fastctcTbl = this->getTable(atbl);
        // the table might not exist
        if (fastctcTbl==0)
            continue;
        auto rcdCT = fastctcTbl->begin();
        // some p3 file in the RT suite has BasaltThicknessIoTbl not OceaCrustalThicknessIoTbl 
	    auto stop = fastctcTbl->end();
        if (rcdCT == stop)
            continue;// skip this table

	    --stop;// Go upto one less than end() since we check the next entry as well in each loop cycle
        bool isReused=false;
	    while (rcdCT != stop )
	    {
            isReused = false;
	        auto entry = (*rcdCT)->getValue<std::string>(GripMapColmnNm[i]);
	        if (entry.compare("")) {
	            //found a map
                LogHandler(LogHandler::DEBUG_SEVERITY) << "found map" << entry << '\n';
                if (atbl == "SurfaceDepthIoTbl" && entry == pwdMapNameFirstRecord)
                {
                    LogHandler(LogHandler::DEBUG_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Don't rename the PWD map @0Ma as this is not an output map from ctc. It is equal to the water depth of the present day stratigraphy";
                }
                else
                {
                    //construct the string to print
                    std::string toPrint("GridMapIoTbl:" + atbl + ":" + entry);
                    std::string newMapName;
                    std::stringstream stream;
                    // just retain 2 decimal places
                    stream << std::fixed << std::setprecision(2) << (*rcdCT)->getValue<double>(InterfaceOutput::s_ContOceaCrustalThicknessAgeFieldName);
                    std::string age1 = stream.str() + "Ma";
                    // Check if the map was reused at the preceding age too
                    if (!(*(rcdCT + 1))->getValue<std::string>(GripMapColmnNm[i]).compare(entry)) {
                        //match
                        // clear the stream or error state, 
                        stream.clear();
                        stream.str("");// assign null string, otherwise it appends to the existing
                        stream << std::fixed << std::setprecision(2) << (*(rcdCT + 1))->getValue<double>(InterfaceOutput::s_ContOceaCrustalThicknessAgeFieldName);
                        std::string age2 = stream.str();
                        LogHandler(LogHandler::DEBUG_SEVERITY) << "found map that's reused at age" << age2 << '\t' << entry << '\n';
                        std::string age = age1 + '_' + age2 + "Ma";
                        newMapName = m_outputData.name_a_map(mapsToRename[i], age);
                        // skip the next entry too
                        ++rcdCT;
                        isReused = true;
                    }
                    else
                    {
                        newMapName = m_outputData.name_a_map(mapsToRename[i], age1);
                    }
                    // Add the newly created record with new map names in BPANameMapping Tbl
                    database::Record* newRecord = BPANameMappingIoTbl->createRecord();
                    database::setTblIoMappingEncode(newRecord, toPrint);
                    database::setBPAColumnName(newRecord, "NAME");
                    database::setBPAColumnValue(newRecord, newMapName);
                }
	                            
	        }         
	        ++rcdCT;
            if (rcdCT == fastctcTbl->end()) 
                break;
	    }

		// take care of the last entry
		auto lastEntry = fastctcTbl->begin() + fastctcTbl->size() - 1;
		auto entry = (*lastEntry)->getValue<std::string>(GripMapColmnNm[i]);
		if (entry.compare("") || !isReused) {
			//found a map
			LogHandler(LogHandler::DEBUG_SEVERITY) << "found map" << entry << '\n';
			//construct the string to print
			std::string toPrint("GridMapIoTbl:" + atbl + ":" + entry);
			std::stringstream stream;
			// just retain 2 decimal places
			stream << std::fixed << std::setprecision(2) << (*lastEntry)->getValue<double>(InterfaceOutput::s_ContOceaCrustalThicknessAgeFieldName);
			std::string age = stream.str() + "Ma";
			std::string newMapName = m_outputData.name_a_map(mapsToRename[i], age);
			// Add the newly created record with new map names in BPANameMapping Tbl
			database::Record* newRecord = BPANameMappingIoTbl->createRecord();
			database::setTblIoMappingEncode(newRecord, toPrint);
			database::setBPAColumnName(newRecord, "NAME");
			database::setBPAColumnValue(newRecord, newMapName);
		}
        ++i;
    }

    // if it reached this point; all is fine
    return true;
}

bool CrustalThicknessCalculator::CleanGridMapIoTbl() const
{
	/// <summary>
	/// Note: Since we are clearing the GridMapIoTbl w.r.t CtC tbls, before any calculations, the map sq no. 
	/// may clash with previous entries already present in the base scenario in these tbls
	/// </summary>
	/// <returns></returns>
	std::vector<std::string> ctcTblsToChange = { "SurfaceDepthIoTbl","OceaCrustalThicknessIoTbl","ContCrustalThicknessIoTbl" };
    std::vector<std::string> StratIoTblMaps;
    database::Table* surfaceDepthIoTbl = this->getTable("SurfaceDepthIoTbl");
    surfaceDepthIoTbl->sort([](database::Record* recordL, database::Record* recordR)
        {  bool isLess = database::getAge(recordL) < database::getAge(recordR);
    return isLess;
        });

    database::Record* firstPWDRecord = surfaceDepthIoTbl->getRecord(0);
    std::string pwdMapNameFirstRecord= firstPWDRecord->getValue<std::string>("DepthGrid");

	database::Table* GridMapIoTblIoTbl = this->getTable("GridMapIoTbl"); 
	auto rcdNM = GridMapIoTblIoTbl->begin();
    
    rcdNM = GridMapIoTblIoTbl->begin();
	while (rcdNM != GridMapIoTblIoTbl->end())
	{
        auto val = (*rcdNM)->getValue<std::string>("ReferredBy");

		if (val.find(ctcTblsToChange[0]) != std::string::npos ||
            val.find(ctcTblsToChange[1]) != std::string::npos ||
			val.find(ctcTblsToChange[2]) != std::string::npos
			)
		{
			
            if ( !val.compare(ctcTblsToChange[0]) )
            {
                LogHandler(LogHandler::DEBUG_SEVERITY) << "to remove " << val << '\n';
                auto key = (*rcdNM)->getValue<std::string>("MapName");
                
                if (key!= pwdMapNameFirstRecord) {
                    rcdNM = GridMapIoTblIoTbl->removeRecord(rcdNM);
                }
                else
                {
                    ++rcdNM;
                }
            }
            else
            {
                // remove all entries associated with Ocean & CrustThickness
				LogHandler(LogHandler::DEBUG_SEVERITY) << "to remove " << val << '\n';
				rcdNM = GridMapIoTblIoTbl->removeRecord(rcdNM);
            }
            
		}
		else
		{
			++rcdNM;
		}

	}
    // if it reached this point; all is fine
    return true;
}



