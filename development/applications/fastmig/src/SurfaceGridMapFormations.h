//
// Copyright (C) 2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _MIGRATION_SURFACEGRIDMAPFORMATIONS_H_
#define _MIGRATION_SURFACEGRIDMAPFORMATIONS_H_

#include "SurfaceGridMap.h"

#include "Interface/Formation.h"

using Interface::Formation;
using Interface::GridMap;

namespace migration
{

   class SurfaceGridMapFormations
   {
   private:

      SurfaceGridMap m_gridMap;
      const Formation* m_base;
      const Formation* m_top;

   public:

      SurfaceGridMapFormations (const SurfaceGridMap& gridMap, const Formation* top,
         const Formation* base) :
         m_gridMap (gridMap), m_base (base), m_top (top)
      {
      }

      ~SurfaceGridMapFormations ()
      {
      }

      const Formation* top () const { return m_top; }
      const Formation* base () const { return m_base; }

      bool valid () const { return m_gridMap.valid (); }
      double operator[](const Tuple2<unsigned int>& ij) const { return m_gridMap[ij]; }

      SurfaceGridMap& getSurface () { return m_gridMap; }
      const SurfaceGridMap& surface () const { return m_gridMap; }
   };

} // namespace migration

#endif
