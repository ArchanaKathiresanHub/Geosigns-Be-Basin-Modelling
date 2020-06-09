//
// Copyright (C) 2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "InterfaceOutput.h"

// std library
#include <stdarg.h>

// CrustalThickness library
#include "InterfaceInput.h"

// DataAccess library
#include "ProjectHandle.h"
#include "CrustFormation.h"
#include "MapWriter.h"
#include "ProjectHandle.h"
#include "Snapshot.h"
#include "Surface.h"

// GeoPhysics library
#include "GeoPhysicsProjectHandle.h"

// CrustalThickness library
#include "InterfaceInput.h" 
#include "CTCPropertyValue.h"

// utilitites
#include "LogHandler.h"
#include "ConstantsNames.h"
# include "ConstantsNumerical.h"

//cmbAPI
#include "MapsManager.h"
#include "cmbAPI.h"
#include "BottomBoundaryManager.h"

#include <algorithm>

using namespace mbapi;

using namespace DataAccess;
using namespace Interface;
using namespace CrustalThicknessInterface;

const char * InterfaceOutput::s_contCrustalThicknessTableName = "ContCrustalThicknessIoTbl";
const char * InterfaceOutput::s_oceaCrustalThicknessTableName = "OceaCrustalThicknessIoTbl";
const char * InterfaceOutput::s_SurfaceDepthTableName = "SurfaceDepthIoTbl";
const char * InterfaceOutput::s_GridMapIoTblName = "GridMapIoTbl";
const char * InterfaceOutput::s_GridMapRefByFieldName = "ReferredBy";
const char * InterfaceOutput::s_GridMapMapNameFieldName = "MapName";
const char * InterfaceOutput::s_GridMapMapTypeFieldName = "MapType";
const char * InterfaceOutput::s_GridMapMapFileNameFieldName = "MapFileName";
const char * InterfaceOutput::s_GridMapMapSeqNbrFieldName = "MapSeqNbr";
const char * InterfaceOutput::s_ContOceaCrustalThicknessAgeFieldName = "Age";
const char * InterfaceOutput::s_ContOceaCrustalThicknessFieldName = "Thickness";
const char * InterfaceOutput::s_ContOceaCrustalThicknessGridFieldName = "ThicknessGrid";
const char * InterfaceOutput::s_SurfaceDepthioTblAgeFieldName = "Age";
const char * InterfaceOutput::s_SurfaceDepthIoTblDepthFieldName = "Depth";
const char * InterfaceOutput::s_SurfaceDepthIoTblDepthGridFieldName = "DepthGrid";

//------------------------------------------------------------//
InterfaceOutput::InterfaceOutput() {
   clean();
}

//------------------------------------------------------------//
InterfaceOutput::~InterfaceOutput() {
  clean();
}

//------------------------------------------------------------//
void InterfaceOutput::clean() {

   for( int i = 0; i < numberOfOutputMaps; ++ i ) {
      m_outputMaps[i] = 0;
      m_outputMapsMask[i] = false;
      m_outputValues[i] = 0;
   }
}

//------------------------------------------------------------//
void InterfaceOutput::retrieveData() {

   for( int i = 0; i < numberOfOutputMaps; ++ i ) {
      if( m_outputMaps[i] != 0 ) {
         m_outputMaps[i]->retrieveData();
      }
   }
}

//------------------------------------------------------------//
void InterfaceOutput::restoreData() {

   for( int i = 0; i < numberOfOutputMaps; ++ i ) {
      if( m_outputMaps[i] != 0 ) {
         m_outputMaps[i]->restoreData();
      }
   }
}

