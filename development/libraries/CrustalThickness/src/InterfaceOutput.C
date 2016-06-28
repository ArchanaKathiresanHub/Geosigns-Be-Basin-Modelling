//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
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

// DataAccess library
#include "Interface/ProjectHandle.h"
#include "Interface/CrustFormation.h"
#include "Interface/PropertyValue.h"
#include "Interface/Snapshot.h"
#include "Interface/Surface.h"

// CrustalThickness library
#include "InterfaceInput.h" 
#include "PropertyValue.h"

// utilitites
#include "LogHandler.h"

using namespace CrustalThicknessInterface;

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
   
   const string dirToOutput = projectHandle->getProjectName() + "_CauldronOutputDir/";

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

         LogHandler( LogHandler::INFO_SEVERITY ) << "Map " << outputMapsNames[i] << " is saved into " << outputFileName << ".";
      }
   }
   delete mapWriter;
   return true;
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
            LogHandler( LogHandler::ERROR_SEVERITY ) << "Could not open XYZ output file for map " << outputMapsNames[i];
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
         LogHandler( LogHandler::INFO_SEVERITY ) << "Map " << outputMapsNames[k] << " is saved into " << outputFileName << ".";
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
            LogHandler( LogHandler::ERROR_SEVERITY ) << "Could not open XYZ output file for map " << outputMapsNames[i];
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
         LogHandler( LogHandler::INFO_SEVERITY ) << "Map " << outputMapsNames[k] << " is saved into " << outputFileName;
      }
   }
   return true; 
}

//------------------------------------------------------------//
void InterfaceOutput::setMapsToOutput(CrustalThicknessInterface::outputMaps mapIndex, ... ) {

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
void InterfaceOutput::updatePossibleOutputsAtSnapshot( outputMaps id, const GeoPhysics::ProjectHandle * pHandle, const Snapshot * theSnapshot ) {
   // The TTS and Incremental TS are only ouput when we have an SDH
   if (id == WLSMap or id == incTectonicSubsidence){
      if (not pHandle->asSurfaceDepthHistory( theSnapshot->getTime() )){
         m_outputMapsMask[id] = false;
      }
      else{
         m_outputMapsMask[id] = true;
      }
   }
   // The PWD is not output at 0.0Ma
   else if (id == isostaticBathymetry){
      if (theSnapshot->getTime() == 0.0)
      {
         m_outputMapsMask[id] = false;
      }
      else{
         m_outputMapsMask[id] = true;
      }
   }
   // The PWDR is only output when we have an SDH and never output at 0.0Ma
   else if (id == PaleowaterdepthResidual){
      if (not pHandle->asSurfaceDepthHistory( theSnapshot->getTime() ) or theSnapshot->getTime() == 0.0)
      {
         m_outputMapsMask[id] = false;
      }
      else{
         m_outputMapsMask[id] = true;
      }
   }
}

//------------------------------------------------------------//
void InterfaceOutput::setAllMapsToOutput( bool flag ) {
 
   for( int i = 0; i < numberOfOutputMaps; ++ i ) {
      m_outputMapsMask[i] = flag;
   }

}

//------------------------------------------------------------//
bool InterfaceOutput::createSnapShotOutputMaps( GeoPhysics::ProjectHandle * pHandle, const Snapshot * theSnapshot, const Interface::Surface *theSurface ) {
   
   LogHandler( LogHandler::DEBUG_SEVERITY ) << "Create snaphot output maps @ snapshot " << theSnapshot->asString();
   int i;
   bool status = true;
   if( theSnapshot->getTime() == 0.0 ) {
      // Output these properties for present-day only
      for( i = 0; i < WLSMap; ++ i ) {
         updatePossibleOutputsAtSnapshot( (outputMaps)i, pHandle, theSnapshot );
         if( m_outputMapsMask[i] ) {
            LogHandler( LogHandler::DEBUG_SEVERITY ) << "   #for map " << outputMapsNames[i];
            m_outputMaps[i] = createSnapshotResultPropertyValueMap(pHandle, outputMapsNames[i], theSnapshot);
            if( m_outputMaps[i] == 0 ) {
               status = false;
               break;
            }
         }
      }
   }
   if( status ) {
      for( i = WLSMap; i < numberOfOutputMaps; ++ i ) {
         outputMaps id = (outputMaps)i;
         updatePossibleOutputsAtSnapshot( id, pHandle, theSnapshot );
         if( m_outputMapsMask[i] ) {
            if( id != isostaticBathymetry && id != incTectonicSubsidence ) {
               LogHandler( LogHandler::DEBUG_SEVERITY ) << "   #for map " << outputMapsNames[i];
               m_outputMaps[i] = createSnapshotResultPropertyValueMap(pHandle, outputMapsNames[i], theSnapshot);
            }
            else {
               LogHandler( LogHandler::DEBUG_SEVERITY ) << "   #for map " << outputMapsNames[i];
               m_outputMaps[i] = createSnapshotResultPropertyValueMap(pHandle, outputMapsNames[i], theSnapshot, theSurface );
            }
            if( m_outputMaps[i] == 0 ) {
               status = false;
               break;
            }
         }
      }
   }
   return status;
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
void InterfaceOutput::disableBackstripOutput( ProjectHandle * pHandle, const Interface::Surface* theSurface, const Snapshot* theSnapshot ) const {
   disableOutput( pHandle, theSurface, theSnapshot, "SedimentBackstrip" );
}

//------------------------------------------------------------//
void InterfaceOutput::disableOutput( ProjectHandle * pHandle, const Interface::Surface* theSurface, const Snapshot* theSnapshot, const std::string& name ) const {
   const Interface::Property * property = pHandle->findProperty( name );
   Interface::PropertyValueList* propVals;
   Interface::PropertyValueList::const_iterator propValIter;
   propVals = pHandle->getPropertyUnrecordedValues( Interface::SURFACE, property, theSnapshot, 0, 0, theSurface, Interface::MAP );
   if (propVals->size() != 0){
      Ctc::PropertyValue *thePropertyValue = const_cast<Ctc::PropertyValue*>(dynamic_cast<const Ctc::PropertyValue*>((*propVals)[0]));
      if (thePropertyValue != nullptr){
         thePropertyValue->allowOutput( false );
      }
      else{
         LogHandler( LogHandler::ERROR_SEVERITY ) << "Cannot disable output '" << name << "' at snapshot " << theSnapshot->getTime();
      }
   }
   else{
      LogHandler( LogHandler::ERROR_SEVERITY ) << "Cannot disable output '" << name << "' at snapshot " << theSnapshot->getTime() <<
         " because the property value does not exist.";
   }
   delete propVals;
}

//------------------------------------------------------------//
void InterfaceOutput::saveOutput( Interface::ProjectHandle * pHandle, bool isDebug, int outputOptions, const Snapshot * theSnapshot ) {

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
   
}

//------------------------------------------------------------//
namespace CrustalThicknessInterface {

outputMaps getPropertyId(const string & propertyName) {

   for( int i = 0; i < numberOfOutputMaps; ++ i ) {
      if( propertyName == outputMapsNames[i] ) {
         return ( outputMaps )i;
      }
   }

   return numberOfOutputMaps;
}

}
