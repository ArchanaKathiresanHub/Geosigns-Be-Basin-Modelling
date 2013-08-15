#include <stdarg.h>

#include "Interface/ProjectHandle.h"
#include "Interface/CrustFormation.h"
#include "Interface/PropertyValue.h"
#include "Interface/Snapshot.h"
#include "Interface/Surface.h"

#include "InterfaceOutput.h"
#include "InterfaceInput.h" 

using namespace CrustalThicknessInterface;

//------------------------------------------------------------//
InterfaceOutput::InterfaceOutput() {

   clean();
}
//------------------------------------------------------------//
InterfaceOutput::~InterfaceOutput() {

   for( int i = 0; i < numberOfOutputMaps; ++ i ) {
      if( m_outputMaps[i] != 0 ) {

         // Where properties maps are deleted???

         // m_outputMaps[i]->release();
         //delete m_outputMaps[i];
      }
   }

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

   // cout << "My rank is " << CrustalThicknessInterface::GetRank() << endl;
   // char ageString[64];
   // sprintf(ageString, "_%lf", 0);
   
   const Interface::Formation * formationCrust = dynamic_cast<const Interface::Formation *>(projectHandle->getCrustFormation ());
   const Interface::Surface   * topOfCrust = formationCrust->getTopSurface();
   const string topCrustSurfaceName = topOfCrust->getName();
   
   float time = (float) theSnapshot->getTime ();

   const string extensionString = ".HDF";
   Interface::MapWriter * mapWriter = projectHandle->getFactory()->produceMapWriter();
   
   const string dirToOutput = projectHandle->getProjectName() + "_CauldronOutputDir/";

   for( int i = 0; i < numberOfOutputMaps; ++ i ) {
      if( m_outputMapsMask[i] != 0 && m_outputMaps[i] != 0) {
         //        string outputFileName = projectHandle->getProjectName() + "_" + outputMapsNames[i] + string(ageString) + extensionString;
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

         if( projectHandle->getRank() == 0 ) {
            cout << "Map " << outputMapsNames[i] << " is saved into " << outputFileName <<  endl;
         }
      }
   }
   delete mapWriter;
   return true;
}

