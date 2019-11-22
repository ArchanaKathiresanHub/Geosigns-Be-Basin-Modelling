//
// Copyright (C) 2016-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "MigrationPropertyManager.h"

#include "DepthHighResFormationCalculator.h"
#include "MaxVesHighResFormationCalculator.h"
#include "VesHighResFormationCalculator.h"
#include "FormationPropertyAtSurface.h"

#include "ObjectFactory.h"
#include "Snapshot.h"

using namespace AbstractDerivedProperties;

namespace migration
{

   MigrationPropertyManager::MigrationPropertyManager (GeoPhysics::ProjectHandle& projectHandle) :
      DerivedProperties::DerivedPropertyManager (projectHandle)
   {
      loadDerivedFormationPropertyCalculator(FormationPropertyCalculatorPtr(new DerivedProperties::VesHighResFormationCalculator(projectHandle)), false);
      loadDerivedFormationPropertyCalculator(FormationPropertyCalculatorPtr(new DerivedProperties::MaxVesHighResFormationCalculator(projectHandle)), false);
      loadDerivedFormationPropertyCalculator(FormationPropertyCalculatorPtr(new DerivedProperties::DepthHighResFormationCalculator(projectHandle)), false);
   }

   MigrationPropertyManager::~MigrationPropertyManager ()
   {

      for (int i = 0; i < m_derivedMaps.size (); ++i)
      {
         delete m_derivedMaps[i];
      }
      m_derivedMaps.clear ();
   }

   DataAccess::Interface::GridMap *  MigrationPropertyManager::produceDerivedGridMap (FormationPropertyPtr aProperty)
   {

      DataAccess::Interface::GridMap * theMap = getProjectHandle ().getFactory ()->produceGridMap (0, 0, getProjectHandle ().getActivityOutputGrid (),
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

   DataAccess::Interface::GridMap *  MigrationPropertyManager::produceDerivedGridMap (FormationSurfacePropertyPtr aProperty)
   {

      DataAccess::Interface::GridMap * theMap = getProjectHandle ().getFactory ()->produceGridMap (0, 0, getProjectHandle ().getActivityOutputGrid (),
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


   DataAccess::Interface::GridMap *  MigrationPropertyManager::produceDerivedGridMap (SurfacePropertyPtr aProperty)
   {

      DataAccess::Interface::GridMap * theMap = getProjectHandle ().getFactory ()->produceGridMap (0, 0, getProjectHandle ().getActivityOutputGrid (),
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
