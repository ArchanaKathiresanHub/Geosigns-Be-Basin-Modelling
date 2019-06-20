//
// Copyright (C) 2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "mpi.h"
#include "overburden.h"
#include "ProjectHandle.h"
#include "PropertyValue.h"
#include "GridMap.h"
#include "Formation.h"

#include <limits>

using namespace DataAccess;

using Interface::ProjectHandle;

namespace migration
{
   namespace overburden
   {

      template <typename PRED>
      vector<const MigrationFormation*> getDownwardOverburdenFormationsIf (const
         MigrationFormation* formation, PRED pred)
      {
         const ProjectHandle* projectHandle = formation->getProjectHandle ();
         Interface::FormationList* formations = projectHandle->getFormations ();

         vector<const MigrationFormation*> overburdenFormationsFromTop;
         for (Interface::FormationList::const_iterator it = formations->begin ();
            it != formations->end (); ++it)
         {
            const MigrationFormation * iteratorFormation = dynamic_cast <const MigrationFormation *> (*it);
            if (iteratorFormation == formation) break;
            else
            {
               if (pred (iteratorFormation))
                  overburdenFormationsFromTop.push_back (iteratorFormation);
            }
         }
         delete formations;

         return overburdenFormationsFromTop;
      }

      template <typename PRED>
      vector<const MigrationFormation*> getUpwardOverburdenFormationsIf (const
         MigrationFormation* formation, PRED pred)
      {
         vector<const MigrationFormation*> overburdenFormationsFromTop = getDownwardOverburdenFormationsIf (
            formation, pred);

         vector<const MigrationFormation*> overburdenFormationsFromBase;
         overburdenFormationsFromBase.reserve (overburdenFormationsFromTop.size ());
         for (vector<const MigrationFormation*>::reverse_iterator it = overburdenFormationsFromTop.rbegin ();
            it != overburdenFormationsFromTop.rend (); ++it)
            overburdenFormationsFromBase.push_back (*it);

         return overburdenFormationsFromBase;
      }

      struct AlwaysTrue
      {
         bool operator()(const MigrationFormation* formation) { return true; }
      };

      OverburdenFormations getOverburdenFormations (const MigrationFormation* formation,
         bool upward)
      {
         if (upward)
            return OverburdenFormations (getUpwardOverburdenFormationsIf (formation, AlwaysTrue ()), true);
         else
            return OverburdenFormations (getDownwardOverburdenFormationsIf (formation, AlwaysTrue ()), false);
      }

      template <typename PRED>
      vector<const MigrationFormation*> getOverburdenFormationsIf (const MigrationFormation* formation, PRED pred,
         bool upward)
      {
         if (upward)
            return getUpwardOverburdenFormationsIf (formation, pred);
         else
            return getDownwardOverburdenFormationsIf (formation, pred);
      }

   }
} // namespace migration::overburden

// Some templates from overburden.[Ch] must be instantiated:

// #include "SelectIfThicknessIsLargerThanZero.h"

// namespace migration { namespace overburden {

// using namespace migration;

// template
// vector<const MigrationFormation*> getOverburdenFormationsIf<SelectIfThicknessIsLargerThanZero>(
//    const MigrationFormation*,SelectIfThicknessIsLargerThanZero, bool);

// } }
