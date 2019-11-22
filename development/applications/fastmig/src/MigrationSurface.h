//
// Copyright (C) 2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _MIGRATION_SURFACE_H_
#define _MIGRATION_SURFACE_H_

#include <iostream>

#include <string>
#include <map>
#include <vector>

using namespace std;

namespace database
{
   class Record;
   class Table;
}

#include "Surface.h"

using namespace DataAccess;

#include "RequestDefs.h"

/// Surface Class
namespace migration
{
   class Migrator;

   /// This class implements the Surface-specific migration functionality.
   /// It is constructed on top of the DataAccess::formation class.
   class MigrationSurface : public Interface::Surface
   {
   public:
      /// This constructor is called by the object factory
      MigrationSurface (Interface::ProjectHandle& projectHandle, Migrator * const migrator, database::Record * record);

      /// Destructor
      virtual ~MigrationSurface (void);

      const MigrationSurface * getTopSurface () const;
      const MigrationSurface * getBottomSurface () const;

   private:
      Migrator * const m_migrator;
   };
}

#endif // _MIGRATION_SURFACE_H_

