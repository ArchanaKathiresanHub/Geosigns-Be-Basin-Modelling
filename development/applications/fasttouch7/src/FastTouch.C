#ifdef _WIN32
#include <direct.h>
#else
#include <unistd.h>
#endif

#include <iostream>
#include <fstream>
using namespace std;

#include "petscvec.h"
#include "petscdmda.h"

#include "h5_parallel_file_types.h"
#ifndef _MSC_VER
#include "h5merge.h"
#endif

#include "database.h"
#include "cauldronschema.h"
#include "cauldronschemafuncs.h"
using namespace database;

#include "FastTouch.h"
#include "MasterTouch.h"

using namespace fasttouch;
 
#include "Interface/ProjectHandle.h"
#include "Interface/Formation.h"
#include "Interface/Surface.h"
#include "Interface/PropertyValue.h"
#include "Interface/Grid.h"
#include "Interface/GridMap.h"
#include "Interface/Snapshot.h"
#include "Interface/TouchstoneMap.h"

using namespace DataAccess;
using namespace Interface;
double MinimumAll (double myValue);


FastTouch::FastTouch (const std::string & inputFileName )
   : m_projectHandle( OpenCauldronProject (inputFileName, "rw"))
   , m_masterTouch(*m_projectHandle)
{
}

FastTouch::~FastTouch (void)
{
}

bool FastTouch::saveTo (const string & outputFileName)
{
   return m_projectHandle->saveToFile (outputFileName);
}

bool FastTouch::removeResqPropertyValues (void)
{
   const Property * resqProperty = m_projectHandle->findProperty ("Resq: ");
   if (!resqProperty) return false;
    
   m_projectHandle->deletePropertyValues (Interface::SURFACE | Interface::FORMATION | Interface::FORMATIONSURFACE,
                                          resqProperty, 0, 0, 0, 0, Interface::MAP); 
   return true;
}

bool FastTouch::compute (void)
{

   H5_Parallel_PropertyList::setOneFilePerProcessOption ();

   bool started = m_projectHandle->startActivity (FastTouchActivityName, m_projectHandle->getLowResolutionOutputGrid ());
 
   if (!started) return false;
 
   TouchstoneMapList * touchstoneMaps = m_projectHandle->getTouchstoneMaps ();
   TouchstoneMapList::iterator mapIter;
 
   for (mapIter = touchstoneMaps->begin (); mapIter != touchstoneMaps->end (); ++mapIter)
   {
      const TouchstoneMap * touchstoneMap = * mapIter;
      if (touchstoneMap->findPropertyValue () == 0)
      {
         addToComputationList (touchstoneMap);
      }
      else
      {
         string str;
         touchstoneMap->asString (str);
         cerr << "Found PropertyValue for " << str << endl;
      }
   }
 
   bool status = m_masterTouch.run ();

   if ( MinimumAll(status ? 1.0 : -1 ) < 0.0 ) 
   {
      //Print message error
      PetscPrintf ( PETSC_COMM_WORLD, "MeSsAgE ERROR MasterTouch::calculate failed more than %d times\n",MAX_RUNS);
      return false;
   }
 
   delete touchstoneMaps;
 
   m_projectHandle->finishActivity ();
 
   status = true;
   if( !mergeOutputFiles ()) {
      PetscPrintf ( PETSC_COMM_WORLD, "MeSsAgE ERROR Unable to merge output files\n");
      status = false;
   } 
   return status;
}

bool FastTouch::addToComputationList (const TouchstoneMap * touchstoneMap)
{
   string str;
   touchstoneMap->asString (str);
   //cerr << "Adding " << str << "to computation list" << endl;

   m_masterTouch.addOutputFormat( 
                                 touchstoneMap->getTCFName (), touchstoneMap->getSurface (), touchstoneMap->getFormation (),
                                 touchstoneMap->getCategory (), touchstoneMap->getFormat (), static_cast<int>(touchstoneMap->getPercentage ()),
                                 touchstoneMap->getFaciesGridMap ( ), touchstoneMap->getFaciesNumber());
   return true;
}

bool FastTouch::mergeOutputFiles ( ) {

   if( ! H5_Parallel_PropertyList::isOneFilePerProcessEnabled() ){
      return true; 
   }
#ifndef _MSC_VER   
   PetscBool noFileCopy = PETSC_FALSE;
   
   PetscOptionsHasName( PETSC_NULL, "-nocopy", &noFileCopy );
   
   string filePathName 
      = m_projectHandle->getProjectPath () 
      + "/" + m_projectHandle->getOutputDir () + "/" + FastTouchActivityName + "_Results.HDF";
   
   bool status = mergeFiles ( allocateFileHandler( PETSC_COMM_WORLD, filePathName, H5_Parallel_PropertyList::getTempDirName(), ( noFileCopy ? CREATE : REUSE )));

   if( status ) {
      status = H5_Parallel_PropertyList::copyMergedFile( filePathName );
   }
   if( status ) {
      PetscPrintf ( PETSC_COMM_WORLD, "Merged Output Maps\n");
   } else {
      PetscPrintf ( PETSC_COMM_WORLD, "  MeSsAgE ERROR Could not copy the file %s.\n", filePathName.c_str() );   
   }
   return status;
#else
   return true;
#endif
}
