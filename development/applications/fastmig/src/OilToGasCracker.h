//
// Copyright (C) 2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _MIGRATION_OILTOGASCRACKER_H_
#define _MIGRATION_OILTOGASCRACKER_H_

#ifdef USEOTGC
#include "migration.h"

namespace Genex6
{
   class Simulator;
}
namespace migration
{
   class Immobiles;
   class Composition;

   class OilToGasCracker
   {
   public:
      OilToGasCracker (bool containSuplhur = false);
      virtual ~OilToGasCracker (void);

      void compute (const Composition & saraIn, Immobiles &immobilesIn,
         double startTime, double endTime,
         double startPressure, double endPressure,
         double startTemperature, double endTemperature,
         Composition & saraOut, Immobiles &immobilesOut);
   private:
      Genex6::Simulator *m_theSimulator;

      bool ComponentsUsedInOTGC[migration::NumComponents];
   };

} // namespace migration
#endif

#endif
