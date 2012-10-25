#include "persistrapfilestdio.h"

namespace PersisTrapFileIo_NS
{
 
//
// PUBLIC METHODS
//
void PersisTrapFileStdIo::writeTrapHistoryToFile (const ReservoirTrapHistory &traps)
{
   writeForEachReservoir (traps);
}   

//
// PRIVATE METHODS
//
void PersisTrapFileStdIo::addTrapDataToFile (const char* reservoirName, 
                                             double age, const TrapOutputMap &traps)
{
   const int trapSize = traps.size();
   if ( trapSize < 1 ) return;
   
   cout << endl << reservoirName << " " << age << endl;
   const_TrapOutputIt trapIt, endTraps = traps.end();
   for ( trapIt = traps.begin(); trapIt != endTraps; ++trapIt )
   {
      cout << "PersisID: " << trapIt->first << " TransID: " << trapIt->second << endl;
   } 
   cout << endl;
}

} // end namespace
