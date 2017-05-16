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
#include "overburden_MPI.h"
#include "overburden.h"
// #include "SelectIfThicknessIsLargerThanZero.h"
#include "Tuple2.h"

#include "Interface/ProjectHandle.h"
#include "Interface/PropertyValue.h"
#include "Interface/GridMap.h"
#include "Interface/Snapshot.h"
#include "Interface/Surface.h"
#include "Formation.h"

#include <limits>

using namespace migration;

using namespace DataAccess;
using Interface::FormationList;
using Interface::PropertyValueList;
using Interface::ProjectHandle;
using Interface::Formation;

namespace migration
{
   namespace overburden_MPI
   {

      vector<FormationSurfaceGridMaps> getFormationSurfaceGridMaps (const vector<const Formation*>&
         formations, const Property* prop, const Snapshot* snapshot)
      {
         vector<FormationSurfaceGridMaps> results;
         results.reserve (formations.size ());
         if (formations.size () == 0)
            return results;

         vector<const Formation*>::const_iterator f = formations.begin ();
         for (; f != formations.end (); ++f)
         {
            if ((*f)->getBottomSurface()->getSnapshot()->getTime() <= snapshot->getTime())
               break;
            FormationSurfaceGridMaps gridMaps = (*f)->getFormationSurfaceGridMaps (prop,
               snapshot);
            results.push_back (gridMaps);
         }
         return results;
      }

      vector<FormationSurfaceGridMaps> getFormationSurfaceGridMaps (const vector<const
         Formation*>& formations, const string& propName, const Snapshot* snapshot)
      {
         vector<FormationSurfaceGridMaps> results;
         if (formations.size () == 0)
            return results;

         const ProjectHandle* projectHandle = formations[0]->getProjectHandle ();
         const Property* prop = projectHandle->findProperty (propName);

         return getFormationSurfaceGridMaps (formations, prop, snapshot);
      }

      vector<SurfaceGridMapFormations> getDownwardAdjacentSurfaceGridMapFormations (
         const vector<const Formation*>& formations, const Property* prop, const Snapshot* snapshot)
      {
         vector<SurfaceGridMapFormations> results;
         if (formations.size () == 0)
            return results;

         results.reserve (formations.size () + 1);

         vector<const Formation*>::const_iterator fit = formations.begin ();
         const Formation* f_prev = NULL;
         while (fit != formations.end ())
         {
            if ( (*fit)->getBottomSurface()->getSnapshot()->getTime() <= snapshot->getTime() )
               break;
            SurfaceGridMap top = (*fit)->getTopSurfaceGridMap (prop, snapshot);
            results.push_back (SurfaceGridMapFormations (top, f_prev, *fit));

            f_prev = *fit;
            ++fit;
         }

         SurfaceGridMap base = f_prev->getBaseSurfaceGridMap (prop, snapshot);
         results.push_back (SurfaceGridMapFormations (base, f_prev, NULL));
         return results;
      }

      vector<SurfaceGridMapFormations> getUpwardAdjacentSurfaceGridMapFormations (const vector<
         const Formation*>& formations, const Property* prop,
         const Snapshot* snapshot)
      {
         vector<SurfaceGridMapFormations> results;
         if (formations.size () == 0)
            return results;

         results.reserve (formations.size () + 1);

         vector<const Formation*>::const_iterator fit = formations.begin ();
         const Formation* f_prev = NULL;
         while (fit != formations.end ())
         {
            if ( (*fit)->getBottomSurface()->getSnapshot()->getTime() <= snapshot->getTime() )
               break;
            SurfaceGridMap base = (*fit)->getBaseSurfaceGridMap (prop, snapshot);
            results.push_back (SurfaceGridMapFormations (base, *fit, f_prev));

            f_prev = *fit;
            ++fit;
         }

         SurfaceGridMap top = f_prev->getTopSurfaceGridMap (prop, snapshot);
         results.push_back (SurfaceGridMapFormations (top, NULL, f_prev));
         return results;
      }

      vector<SurfaceGridMapFormations> getAdjacentSurfaceGridMapFormations (const overburden::
         OverburdenFormations& overburden, const Property* prop,
         const Snapshot* snapshot)
      {
         if (overburden.upward ())
            return getUpwardAdjacentSurfaceGridMapFormations (overburden.formations (), prop, snapshot);
         else
            return getDownwardAdjacentSurfaceGridMapFormations (overburden.formations (), prop, snapshot);
      }

      vector<SurfaceGridMapFormations> getAdjacentSurfaceGridMapFormations (const overburden::
         OverburdenFormations& overburden, const string& propName, const Snapshot* snapshot)
      {
         vector<SurfaceGridMapFormations> results;
         if (overburden.formations ().size () == 0)
            return results;

         const ProjectHandle* projectHandle = overburden.formations ()[0]->getProjectHandle ();
         const Property* prop = projectHandle->findProperty (propName);

         return getAdjacentSurfaceGridMapFormations (overburden, prop, snapshot);
      }

      template <typename PRED>
      vector<const Formation*> getOverburdenFormationsIf (vector<FormationSurfaceGridMaps>::const_iterator begin,
         vector<FormationSurfaceGridMaps>::const_iterator end, PRED pred)
      {
         assert (begin != end);

         vector<const Formation*> overburdenFormations;
         for (vector<FormationSurfaceGridMaps>::const_iterator it = begin; it != end; ++it)
         {
            if (pred (*it))
               overburdenFormations.push_back ((*it).formation ());
         }
         return overburdenFormations;
      }

      struct SelectIfGridPairThicknessIsLargerThanZero
      {

         const Snapshot* m_snapshot;
         unsigned int m_i, m_j;
         bool m_allValid;