//------------------------------------------------------------//
bool InterfaceOutput::saveOutputMaps( Interface::ProjectHandle * projectHandle, const Snapshot * theSnapshot ) {

   LogHandler( LogHandler::DEBUG_SEVERITY ) << "saveOutputMaps: My rank is " << projectHandle->getRank();

   
   const Interface::Formation * formationCrust = dynamic_cast<const Interface::Formation *>(projectHandle->getCrustFormation ());
   const Interface::Surface   * topOfCrust = formationCrust->getTopSurface();
   const string topCrustSurfaceName = topOfCrust->getName();

   float time = (float) theSnapshot->getTime ();

   const string extensionString = ".HDF";
   Interface::MapWriter * mapWriter = projectHandle->getFactory()->produceMapWriter();

   const string dirToOutput = projectHandle->getProjectName() + Utilities::Names::CauldronOutputDir + "/";

   for( int i = 0; i < numberOfOutputMaps; ++ i ) {
      if( m_outputMapsMask[i] != 0 && m_outputMaps[i] != 0) {
         string outputFileName =  dirToOutput + projectHandle->getProjectName() + "_" + outputMapsNames[i] + extensionString;

         // Put 0 as a DataSetName to make comparison with regression tests results easier. Also 0 should be there if we want to re-use the map in fastcauldron
         string dataSetName = "0"; //outputMapsNames[i];
         dataSetName += "_";
         dataSetName += theSnapshot->asString();
         dataSetName += "_";
         dataSetName += topCrustSurfaceName;

         mapWriter->open( outputFileName, false );
         mapWriter->saveDescription (projectHandle->getActivityOutputGrid ());

         mapWriter->writeMapToHDF (m_outputMaps[i], time, time, dataSetName, topCrustSurfaceName);
         mapWriter->close();

         LogHandler( LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP ) << "Map " << outputMapsNames[i] << " saved into " << outputFileName << ".";
      }
   }
   delete mapWriter;
   return true;
}

bool InterfaceOutput::mergeOutputMapsToInputs(DataAccess::Interface::ProjectHandle* pHandle, const DataAccess::Interface::Snapshot* theSnapshot)
{
    bool success = false;
    LogHandler(LogHandler::DEBUG_SEVERITY) << "saveOutputMaps: My rank is " << pHandle->getRank();

    std::vector<outputMaps> mapsToMerge = { isostaticBathymetry, thicknessBasaltMap, thicknessCrustMap };
    int mapsSequenceNbr = 0;
    string outputFileName = "Inputs.HDF";
    std::fstream file;

    Interface::MapWriter* mapWriter = pHandle->getFactory()->produceMapWriter();
    file.open(outputFileName);
    if (!file)
       success = mapWriter->open(outputFileName, false);
    else
       success = mapWriter->open(outputFileName, true);
    file.close();


    //to get the latest Maps Sequence Number from GridMapIoTbl
    mapsSequenceNbr = InterfaceOutput::getMapsSequenceNbr(pHandle);

    for (int i = 0; i < mapsToMerge.size(); ++i) {
       auto ii =  mapsToMerge[i];
       if (m_outputMapsMask[ii] != 0 && m_outputMaps[ii] != 0) {
          mapsSequenceNbr++; // increment by 1

          // to write ctc maps in Inputs.HDF file
          success = mapWriter->writeInputMap(m_outputMaps[ii], mapsSequenceNbr);
          LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Map " << outputMapsNames[ii] << " saved into " << outputFileName << ".";

          // append newly added ctc map references in GridMapIoTbl record
          success = InterfaceOutput::addRecordsInGridMapIoTbl(pHandle, mapsSequenceNbr, mapsToMerge[i]);
          
          // Here we update the projectHandle with new record, right? the actual writing happens in saveToFile( m_projectFileName )
          success = InterfaceOutput::addCTCoutputMapRecordsInProject3dIoTbl(pHandle, theSnapshot, mapsSequenceNbr, mapsToMerge[i]);
        }
    }

    success = mapWriter->close();
    delete mapWriter;
    return success;
}

int InterfaceOutput::getMapsSequenceNbr(DataAccess::Interface::ProjectHandle* pHandle)
{
    int nextMapSeqVal, mapsSequenceNbr = 0;
    database::Table::iterator tblIter;
    database::Record* gridMapRecord;

    database::Table* gridMapTbl = pHandle->getTable(s_GridMapIoTblName);

    for (tblIter = gridMapTbl->begin(); tblIter != gridMapTbl->end(); ++tblIter)
    {
        gridMapRecord = gridMapTbl->getRecord(tblIter);
        nextMapSeqVal = gridMapRecord->getValue<int>(s_GridMapMapSeqNbrFieldName);
        if (tblIter != gridMapTbl->begin())
            mapsSequenceNbr = (mapsSequenceNbr > nextMapSeqVal) ? mapsSequenceNbr : nextMapSeqVal;
        else
            mapsSequenceNbr = nextMapSeqVal;
    }

    return mapsSequenceNbr;
}

