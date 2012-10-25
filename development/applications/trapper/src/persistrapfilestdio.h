//
// Class to write persistent trap info to screen when in debug mode
//

#ifndef __persistrapfilestdio__
#define __persistrapfilestdio__

#include "persistrapfileio.h"
#include "includestreams.h"

namespace PersisTrapFileIo_NS
{

class PersisTrapFileStdIo : public PersisTrapFileIo
{
public:
   // override pure virtual base method
   void createFile (const char* filename) { cout << endl << "Persistent trap data for " << filename << endl; }
   void openFileForAppend (const char* filename) { cout << endl << "Persistent trap data for " << filename << endl;  }
   void writeTrapHistoryToFile (const ReservoirTrapHistory &traps);
   
private:
   // override pure virtual base method
   void  addTrapDataToFile (const char* reservoirName, 
                            double age, const TrapOutputMap &traps);
};

} // end namespace

#endif
