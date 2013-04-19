//
// Abstract Class to write Persistent trap data to file
// 

#ifndef __persistrapfileio__
#define __persistrapfileio__

#include "traplist.h"
using namespace TrapListNameSpace;

#include <string>
#include <map>
using namespace std;

namespace PersisTrapFileIo_NS
{  
   // typedefs used for struct and map containers in PersisTrapFileIo
   
   // trap struct to be written to / read from Hdf5 file
   typedef struct TrapOutput
   {
      int    persisId;
      int    transId;
   } TrapOutput;
   
   // trap mapping from persisId to transId
   typedef map <int, int> TrapOutputMap;
   typedef TrapOutputMap::iterator TrapOutputIt;
   typedef TrapOutputMap::const_iterator const_TrapOutputIt;
   
   // list traps per age
   typedef map <double, TrapOutputMap> TrapHistory;
   typedef TrapHistory::iterator TrapHistoryIt;
   typedef TrapHistory::const_iterator const_TrapHistoryIt;
   
   // list traps per age per reservoir
   typedef map <string, TrapHistory> ReservoirTrapHistory;
   typedef ReservoirTrapHistory::iterator ReservoirTrapIt;
   typedef ReservoirTrapHistory::const_iterator const_ReservoirTrapIt;
   
   class PersisTrapFileIo
   {
   public:
      // CTOR / DTOR
      PersisTrapFileIo () {}
      virtual ~PersisTrapFileIo () {}
      
      // METHODS
      virtual void createFile (const char* filename) = 0;
      virtual void openFileForAppend (const char* filename) = 0;
      virtual void writeTrapHistoryToFile (const ReservoirTrapHistory &traps) = 0;
      
   protected:
      virtual void writeForEachReservoir (const ReservoirTrapHistory &traps);
      virtual void writeForEachAge (const string& resName, const TrapHistory& traps);
      virtual void addTrapDataToFile (const char* reservoirName, 
                              double age, 
                              const TrapOutputMap &traps) = 0;
   };
   
} // end namespace

#endif
