//
// Copyright (C) 2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _MIGRATION_RETRIEVEANDRESTORESURFACEGRIDMAPCONTAINER_H_
#define _MIGRATION_RETRIEVEANDRESTORESURFACEGRIDMAPCONTAINER_H_

#include "SurfaceGridMapContainer.h"

namespace migration
{

   /// This class makes sure the GridMaps of the GridMapContainer 
   /// are automatically restored if the RetrieveAndRestoreLocalContainer 
   /// class goes out of scope.
   class RetrieveAndRestoreSurfaceGridMapContainer
   {
   private:

      SurfaceGridMapContainer& m_container;

   public:

      RetrieveAndRestoreSurfaceGridMapContainer (SurfaceGridMapContainer& container) :
         m_container (container)
      {
         container.retrieveData ();
      }

      ~RetrieveAndRestoreSurfaceGridMapContainer ()
      {
         // We must not only restore the data, we must also remove all GridMaps from 
         // m_container, or at least clear the GridMaps somewhere close after the calculation, 
         // or else DataAccess::Parent is faster in removing the GridMaps than 
         // Reservoir, and a GPF happens in ~Reservoir:
         m_container.clear ();
      }
   };

}

#endif
