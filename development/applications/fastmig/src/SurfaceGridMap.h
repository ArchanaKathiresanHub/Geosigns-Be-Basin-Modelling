//
// Copyright (C) 2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _MIGRATION_SURFACEGRIDMAP_H_
#define _MIGRATION_SURFACEGRIDMAP_H_

#include "Tuple2.h"

#include "GridMap.h"

using namespace DataAccess;
using Interface::GridMap;

using functions::Tuple2;

namespace migration
{

   class SurfaceGridMap
   {
   private:

      enum GridMapStatus
      {
         CREATED = 0, RETRIEVED, RESTORED
      };

      unsigned int m_index;
      const GridMap* m_gridMap;
      GridMapStatus m_status;

   public:

      SurfaceGridMap (const GridMap* gridMap, unsigned int index);
      ~SurfaceGridMap ();

      void retrieveData ();
      void restoreData ();
      void release ();

      double operator[](const Tuple2<unsigned int>& ij) const;

      bool valid () const { return (m_gridMap != 0); }
   };

} // namespace migration

#endif
