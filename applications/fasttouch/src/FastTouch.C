#include <unistd.h>

#include <iostream>
#include <fstream>
using namespace std;

#include "petscvec.h"
#include "petscda.h"

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


FastTouch::FastTouch (database::Database * database, const string & name, const string & accessMode)
      : ProjectHandle (database, name, accessMode)
{
   m_masterTouch = new MasterTouch (this);
}

FastTouch::~FastTouch (void)
{
   delete m_masterTouch;
   deleteTouchstoneMaps ();
}

FastTouch * FastTouch::CreateFrom (const string & inputFileName)
{
   FastTouch * fastTouch = (FastTouch *) OpenCauldronProject (inputFileName, "rw");
   if (fastTouch) fastTouch->loadTouchstoneMaps ();
   return fastTouch;
}

bool FastTouch::saveTo (const string & outputFileName)
{
   return saveToFile (outputFileName);
}

bool FastTouch::removeResqPropertyValues (void)
{
   const Property * resqProperty = findProperty ("Resq: ");
   if (!resqProperty) return false;

   deletePropertyValues (Interface::SURFACE | Interface::FORMATION | Interface::FORMATIONSURFACE,
	 resqProperty, 0, 0, 0, 0, Interface::MAP); 

   return true;
}

bool FastTouch::compute (void)
{
   string activityName = "FastTouch";

   bool started = startActivity (activityName, getLowResolutionOutputGrid ());
   if (!started) return false;

   TouchstoneMapList * touchstoneMaps = getTouchstoneMaps ();
   TouchstoneMapList::iterator mapIter;

   // cerr << "number of touchstone maps = " << touchstoneMaps->size () << endl;

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

   m_masterTouch->run ();

   delete touchstoneMaps;

   finishActivity ();

   return true;
}

bool FastTouch::addToComputationList (const TouchstoneMap * touchstoneMap)
{
   string str;
   touchstoneMap->asString (str);
   // cerr << "Adding " << str << "to computation list" << endl;

   m_masterTouch->addOutputFormat (touchstoneMap->getTCFName (), touchstoneMap->getSurface (), touchstoneMap->getFormation (),
	 touchstoneMap->getCategory (), touchstoneMap->getFormat (), touchstoneMap->getPercentage ());
   return true;
}

