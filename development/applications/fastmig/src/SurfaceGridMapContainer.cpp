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

#include "SurfaceGridMapContainer.h"

namespace migration
{

   SurfaceGridMapContainer::SurfaceGridMapContainer () :
      m_status (CREATED)
   {
   }

   SurfaceGridMapContainer::~SurfaceGridMapContainer ()
   {
      clear ();
   }

   bool SurfaceGridMapContainer::allValid ()
   {
      for (map<ConstantPropertyEnum, constant_properties>::iterator it = m_constants.begin ();
         it != m_constants.end (); ++it)
      {
         for (constant_properties::iterator gm = (*it).second.begin ();
            gm != (*it).second.end (); ++gm)
            if (!(*gm).second.valid ())
               return false;
      }

      { for (map<ContinuousPropertyEnum, continuous_properties>::iterator it = m_continuous.begin ();
      it != m_continuous.end (); ++it)
      {
         for (continuous_properties::iterator gm = (*it).second.begin ();
            gm != (*it).second.end (); ++gm)
            if (!(*gm).surface ().valid ())
               return false;
      } }

      { for (map<DiscontinuousPropertyEnum, discontinuous_properties>::iterator it = m_discontinuous.begin ();
      it != m_discontinuous.end (); ++it)
      {
         for (discontinuous_properties::iterator gm = (*it).second.begin ();
            gm != (*it).second.end (); ++gm)
         {
            if (!(*gm).top ().valid ())
               return false;
            if (!(*gm).base ().valid ())
               return false;
         }
      } }

      return true;
   }

   void SurfaceGridMapContainer::retrieveData ()
   {
      assert (m_status == CREATED || m_status == RESTORED);

      for (map<ConstantPropertyEnum, constant_properties>::iterator it = m_constants.begin ();
         it != m_constants.end (); ++it)
      {
         for (constant_properties::iterator gm = (*it).second.begin ();
            gm != (*it).second.end (); ++gm)
            (*gm).second.retrieveData ();
      }

      { for (map<ContinuousPropertyEnum, continuous_properties>::iterator it = m_continuous.begin ();
      it != m_continuous.end (); ++it)
      {
         for (continuous_properties::iterator gm = (*it).second.begin ();
            gm != (*it).second.end (); ++gm)
            (*gm).getSurface ().retrieveData ();
      } }

      { for (map<DiscontinuousPropertyEnum, discontinuous_properties>::iterator it = m_discontinuous.begin ();
      it != m_discontinuous.end (); ++it)
      {
         for (discontinuous_properties::iterator gm = (*it).second.begin ();
            gm != (*it).second.end (); ++gm)
         {
            (*gm).getTop ().retrieveData ();
            (*gm).getBase ().retrieveData ();
         }
      } }

      m_status = RETRIEVED;
   }

   void SurfaceGridMapContainer::restoreData ()
   {
      assert (m_status == RETRIEVED);

      for (map<ConstantPropertyEnum, constant_properties>::iterator it = m_constants.begin ();
         it != m_constants.end (); ++it)
      {
         for (constant_properties::iterator gm = (*it).second.begin ();
            gm != (*it).second.end (); ++gm)
            (*gm).second.restoreData ();
      }

      { for (map<ContinuousPropertyEnum, continuous_properties>::iterator it = m_continuous.begin ();
      it != m_continuous.end (); ++it)
      {
         for (continuous_properties::iterator gm = (*it).second.begin ();
            gm != (*it).second.end (); ++gm)
            (*gm).getSurface ().restoreData ();
      } }

      { for (map<DiscontinuousPropertyEnum, discontinuous_properties>::iterator it = m_discontinuous.begin ();
      it != m_discontinuous.end (); ++it)
      {
         for (discontinuous_properties::iterator gm = (*it).second.begin ();
            gm != (*it).second.end (); ++gm)
         {
            (*gm).getTop ().restoreData ();
            (*gm).getBase ().restoreData ();
         }
      } }

      m_status = RESTORED;
   }

   void SurfaceGridMapContainer::clear ()
   {
      if (m_status == RETRIEVED)
         restoreData ();

      for (map<ConstantPropertyEnum, constant_properties>::iterator it = m_constants.begin ();
         it != m_constants.end (); ++it)
      {
         for (constant_properties::iterator gm = (*it).second.begin ();
            gm != (*it).second.end (); ++gm)
            (*gm).second.release ();
      }

      m_constants.clear ();
      m_continuous.clear ();
      m_discontinuous.clear ();

      m_status = CREATED;
   }


   void SurfaceGridMapContainer::setConstants (ConstantPropertyEnum prop, const constant_properties& gridMaps)
   {
      assert (m_status == CREATED);
      m_constants[prop] = gridMaps;
   }

   void SurfaceGridMapContainer::setContinuous (ContinuousPropertyEnum prop,
      const continuous_properties& gridMaps)
   {
      assert (m_status == CREATED);
      m_continuous[prop] = gridMaps;
   }

   void SurfaceGridMapContainer::setDiscontinuous (DiscontinuousPropertyEnum prop,
      const discontinuous_properties& gridMaps)
   {
      assert (m_status == CREATED);
      m_discontinuous[prop] = gridMaps;
   }

} // namespace migration