//------------------------------------------------------------//
bool InterfaceOutput::addCTCoutputMapRecordsInProject3dIoTbl(DataAccess::Interface::ProjectHandle* pHandle, const DataAccess::Interface::Snapshot* theSnapshot, int mapsSequenceNbr, int mapsToMerge)
{
    bool success = false;
    string s_tblName;
    database::Table::iterator tblIter;
    database::Record* crustThckIoTblRecord;
    if (mapsToMerge == isostaticBathymetry)
        s_tblName = s_SurfaceDepthTableName;
    else if (mapsToMerge == thicknessBasaltMap)
        s_tblName = s_oceaCrustalThicknessTableName;
    else if (mapsToMerge == thicknessCrustMap)
        s_tblName = s_contCrustalThicknessTableName;
    else
        return true;

    database::Table* crustThckIoTbl = pHandle->getTable(s_tblName);

    //crustThckIoTbl->clear(true);
    crustThckIoTbl->createRecord();
    crustThckIoTblRecord = crustThckIoTbl->getRecord((int)crustThckIoTbl->size() - 1);

    if (mapsToMerge == isostaticBathymetry)
    {
        crustThckIoTblRecord->setValue<double>(s_SurfaceDepthioTblAgeFieldName, theSnapshot->getTime());
        crustThckIoTblRecord->setValue<double>(s_SurfaceDepthIoTblDepthFieldName, -9999);
        crustThckIoTblRecord->setValue<std::string>(s_SurfaceDepthIoTblDepthGridFieldName, "MAP-" + std::to_string(mapsSequenceNbr));
    }
    else
    {
        crustThckIoTblRecord->setValue<double>(s_ContOceaCrustalThicknessAgeFieldName, theSnapshot->getTime());
        crustThckIoTblRecord->setValue<double>(s_ContOceaCrustalThicknessFieldName, -9999);
        crustThckIoTblRecord->setValue<std::string>(s_ContOceaCrustalThicknessGridFieldName, "MAP-" + std::to_string(mapsSequenceNbr));
    }

    //delete old records of same age
    for (tblIter = crustThckIoTbl->begin(); tblIter != (crustThckIoTbl->end() - 1); ++tblIter)
    {
        database::Record* record = *tblIter;
        if (crustThckIoTblRecord->getValue<double>(s_ContOceaCrustalThicknessAgeFieldName) == record->getValue<double>(s_ContOceaCrustalThicknessAgeFieldName))
        {
            success = crustThckIoTbl->eraseRecord(record);
            break;
        }
    }

    return success;
}


bool InterfaceOutput::addRecordsInGridMapIoTbl(DataAccess::Interface::ProjectHandle* pHandle, int mapsSequenceNbr, int mapsToMerge)
{
   bool success = false;
   database::Table* gridMapTbl = pHandle->getTable(s_GridMapIoTblName);
   database::Record * gridMapRecord;
   string outputFileName = "Inputs.HDF";

   gridMapTbl->createRecord();
   gridMapRecord = gridMapTbl->getRecord((int)gridMapTbl->size() - 1);

   if (mapsToMerge == isostaticBathymetry)
      gridMapRecord->setValue<std::string>(s_GridMapRefByFieldName, s_SurfaceDepthTableName);
   else if (mapsToMerge == thicknessBasaltMap)
      gridMapRecord->setValue<std::string>(s_GridMapRefByFieldName, s_oceaCrustalThicknessTableName);
   else
      gridMapRecord->setValue<std::string>(s_GridMapRefByFieldName, s_contCrustalThicknessTableName);

   gridMapRecord->setValue<std::string>(s_GridMapMapNameFieldName, "MAP-" + std::to_string(mapsSequenceNbr));
   gridMapRecord->setValue<std::string>(s_GridMapMapTypeFieldName, "HDF5");
   gridMapRecord->setValue<std::string>(s_GridMapMapFileNameFieldName, outputFileName);
   gridMapRecord->setValue(s_GridMapMapSeqNbrFieldName, mapsSequenceNbr);

   return success;
}