         SelectIfGridPairThicknessIsLargerThanZero (const Snapshot* snapshot, unsigned int i,
            unsigned int j) :
            m_snapshot (snapshot), m_i (i), m_j (j),
            m_allValid (true)
         {
         }

         bool operator()(const FormationSurfaceGridMaps& pair)
         {
            if (!pair.base ().valid () || !pair.top ().valid ())
            {
               m_allValid = false;
               return false;
            }

            double thickness = pair.base ()[functions::Tuple2<unsigned int>(m_i, m_j)] - pair.top ()[functions::Tuple2<unsigned int>(m_i, m_j)];
            assert (thickness >= 0.0);

            // Formations with thickness equal 0.0 don't count here as real formations.
            // Those are ignored:
            return thickness > 0.0;
         }

         bool allValid () const { return m_allValid; }
      };

      bool getRelevantOverburdenFormations (vector<FormationSurfaceGridMaps>::const_iterator begin,
         vector<FormationSurfaceGridMaps>::const_iterator end, const Snapshot* snapshot, unsigned int i,
         unsigned int j, vector<const Formation*>& overburdenFormations)
      {
         SelectIfGridPairThicknessIsLargerThanZero pred (snapshot, i, j);

         overburdenFormations = getOverburdenFormationsIf (begin, end, pred);
         return pred.allValid ();
      }

      enum SelectContinueOrStop
      {
         SELECT_CONTINUE, SELECT_STOP, CONTINUE
      };

      template <typename PRED>
      struct ContinueIfOverburdenDepthAndNumberOfFormationsLeft
      {

         const Snapshot* m_snapshot;
         unsigned int m_i, m_j;
         double m_maxOverburdenDepthLeft;
         int m_maxFormationsLeft;
         PRED m_pred;

         bool m_someValid;
         bool m_allValid;

         ContinueIfOverburdenDepthAndNumberOfFormationsLeft (const Snapshot* snapshot, unsigned int i, unsigned int j,
            const double& maxOverburdenDepth, int maxFormations, PRED pred) :
            m_snapshot (snapshot),
            m_i (i), m_j (j),
            m_maxOverburdenDepthLeft (maxOverburdenDepth),
            m_maxFormationsLeft (maxFormations),
            m_pred (pred),
            m_allValid (true),
            m_someValid (false)
         {
         }

         SelectContinueOrStop operator()(const FormationSurfaceGridMaps& gridMaps)
         {
            if (!gridMaps.base ().valid () || !gridMaps.top ().valid ())
            {
               m_allValid = false;
               return CONTINUE;
            }

            double thickness = gridMaps.base ()[functions::Tuple2<unsigned int>(m_i, m_j)];
            thickness -= gridMaps.top ()[functions::Tuple2<unsigned int>(m_i, m_j)];
            m_someValid = true;

            if (m_pred (gridMaps, thickness))
            {
               --m_maxFormationsLeft;
               if (m_maxFormationsLeft == 0)
                  return SELECT_STOP;
               m_maxOverburdenDepthLeft -= thickness;
               if (m_maxOverburdenDepthLeft <= 0.0)
                  return SELECT_STOP;
               return SELECT_CONTINUE;
            }
            else
               return CONTINUE;
         }

         bool allValid () const { return m_allValid; }
         bool someValid () const { return m_someValid; }
      };

      template <typename PRED>
      bool getOverburdenFormationsIf (vector<FormationSurfaceGridMaps>::const_iterator begin,
         vector<FormationSurfaceGridMaps>::const_iterator end, const Snapshot* snapshot, unsigned int i,
         unsigned int j, const double& maxOverburdenDepth, int maxFormations, bool someValid, PRED pred,
         vector<const Formation*>& overburdenFormations)
      {
         ContinueIfOverburdenDepthAndNumberOfFormationsLeft<PRED> select (snapshot, i, j, maxOverburdenDepth,
            maxFormations, pred);

         for (vector<FormationSurfaceGridMaps>::const_iterator it = begin; it != end; ++it)
         {
            SelectContinueOrStop result = select.operator()(*it);
            if (result == SELECT_CONTINUE || result == SELECT_STOP)
               overburdenFormations.push_back ((*it).formation ());
            if (result == SELECT_STOP)
               break;
         }
         return someValid ? select.someValid () : select.allValid ();
      }

      struct AlwaysTrue
      {
         bool operator()(const FormationSurfaceGridMaps& gridMaps, const double& thickness) { return true; }
      };

      bool getOverburdenFormations (vector<FormationSurfaceGridMaps>::const_iterator begin,
         vector<FormationSurfaceGridMaps>::const_iterator end, const Snapshot* snapshot, unsigned int i,
         unsigned int j, const double& maxOverburdenDepth, int maxFormations, bool someValid,
         vector<const Formation*>& overburdenFormations)
      {
         return getOverburdenFormationsIf (begin, end, snapshot, i, j, maxOverburdenDepth, maxFormations,
            someValid, AlwaysTrue (), overburdenFormations);
      }

      struct TrueIfThicknessBiggerThanZero
      {
         bool operator()(const FormationSurfaceGridMaps& gridMaps, const double& thickness) { return thickness > 0.0; }
      };

      bool getRelevantOverburdenFormations (vector<FormationSurfaceGridMaps>::const_iterator begin,
         vector<FormationSurfaceGridMaps>::const_iterator end, const Snapshot* snapshot, unsigned int i, unsigned int j,
         const double& maxOverburdenDepth, int maxFormations, bool someValid, vector<const Formation*>& overburdenFormations)
      {
         return getOverburdenFormationsIf (begin, end, snapshot, i, j, maxOverburdenDepth, maxFormations, someValid,
            TrueIfThicknessBiggerThanZero (), overburdenFormations);
      }

   }
} // namespace migration::overburden_MPI
