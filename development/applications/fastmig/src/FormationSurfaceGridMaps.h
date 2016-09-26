//
// Copyright (C) 2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _MIGRATION_FORMATIONSURFACEGRIDMAPS_H_
#define _MIGRATION_FORMATIONSURFACEGRIDMAPS_H_

#include "migration.h"

#include "SurfaceGridMap.h"

namespace migration
{

   class FormationSurfaceGridMaps
   {
   private:

      SurfaceGridMap m_top;
      SurfaceGridMap m_base;
      const Formation* m_formation;

   public:

      FormationSurfaceGridMaps (const SurfaceGridMap& top, const SurfaceGridMap& base,
         const Formation* formation) :
         m_top (top), m_base (base), m_formation (formation)
      {
      }

      ~FormationSurfaceGridMaps ()
      {
      }

      SurfaceGridMap& getTop () { return m_top; }
      SurfaceGridMap& getBase () { return m_base; }

      const SurfaceGridMap& top () const { return m_top; }
      const SurfaceGridMap& base () const { return m_base; }

      const Formation* formation () const { return m_formation; }
   };

} // namespace migration

#endif