//------------------------------------------------------------//
bool InterfaceOutput::saveXYZOutputMaps( Interface::ProjectHandle * projectHandle ) {

   LogHandler( LogHandler::DEBUG_SEVERITY ) << "saveXYZOutputMaps: My rank is " << projectHandle->getRank();
   
   const string extensionString = ".XYZ";
   ofstream outputFileCrust;

   const Interface::Grid * grid = projectHandle->getActivityOutputGrid ();

   unsigned int i, j, k;
   double posI, posJ;
   unsigned lastI = grid->numI();
   unsigned lastJ = grid->numJ();

   for( k = 0; k < numberOfOutputMaps; ++ k ) {
      if( m_outputMapsMask[k] != 0 && m_outputMaps[k] != 0 ) {
         string outputFileName = projectHandle->getProjectName() + "_" + outputMapsNames[k] + extensionString;

         outputFileCrust.open (outputFileName.c_str ());
         if (outputFileCrust.fail ()) {
            LogHandler( LogHandler::ERROR_SEVERITY ) << "Could not open XYZ output file for map " << outputMapsNames[k];
            continue;
         }
         m_outputMaps[k]->retrieveData();
         for ( i = 0; i < lastI; ++ i ) {
            for ( j = 0; j < lastJ; ++ j ) {

               grid->getPosition( i, j, posI, posJ );
               outputFileCrust << posI << ", " << posJ << ", ";
               outputFileCrust << m_outputMaps[k]->getValue(i, j) << endl;
            }
         }
         m_outputMaps[k]->restoreData();
         outputFileCrust.close();
         LogHandler( LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP ) << "Map " << outputMapsNames[k] << " saved into " << outputFileName << ".";
      }
   }
   return true;
}

//------------------------------------------------------------//
bool InterfaceOutput::saveExcelSurfaceOutputMaps( Interface::ProjectHandle * projectHandle ) {

   LogHandler( LogHandler::DEBUG_SEVERITY ) << "saveExcelSurfaceOutputMaps: My rank is " << projectHandle->getRank();
   
   const string extensionString = ".SUR";
   ofstream outputFileCrust;

   const Interface::Grid * grid = projectHandle->getActivityOutputGrid ();

   unsigned int i, j, k;
   double posI, posJ;
   unsigned lastI = grid->numI();
   unsigned lastJ = grid->numJ();

   for( k = 0; k < numberOfOutputMaps; ++ k ) {
      if( m_outputMapsMask[k] != 0 && m_outputMaps[k] != 0 ) {
         string outputFileName = projectHandle->getProjectName() + "_" + outputMapsNames[k] + extensionString;

         outputFileCrust.open (outputFileName.c_str ());
         if (outputFileCrust.fail ()) {
            LogHandler( LogHandler::ERROR_SEVERITY ) << "Could not open XYZ output file for map " << outputMapsNames[k];
            continue;
         }
         m_outputMaps[k]->retrieveData();

         outputFileCrust << ", ";
         for ( j = 0; j < lastJ; ++ j ) {
            grid->getPosition( 0, j, posI, posJ );
            outputFileCrust << posJ << ", ";
         }
         outputFileCrust << endl;

         for ( i = 0; i < lastI; ++ i ) {
            grid->getPosition( i, 0, posI, posJ );
            outputFileCrust << posI << ", " ;
            for ( j = 0; j < lastJ; ++ j ) {
               outputFileCrust << m_outputMaps[k]->getValue(i, j) << ", ";
            }
            outputFileCrust << endl;
         }
         m_outputMaps[k]->restoreData();
         outputFileCrust.close();
         LogHandler( LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP ) << "Map " << outputMapsNames[k] << " saved into " << outputFileName;
      }
   }
   return true;
}

//------------------------------------------------------------//
void InterfaceOutput::setMapsToOutput(const CrustalThicknessInterface::outputMaps mapIndex, ... ) {

   va_list vlist;
   va_start( vlist, mapIndex );

   int i = mapIndex;
   while( i < CrustalThicknessInterface::numberOfOutputMaps ) {
      m_outputMapsMask[i] = true;
      i = va_arg( vlist, int );
   }
   va_end( vlist );
}

