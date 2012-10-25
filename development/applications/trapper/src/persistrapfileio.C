//
// Abstract Class to write Persistent trap data to file
// 

#include "persistrapfileio.h"

namespace PersisTrapFileIo_NS
{ 
   
//
// PROTECTED METHODS
//
void PersisTrapFileIo::writeForEachReservoir (const ReservoirTrapHistory &traps)
{
   const_ReservoirTrapIt trapIt, endTraps = traps.end();
   for ( trapIt = traps.begin(); trapIt != endTraps; ++trapIt )
   {
      writeForEachAge (trapIt->first, trapIt->second);
   }
}

void PersisTrapFileIo::writeForEachAge (const string& resName, const TrapHistory& traps)
{
   const_TrapHistoryIt trapIt, endTraps = traps.end();
   for ( trapIt = traps.begin(); trapIt != endTraps; ++trapIt )
   {
      addTrapDataToFile (resName.c_str(), trapIt->first, trapIt->second);
   }
}
   
} // end namespace
