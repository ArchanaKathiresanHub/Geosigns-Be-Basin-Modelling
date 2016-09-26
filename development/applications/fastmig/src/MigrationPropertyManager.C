//
// Copyright (C) 2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "MigrationPropertyManager.h"

#include "PorosityFormationCalculator.h"
#include "PermeabilityFormationCalculator.h"
#include "PermeabilityFormationSurfaceCalculator.h"
#include "VesFormationCalculator.h"
#include "VesSurfaceCalculator.h"
#include "HydrostaticPressureSurfaceCalculator.h"
#include "HydrostaticPressureFormationCalculator.h"
#include "LithostaticPressureSurfaceCalculator.h"
#include "LithostaticPressureFormationCalculator.h"
#include "FormationPropertyAtSurface.h"

#include "ObjectFactory.h"
#include "Interface/Snapshot.h"

namespace migration
{

   MigrationPropertyManager::MigrationPropertyManager (GeoPhysics::ProjectHandle* projectHandle) :
      DerivedProperties::DerivedPropertyManager (projectHandle)
   {
   }

   MigrationPropertyManager::~MigrationPropertyManager ()
   {

      for (int i = 0; i < m_derivedMaps.size (); ++i)
      {
         delete m_derivedMaps[i];
      }
      m_derivedMaps.clear ();
   }

   DataAccess::Interface::GridMap *  MigrationPropertyManager::produceDerivedGridMap (DerivedProperties::FormationPropertyPtr aProperty)
   {

      DataAccess::Interface::GridMap * theMap = getProjectHandle ()->getFactory ()->produceGridMap (0, 0, getProjectHandle ()->getActivityOutputGrid (),
         aProperty->lengthK ());
      theMap->retrieveData ();

      for (unsigned int i = theMap->firstI (); i <= theMap->lastI (); ++i)
      {
         for (unsigned int j = theMap->firstJ (); j <= theMap->lastJ (); ++j)
         {
            for (unsigned int k = 0; k < theMap->getDepth (); ++k)
            {
               theMap->setValue (i, j, k, aProperty->get (i, j, k));
            }
         }
      }

      theMap->restoreData (true);

      m_derivedMaps.push_back (theMap);

      return theMap;
   }

   DataAccess::Interface::GridMap *  MigrationPropertyManager::produceDerivedGridMap (DerivedProperties::FormationSurfacePropertyPtr aProperty)
   {

      DataAccess::Interface::GridMap * theMap = getProjectHandle ()->getFactory ()->produceGridMap (0, 0, getProjectHandle ()->getActivityOutputGrid (),
         aProperty->getUndefinedValue (), 1);
      theMap->retrieveData ();

      for (unsigned int i = theMap->firstI (); i <= theMap->lastI (); ++i)
      {
         for (unsigned int j = theMap->firstJ (); j <= theMap->lastJ (); ++j)
         {
            theMap->setValue (i, j, aProperty->get (i, j));
         }
      }

      theMap->restoreData (true);

      m_derivedMaps.push_back (theMap);

      return theMap;
   }


   DataAccess::Interface::GridMap *  MigrationPropertyManager::produceDerivedGridMap (DerivedProperties::SurfacePropertyPtr aProperty)
   {

      DataAccess::Interface::GridMap * theMap = getProjectHandle ()->getFactory ()->produceGridMap (0, 0, getProjectHandle ()->getActivityOutputGrid (),
         aProperty->getUndefinedValue (), 1);
      theMap->retrieveData ();

      for (unsigned int i = theMap->firstI (); i <= theMap->lastI (); ++i)
      {
         for (unsigned int j = theMap->firstJ (); j <= theMap->lastJ (); ++j)
         {
            theMap->setValue (i, j, aProperty->get (i, j));
         }
      }


      theMap->restoreData (true);

      m_derivedMaps.push_back (theMap);

      return theMap;
   }


}