//------------------------------------------------------------//
bool InterfaceOutput::saveXYZOutputMaps( Interface::ProjectHandle * projectHandle ) {

   // cout << "My rank is " << CrustalThicknessInterface::GetRank() << endl;
   // char ageString[64];
   // sprintf(ageString, "_%lf", 0);
   
   const string extensionString = ".XYZ";
   ofstream outputFileCrust;
   
   const Interface::Grid * grid = projectHandle->getActivityOutputGrid ();
   
   unsigned int i, j, k;
   double posI, posJ;
   unsigned lastI = grid->numI();
   unsigned lastJ = grid->numJ();

   for( k = 0; k < numberOfOutputMaps; ++ k ) {
      if( m_outputMapsMask[k] != 0 && m_outputMaps[k] != 0 ) {
         //        string outputFileName = projectHandle->getProjectName() + "_" + outputMapsNames[i] + string(ageString) + extensionString;
         string outputFileName = projectHandle->getProjectName() + "_" + outputMapsNames[k] + extensionString;
         
         outputFileCrust.open (outputFileName.c_str ());
         if (outputFileCrust.fail ()) {
            cout << "Could not open XYZ output file for map " << outputMapsNames[i] << endl;
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
         cout << "Map " << outputMapsNames[k] << " is saved into " << outputFileName <<  endl;
      }
   }
   return true; 
}
//------------------------------------------------------------//
bool InterfaceOutput::saveExcelSurfaceOutputMaps( Interface::ProjectHandle * projectHandle ) {

   // cout << "My rank is " << CrustalThicknessInterface::GetRank() << endl;

   // char ageString[64];
   // sprintf(ageString, "_%lf", 0);
   
   const string extensionString = ".SUR";
   ofstream outputFileCrust;
   
   const Interface::Grid * grid = projectHandle->getActivityOutputGrid ();
   
   unsigned int i, j, k;
   double posI, posJ;
   unsigned lastI = grid->numI();
   unsigned lastJ = grid->numJ();

   for( k = 0; k < numberOfOutputMaps; ++ k ) {
      if( m_outputMapsMask[k] != 0 && m_outputMaps[k] != 0 ) {
         //        string outputFileName = projectHandle->getProjectName() + "_" + outputMapsNames[i] + string(ageString) + extensionString;
         string outputFileName = projectHandle->getProjectName() + "_" + outputMapsNames[k] + extensionString;
         
         outputFileCrust.open (outputFileName.c_str ());
         if (outputFileCrust.fail ()) {
            cout << "Could not open XYZ output file for map " << outputMapsNames[i] << endl;
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
         cout << "Map " << outputMapsNames[k] << " is saved into " << outputFileName <<  endl;
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
void InterfaceOutput::setAllMapsToOutput( bool flag ) {
 
   for( int i = 0; i < numberOfOutputMaps; ++ i ) {
      m_outputMapsMask[i] = flag;
   }

}
//------------------------------------------------------------//
bool InterfaceOutput::allocateOutputMaps(Interface::ProjectHandle * projectHandle) {

  const Interface::Grid * grid = projectHandle->getActivityOutputGrid ();
  bool status = true;

  for( int i = 0; i < numberOfOutputMaps; ++ i ) {
     if( m_outputMapsMask[i] ) {
        m_outputMaps[i] = projectHandle->getFactory()->produceGridMap(0, 0, grid, Interface::DefaultUndefinedMapValue, 1);    
        if( m_outputMaps[i] == 0 ) {
           status = false;
           break;
        } 
     }
  }
  return status;
}

//------------------------------------------------------------//
bool InterfaceOutput::createSnapShotOutputMaps(ProjectHandle * pHandle, const Snapshot * theSnapshot, const Interface::Surface *theSurface ) {
   
   int i;
   bool status = true;
   if( theSnapshot->getTime() == 0.0 ) {
      // Output these properties for present-day only
      for( i = 0; i < WLSMap; ++ i ) {
         if( m_outputMapsMask[i] ) {
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
         if( m_outputMapsMask[i] ) {
            outputMaps id = ( outputMaps ) i;
            if( id != isostaticBathymetry && id != incTectonicSubsidence ) {
               m_outputMaps[i] = createSnapshotResultPropertyValueMap(pHandle, outputMapsNames[i], theSnapshot);
            } else {
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
void InterfaceOutput::allocateOutputMap( ProjectHandle * pHandle, outputMaps aMapIndex ) 
{
   m_outputMapsMask[aMapIndex] = true;
   m_outputMaps[aMapIndex] = pHandle->getFactory ()->produceGridMap (0, 0, pHandle->getActivityOutputGrid (), DefaultUndefinedMapValue, 1);;
} 

//------------------------------------------------------------//
void InterfaceOutput::deleteOutputMap( outputMaps aMapIndex ) 
{
   if( m_outputMaps[aMapIndex] != 0 ) {
      delete m_outputMaps[aMapIndex];
      m_outputMaps[aMapIndex] = 0;
   }
   m_outputMapsMask[aMapIndex] = false;
} 

//------------------------------------------------------------//
bool InterfaceOutput::updateIsoBathymetryMaps( ProjectHandle * pHandle, std::vector<double> & snapshots ) {
   
   if( m_outputMapsMask[isostaticBathymetry] ) {
      const Interface::Property * isoBathymetry = pHandle->findProperty("IsostaticBathymetry");
      
      if( isoBathymetry != 0 ) {
         const GridMap * presentDayWLS = getMap( WLSMap );
                  
         if( presentDayWLS ) {
            const Interface::Formation * formationCrust = dynamic_cast<const Interface::Formation *>( pHandle->getCrustFormation ());
            const Interface::Surface   * topOfCrust = formationCrust->getTopSurface();

            unsigned firstI = presentDayWLS->firstI();
            unsigned firstJ = presentDayWLS->firstJ();
            unsigned lastI  = presentDayWLS->lastI();
            unsigned lastJ  = presentDayWLS->lastJ();
            
            presentDayWLS->retrieveData();
            
            unsigned int i, j, k;
            for( k = 0; k < snapshots.size(); ++ k ) {
               const Snapshot * theSnapshot = (const Snapshot *) pHandle->findSnapshot ( snapshots[k] );
               
               Interface::PropertyValueList* propVals;
               Interface::PropertyValueList::const_iterator propValIter;
               
               propVals = pHandle->getPropertyValues ( Interface::SURFACE, isoBathymetry, theSnapshot, 0, 0, topOfCrust, Interface::MAP );
               
               if (propVals->size() == 1) {
                  const PropertyValue *thePropertyValue = (*propVals)[ 0 ];  
                  GridMap *theMap = ( thePropertyValue ? thePropertyValue->getGridMap() : 0 );
                  
                  if( theMap ) {
                     theMap->retrieveData();
                     for ( i = firstI; i <= lastI; ++ i ) {
                        for ( j = firstJ; j <= lastJ; ++ j ) { 
                           if( presentDayWLS->getValue( i, j ) != Interface::DefaultUndefinedMapValue ) {
                              const double value = theMap->getValue( i, j );
                              theMap->setValue( i, j, presentDayWLS->getValue( i, j ) - value );
                           }
                        }
                     }  
                     theMap->restoreData();
                  }
               }
               
               delete propVals;
            }
            presentDayWLS->retrieveData();
         } else {
            return false;
         }  
      }
   } else {
      return false; 
   }
   return true;
}
//------------------------------------------------------------//
GridMap * InterfaceOutput::getCurrentPropertyMap( ProjectHandle * pHandle, double aSnapshotId, const std::string& propertyName ) {

   const Interface::Property * aMap = pHandle->findProperty( propertyName );
    
   if( aMap == 0 ) { return false; }
   const Snapshot * theSnapshot = (const Snapshot *) pHandle->findSnapshot ( aSnapshotId );
   
   Interface::FormationList * myFormations = pHandle->getFormations (theSnapshot, true);
   const Interface::Formation * formationWB = (*myFormations)[0]; // find Water bottom
   
   if (!formationWB) {
      string s = "Could not find topformation.";
      return false; //throw s;
   } 
   const Interface::Surface * topOfSedimentSurface = formationWB->getTopSurface();
   
   Interface::PropertyValueList * propertyValues = pHandle->getPropertyValues (SURFACE, aMap, theSnapshot, 0, 0, topOfSedimentSurface, MAP);
   
   if (propertyValues->size() != 1) {
      stringstream ss;
      ss << "Could not find property IncTectonicSubsidence for surface " << topOfSedimentSurface->getName() <<  
         " at age " << theSnapshot->getTime() << "." << endl;
      
      return false; //throw ss.str();
   }
   const PropertyValue * thePropertyValue = (*propertyValues)[ 0 ];  
   delete propertyValues;
   
   return ( thePropertyValue ? thePropertyValue->getGridMap() : 0 ); //  incTS
}

//------------------------------------------------------------//
void InterfaceOutput::debugOutput( Interface::ProjectHandle * pHandle, bool isDebug, int outputOptions, const Snapshot * theSnapshot ) {

   if( isDebug ) {
      if( outputOptions & XYZ ) {
         if( pHandle->getSize() > 1 ) {
            cout << "Can not save maps in XYZ format in parallel. Run with nprocs = 1." << endl;
         } else {
            saveXYZOutputMaps( pHandle );
         }
      }
      if( outputOptions & SUR ) {
         if( pHandle->getSize() > 1 ) {
            cout << "Can not save maps in XYZ format in parallel. Run with nprocs = 1." << endl;
         } else {
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
