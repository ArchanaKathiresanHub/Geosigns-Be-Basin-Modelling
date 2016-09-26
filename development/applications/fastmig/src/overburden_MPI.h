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

#include "Formation.h"
#include "overburden.h"
#include "SurfaceGridMapFormations.h"

#include "Interface/Snapshot.h"

#include <vector>

using namespace DataAccess;

using Interface::GridMap;
using Interface::Snapshot;
using Interface::Property;

using std::vector;

namespace migration
{
   class Formation;
   namespace overburden_MPI
   {

      vector<FormationSurfaceGridMaps> getFormationSurfaceGridMaps (const vector<const Formation*>& formations,
         const Property* prop, const Snapshot* snapshot);

      vector<FormationSurfaceGridMaps> getFormationSurfaceGridMaps (const vector<const Formation*>& formations,
         const string& propName, const Snapshot* snapshot);

      vector<SurfaceGridMapFormations> getAdjacentSurfaceGridMapFormations (const overburden::
         OverburdenFormations& formations, const Property* prop, const Snapshot* snapshot);

      vector<SurfaceGridMapFormations> getAdjacentSurfaceGridMapFormations (const overburden::
         OverburdenFormations& formations, const string& prop, const Snapshot* snapshot);

      ///
      ///

      template <typename PRED>
      vector<const Formation*> getOverburdenFormationsIf (vector<FormationSurfaceGridMaps>::const_iterator begin,
         vector<FormationSurfaceGridMaps>::const_iterator end, PRED pred);

      bool getRelevantOverburdenFormations (vector<FormationSurfaceGridMaps>::const_iterator begin,
         vector<FormationSurfaceGridMaps>::const_iterator end, const Snapshot* snapshot,
         unsigned int i, unsigned int j, vector<const Formation*>& overburdenFormations);

      template <typename PRED>
      bool getOverburdenFormationsIf (vector<FormationSurfaceGridMaps>::const_iterator begin,
         vector<FormationSurfaceGridMaps>::const_iterator end, const Snapshot* snapshot, unsigned int i,
         unsigned int j, const double& maxOverburdenDepth, int maxFormations, bool someValid, PRED pred,
         vector<const Formation*>& overburdenFormations);

      bool getOverburdenFormations (vector<FormationSurfaceGridMaps>::const_iterator begin,
         vector<FormationSurfaceGridMaps>::const_iterator end, const Snapshot* snapshot, unsigned int i,
         unsigned int j, const double& maxOverburdenDepth, int maxFormations, bool someValid,
         vector<const Formation*>& overburdenFormations);

      bool getRelevantOverburdenFormations (vector<FormationSurfaceGridMaps>::const_iterator begin,
         vector<FormationSurfaceGridMaps>::const_iterator end, const Snapshot* snapshot, unsigned int i, unsigned int j,
         const double& maxOverburdenDepth, int maxFormations, bool someValid,
         vector<const Formation*>& overburdenFormations);

   }
} // namespace migration::overburden_MPI

#endif
