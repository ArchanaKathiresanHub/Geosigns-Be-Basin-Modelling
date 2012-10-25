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
   "", "Remigration", "RemigrationLeakage", "LeakageFromTrap", "LeakageToTrap", "Expulsion", "Absorption", "ExpulsionLeakage", "Spill", "SpillUpOrOut", "ThroughLeakage", "Diffusion", "Biodegradation", "OiltoGasCrackingLost", "OiltoGasCrackingGained"
};

const char * ComponentNames[] = 
{
   "asphaltenes", "resins",
   "C15+Aro", "C15+Sat",
   "C6-14Aro", "C6-14Sat",
   "C5", "C4", "C3", "C2", "C1",
   "COx", "N2", "H2S",
   "LSC", "C15+AT", "C6-14BT", "C6-14DBT", "C6-14BP", "C15+AroS", "C15+SatS", "C6-14SatS", "C6-14AroS", 
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

const char * TableComponentNames[] = 
{
   "asphaltenes", "resins",
   "C15Aro", "C15Sat",
   "C6_14Aro", "C6_14Sat",
   "C5", "C4", "C3", "C2", "C1",
   "COx", "N2", "H2S",
   "LSC", "C15AT", "C6_14BT", "C6_14DBT", "C6_14BP", "C15AroS", "C15SatS", "C6_14SatS", "C6_14AroS"
};


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
