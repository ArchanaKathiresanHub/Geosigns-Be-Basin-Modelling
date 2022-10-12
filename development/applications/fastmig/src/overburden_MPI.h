//
// Copyright (C) 2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _MIGRATION_OVERBURDEN_MPI_H_
#define _MIGRATION_OVERBURDEN_MPI_H_

#include "MigrationFormation.h"
#include "overburden.h"
#include "SurfaceGridMapFormations.h"

#include "Snapshot.h"

#include <vector>

using namespace DataAccess;

using Interface::GridMap;
using Interface::Snapshot;
using Interface::Property;

using std::vector;

namespace migration
{
   class MigrationFormation;
   namespace overburden_MPI
   {

      std::vector<FormationSurfaceGridMaps> getFormationSurfaceGridMaps (const std::vector<const MigrationFormation*>& formations,
         const Property* prop, const Snapshot* snapshot);

      std::vector<FormationSurfaceGridMaps> getFormationSurfaceGridMaps (const std::vector<const MigrationFormation*>& formations,
         const std::string& propName, const Snapshot* snapshot);

      std::vector<SurfaceGridMapFormations> getAdjacentSurfaceGridMapFormations (const overburden::
         OverburdenFormations& formations, const Property* prop, const Snapshot* snapshot);

      std::vector<SurfaceGridMapFormations> getAdjacentSurfaceGridMapFormations (const overburden::
         OverburdenFormations& formations, const std::string& prop, const Snapshot* snapshot);

      ///
      ///

      template <typename PRED>
      std::vector<const MigrationFormation*> getOverburdenFormationsIf (vector<FormationSurfaceGridMaps>::const_iterator begin,
         std::vector<FormationSurfaceGridMaps>::const_iterator end, PRED pred);

      bool getRelevantOverburdenFormations (vector<FormationSurfaceGridMaps>::const_iterator begin,
         std::vector<FormationSurfaceGridMaps>::const_iterator end, const Snapshot* snapshot,
         unsigned int i, unsigned int j, std::vector<const MigrationFormation*>& overburdenFormations);

      template <typename PRED>
      bool getOverburdenFormationsIf (vector<FormationSurfaceGridMaps>::const_iterator begin,
         std::vector<FormationSurfaceGridMaps>::const_iterator end, const Snapshot* snapshot, unsigned int i,
         unsigned int j, const double& maxOverburdenDepth, int maxFormations, bool someValid, PRED pred,
         std::vector<const MigrationFormation*>& overburdenFormations);

      bool getOverburdenFormations (vector<FormationSurfaceGridMaps>::const_iterator begin,
         std::vector<FormationSurfaceGridMaps>::const_iterator end, const Snapshot* snapshot, unsigned int i,
         unsigned int j, const double& maxOverburdenDepth, int maxFormations, bool someValid,
         std::vector<const MigrationFormation*>& overburdenFormations);

      bool getRelevantOverburdenFormations (vector<FormationSurfaceGridMaps>::const_iterator begin,
         std::vector<FormationSurfaceGridMaps>::const_iterator end, const Snapshot* snapshot, unsigned int i, unsigned int j,
         const double& maxOverburdenDepth, int maxFormations, bool someValid,
         std::vector<const MigrationFormation*>& overburdenFormations);

   }
} // namespace migration::overburden_MPI

#endif