//------------------------------------------------------------//
void InterfaceOutput::updatePossibleOutputsAtSnapshot( const outputMaps id,
                                                       const GeoPhysics::ProjectHandle * pHandle,
                                                       std::shared_ptr< const InterfaceInput > interfaceInput,
                                                       const Snapshot * theSnapshot,
                                                       const bool debug ) {
   
   bool toBeOutput = true;
   // The McKenzie general properties are only ouput when we have an SDH at the end of a rifting event
   if (  id == RDAadjustedMap
      or id == thicknessCrustMap
      or id == thicknessBasaltMap
      or id == thicknessCrustMeltOnset
      or id == topBasaltMap
      or id == mohoMap
      or id == ECTMap ){
      if (not interfaceInput->getRiftingCalculationMask( theSnapshot->getTime() )){
         toBeOutput = false;
      }
   }

   // The TTS and Incremental TS properties are only ouput when we have an SDH
   else if ( id == WLSMap
          or id == WLSadjustedMap
          or id == incTectonicSubsidence) {
      if (not pHandle->hasSurfaceDepthHistory( theSnapshot->getTime() )) {
         toBeOutput = false;
      }
   }

   // The PWD is not output at 0.0Ma (it is equal to the water depth of the input stratigraphy)
   else if (id == isostaticBathymetry) {
      if (theSnapshot->getTime() == 0.0) {
         toBeOutput = false;
      }
   }

   // The PWDR is only output when we have an SDH and never output at 0.0Ma (since we do not have a PWD at 0.0Ma)
   else if (id == PaleowaterdepthResidual) {
      if (not pHandle->hasSurfaceDepthHistory( theSnapshot->getTime() ) or theSnapshot->getTime() == 0.0) {
         toBeOutput = false;
      }
   }

   // The debug outputs
   // The McKenzie debug properties are only ouput when we have an SDH at the end of a rifting event and in debug mode
   else if ( id == estimatedCrustDensityMap
      or id == basaltDensityMap
      or id == PTaMap
      or id == TFOnsetMap
      or id == TFOnsetMigMap
      or id == TFOnsetLinMap
      or id == WLSExhumeMap
      or id == WLSCritMap
      or id == WLSOnsetMap
      or id == WLSExhumeSerpMap
      or id == slopePreMelt
      or id == slopePostMelt
      or id == interceptPostMelt
      or id == TFMap
      or id == UpperContinentalCrustThickness
      or id == LowerContinentalCrustThickness
      or id == UpperOceanicCrustThickness
      or id == LowerOceanicCrustThickness) {
      if (not debug or not interfaceInput->getRiftingCalculationMask( theSnapshot->getTime() )) {
         toBeOutput = false;
      }
   }
   // The response factor is only output in debug mode exept for present day
   else if (id == ResponseFactor) {
      if (not debug or theSnapshot->getTime() == 0.0){
         toBeOutput = false;
      }
   }
   
   m_outputMapsMask[id] = toBeOutput;
}

//------------------------------------------------------------//
void InterfaceOutput::setAllMapsToOutput( const bool flag ) {
 
   for( int i = 0; i < numberOfOutputMaps; ++ i ) {
      m_outputMapsMask[i] = flag;
   }

}

//------------------------------------------------------------//
void InterfaceOutput::createSnapShotOutputMaps( GeoPhysics::ProjectHandle * pHandle,
                                                std::shared_ptr< const InterfaceInput > interfaceInput,
                                                const Snapshot * theSnapshot,
                                                const Interface::Surface *theSurface,
                                                const bool debug ) {
   
   LogHandler( LogHandler::DEBUG_SEVERITY ) << "Create snaphot output maps @ snapshot " << theSnapshot->asString();
   for( int i = 0; i < numberOfOutputMaps; ++ i ) {
      outputMaps id = (outputMaps)i;
      updatePossibleOutputsAtSnapshot( id, pHandle, interfaceInput, theSnapshot, debug );
      if( m_outputMapsMask[i] ) {
         if( id != isostaticBathymetry && id != incTectonicSubsidence ) {
            LogHandler( LogHandler::DEBUG_SEVERITY ) << "   #for map " << outputMapsNames[i];
            m_outputMaps[i] = createSnapshotResultPropertyValueMap(pHandle, outputMapsNames[i], theSnapshot);
         }
         else {
            LogHandler( LogHandler::DEBUG_SEVERITY ) << "   #for map " << outputMapsNames[i];
            m_outputMaps[i] = createSnapshotResultPropertyValueMap(pHandle, outputMapsNames[i], theSnapshot, theSurface );
         }
         if( m_outputMaps[i] == nullptr ) {
            throw std::runtime_error( "Cannot allocate output map " + outputMapsNames[i] );
         }
      }
   }
}

