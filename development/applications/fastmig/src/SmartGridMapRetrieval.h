//
// Copyright (C) 2010-2015 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _MIGRATION_SMARTGRIDMAPRETRIEVAL_H_
#define _MIGRATION_SMARTGRIDMAPRETRIEVAL_H_

#include "Interface/GridMap.h"

namespace migration
{
   // class for gridmap data retrieval at creation and restoral at deletion
   class SmartGridMapRetrieval
   {
   public:
      SmartGridMapRetrieval (const GridMap * gridMap, bool save) : m_gridMap (gridMap), m_save (save)
      {
         m_gridMap->retrieveData ();
      }

         ~SmartGridMapRetrieval (void)
         {
            m_gridMap->restoreData (m_save);
         }

   private:
         const GridMap * m_gridMap;
         bool m_save;
   };

}

#endif
