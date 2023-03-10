//
// Copyright (C) 2016-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef FASTMIG__MIGRATION_PROPERTY_MANAGER_H
#define FASTMIG__MIGRATION_PROPERTY_MANAGER_H


#include "DerivedPropertyManager.h"

namespace DerivedProperties
{
   class DerivedPropertyManager;
}

namespace migration
{

   /// \brief Handles derived properties and their calculation for fastmig.
   class MigrationPropertyManager : public DerivedProperties::DerivedPropertyManager
   {

   public:

      /// \brief Constructor.
      MigrationPropertyManager (GeoPhysics::ProjectHandle& projectHandle);

      /// \brief Destructor.
      ~MigrationPropertyManager ();

      DataAccess::Interface::GridMap * produceDerivedGridMap (AbstractDerivedProperties::FormationPropertyPtr aProperty);
      DataAccess::Interface::GridMap * produceDerivedGridMap (AbstractDerivedProperties::FormationSurfacePropertyPtr aProperty);
      DataAccess::Interface::GridMap * produceDerivedGridMap (AbstractDerivedProperties::SurfacePropertyPtr aProperty);

   private:
      std::vector <DataAccess::Interface::GridMap *> m_derivedMaps;

   };

}


#endif // FASTMIG__MIGRATION_PROPERTY_MANAGER_H
