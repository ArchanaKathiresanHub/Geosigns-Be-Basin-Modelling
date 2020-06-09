//
// Copyright (C) 2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "Column.h"
#include "migration.h"

namespace migration
{
   int DebugOn = 0;
   bool DebugPoint = false;
   unsigned int DebugPointI = (unsigned int) 1e9;
   unsigned int DebugPointJ = (unsigned int) 1e9;

   bool DebugColumn (Column * column)
   {
      return (DebugPoint && column->getI () == DebugPointI && column->getJ () == DebugPointJ);
   }

   const char * ExpulsionDirectionNames[] =
   {
      "None", "Upward", "Downward", "Up- & Downward"
   };

   const char * MigrationProcessNames[] =
   {
      "", "Remigration", "RemigrationLeakage", "LeakageFromTrap", "LeakageToTrap", "Expulsion", "Absorption", "ExpulsionLeakage",
      "Spill", "SpillUpOrOut", "ThroughLeakage", "Diffusion", "Biodegradation", "OiltoGasCrackingLost", "OiltoGasCrackingGained"
   };

#ifdef USEOTGC
   const char * ImmobileNames[] =
   {
      "precoke", "coke1",
      "Hetero1", "coke2",
      "CokeS",
      ""
   };

   const double ImmobileDensities[] =
   {
      1325.295,
      1314.237,
      2331.302,
      2245.047,
      2159.056
   };
#endif

   const bool ComponentsUsed[] =
   {
      true, true, true, true, true,
      true, true,
      true, true,
      true, true,
#ifdef INCLUDE_COX
      true, 
#else
      false,
#endif
      true,

#ifdef INCLUDE_COX // H2S
      true, 
#else
      false,
#endif

      true, true, true, true, true, true, true, true, true
   };

   const char * PhaseNames[] =
   {
      "Gas", "Fluid"
   };

   const double DefaultChargeDensities[] =
   {
      1, 500
   };

   const char * PropertyPrefix = "ResRock";

   ostringstream cerrstrstr;

}