//------------------------------------------------------------//
GridMap * InterfaceOutput::createSnapshotResultPropertyValueMap (ProjectHandle * pHandle, const std::string& propertyName, 
                                                                 const Snapshot* theSnapshot, const Interface::Surface *theSurface ) {

   const Interface::Formation * formationCrust = dynamic_cast<const Interface::Formation *>(pHandle->getCrustFormation ());
   const Interface::Surface   * topSurface = ( theSurface != 0 ? theSurface : formationCrust->getTopSurface() );

   PropertyValue *thePropertyValue = pHandle->createMapPropertyValue (propertyName, theSnapshot, 0, 0, topSurface);
   GridMap *theMap = 0;

   if(thePropertyValue) {
      theMap = thePropertyValue->getGridMap();
   }

   return theMap;
}

//------------------------------------------------------------//
void InterfaceOutput::disableDebugOutput( ProjectHandle * pHandle, const Interface::Surface* theSurface, const Snapshot* theSnapshot ) const {
   disableOutput( pHandle, theSurface, theSnapshot, "SedimentBackstrip"                     );
   disableOutput( pHandle, theSurface, theSnapshot, "IncrementalTectonicSubsidenceAdjusted" );
   disableOutput( pHandle, theSurface, theSnapshot, "TF"                                    );
}

//------------------------------------------------------------//
void InterfaceOutput::disableOutput( ProjectHandle * pHandle, const Interface::Surface* theSurface, const Snapshot* theSnapshot, const std::string& name ) const {
   const Interface::Property * property = pHandle->findProperty( name );
   Interface::PropertyValueList* propVals;
   Interface::PropertyValueList::const_iterator propValIter;
   propVals = pHandle->getPropertyUnrecordedValues( Interface::SURFACE, property, theSnapshot, 0, 0, theSurface, Interface::MAP );
   if (propVals->size() != 0){
      Ctc::CTCPropertyValue *thePropertyValue = const_cast<Ctc::CTCPropertyValue*>(dynamic_cast<const Ctc::CTCPropertyValue*>((*propVals)[0]));
      if (thePropertyValue != nullptr){
         thePropertyValue->allowOutput( false );
      }
      else{
         LogHandler( LogHandler::ERROR_SEVERITY ) << "Cannot disable output '" << name << "' at snapshot " << theSnapshot->getTime();
      }
   }
   delete propVals;
}

//------------------------------------------------------------//
void InterfaceOutput::saveOutput( Interface::ProjectHandle * pHandle, bool isDebug, int outputOptions, const Snapshot * theSnapshot, bool merge ) {

   bool success = true;
   LogHandler( LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_STEP ) << "save maps to local disk";
   if( isDebug ) {
      if( outputOptions & XYZ ) {
         if( pHandle->getSize() > 1 ) {
            LogHandler( LogHandler::ERROR_SEVERITY ) << "Can not save maps in XYZ format in parallel. Run with nprocs = 1 to save as XYZ.";
         } else {
            saveXYZOutputMaps( pHandle );
         }
      }
      if( outputOptions & SUR ) {
         if( pHandle->getSize() > 1 ) {
            LogHandler( LogHandler::ERROR_SEVERITY ) << "Can not save maps in SUR format in parallel. Run with nprocs = 1 to save as SUR.";
         }
         else {
            saveExcelSurfaceOutputMaps( pHandle );
         }
      }
      if( outputOptions & HDF ) {
         saveOutputMaps( pHandle, theSnapshot );
      }
   } else if( outputOptions & XYZ ) {
      saveXYZOutputMaps( pHandle );
   } else  if( outputOptions & HDF ) {
      saveOutputMaps( pHandle, theSnapshot );
   }
   else if (merge) {
      success = mergeOutputMapsToInputs(pHandle, theSnapshot);
      if (!success)
      {
         throw ErrorHandler::Exception(ErrorHandler::IoError) << "Failed to merge output maps.";
      }
   }
   
}
