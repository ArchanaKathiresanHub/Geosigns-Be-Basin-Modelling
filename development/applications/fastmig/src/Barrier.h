//
// Copyright (C) 2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _MIGRATION_BARRIER_H_
#define _MIGRATION_BARRIER_H_

#include "Reservoir.h"
#include "migration.h"

namespace DataAccess
{
   namespace Interface
   {
      class Snapshot;
   }
}

namespace migration
{
   /// Class describing a barrier for vertical migrations into its reservoir
   /// Can be on or off

   class Barrier
   {
   public:
      /// Constructor
      Barrier (Reservoir * reservoir);
      virtual ~Barrier (void);

      /// Use permeability and porosity properties of given formation to update the barrier's properties
      void updateBlocking (const Formation * formation,
                           const DataAccess::Interface::Snapshot * snapshot);
      void updateBlockingPermeability (const Formation * formation,
                                       const DataAccess::Interface::Snapshot * snapshot);
      void updateBlockingPorosity (const Formation * formation,
                                   const DataAccess::Interface::Snapshot * snapshot);

      /// return whether the barrier is blocking for (i, j)
      bool isBlocking (unsigned int i, unsigned int j);

   private:
      bool ** m_values;
      Reservoir * m_reservoir;
      double m_blockingPermeability;
      double m_blockingPorosity;
      unsigned int m_firstI;
      unsigned int m_firstJ;
   };
}

#endif // _MIGRATION_BARRIER_H_
