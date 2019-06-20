//
// Copyright (C) 2012-2017 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

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

#include "ProjectHandle.h"
#include "ObjectFactory.h"
#include "Formation.h"
#include "Surface.h"
#include "PropertyValue.h"
#include "Grid.h"
#include "GridMap.h"
#include "Snapshot.h"
#include "TouchstoneMap.h"

#include "FilePath.h"

using namespace DataAccess;
using namespace Interface;
double MinimumAll (double myValue);


FastTouch::FastTouch (const std::string & inputFileName, ObjectFactory* factory )
   : m_projectHandle( OpenCauldronProject (inputFileName, "rw", factory))
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
   Interface::PropertyListPtr list = m_projectHandle->getProperties(DataModel::FASTTOUCH_PROPERTY);

   if (list->empty()) {
      return false;
   }

   for (size_t i = 0; i < list->size (); ++i) 
   {
      const Interface::Property* resqProperty = (*list)[ i ];
   
      m_projectHandle->deletePropertyValues (Interface::SURFACE | Interface::FORMATION | Interface::FORMATIONSURFACE,
                                             resqProperty, 0, 0, 0, 0, Interface::MAP);
   }
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
      PetscPrintf ( PETSC_COMM_WORLD, "Basin_Error: MasterTouch::calculate failed more than %d times\n",MAX_RUNS);
      return false;
   }

   delete touchstoneMaps;

   m_projectHandle->finishActivity ();
   m_projectHandle->setSimulationDetails ( "fasttouch", "Default", "" );

   status = true;
   if( !mergeOutputFiles ()) {
      PetscPrintf ( PETSC_COMM_WORLD, "Basin_Error: Unable to merge output files\n");
      status = false;
   }
   return status;
}

bool FastTouch::addToComputationList (const TouchstoneMap * touchstoneMap)
{
   string str;
   touchstoneMap->asString (str);

   m_masterTouch.addOutputFormat(touchstoneMap->getTCFName (), touchstoneMap->getSurface (), touchstoneMap->getFormation (),
                                 touchstoneMap->getCategory (), touchstoneMap->getFormat (), static_cast<int>(touchstoneMap->getPercentage ()),
                                 touchstoneMap->getFaciesGridMap ( ), touchstoneMap->getFaciesNumber(), touchstoneMap->getRunName());
   return true;
}

bool FastTouch::mergeOutputFiles ( ) {

  if( m_projectHandle->getModellingMode () == Interface::MODE1D ) return true;

#ifndef _MSC_VER
  ibs::FilePath localPath  ( m_projectHandle->getProjectPath () );
  localPath <<  m_projectHandle->getOutputDir ();
  const bool status = H5_Parallel_PropertyList ::mergeOutputFiles ( FastTouchActivityName, localPath.path() );

  return status;
#else
   return true;
#endif
}
