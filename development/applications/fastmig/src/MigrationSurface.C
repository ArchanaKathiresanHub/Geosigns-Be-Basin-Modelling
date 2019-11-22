//
// Copyright (C) 2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "RequestHandling.h"
#include "Formation.h"
#include "MigrationSurface.h"
#include "rankings.h"
#include "migration.h"

#include "ProjectHandle.h"


#include <assert.h>
#include <math.h>

#include <iostream>
#include <vector>
using namespace std;

using namespace CBMGenerics;

using namespace DataAccess;

namespace migration
{

   MigrationSurface::MigrationSurface (Interface::ProjectHandle& projectHandle, Migrator * const migrator, database::Record * record)
      : Interface::Surface (projectHandle, record),
      m_migrator (migrator)
   {
   }

   MigrationSurface::~MigrationSurface (void)
   {
   }

   const MigrationSurface * MigrationSurface::getTopSurface () const
   {
      const Interface::Formation * formation = getTopFormation ();

      return (const MigrationSurface *)(formation ? formation->getTopSurface () : 0);
   }

   const MigrationSurface * MigrationSurface::getBottomSurface () const
   {
      const Interface::Formation * formation = getBottomFormation ();

      return (const MigrationSurface *)(formation ? formation->getBottomSurface () : 0);
   }

}
