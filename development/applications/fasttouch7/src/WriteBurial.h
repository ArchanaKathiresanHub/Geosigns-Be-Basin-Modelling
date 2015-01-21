#ifndef WRITEBURIAL_H
#define WRITEBURIAL_H

#include "BurialHistoryTimeStep.h"
#include "DataPipe.h"
#include <vector>

//this class defines how burial histories should be written
class WriteBurial 
{
private:
   DataPipe  m_filename;
public:
   WriteBurial(const char * filename);
//write methods
   void writeIndexes(int firstI, int lastI, int firstJ, int lastJ) ;
   void writeSnapshotsIndexes(const std::vector<size_t> & usedSnapshotsIndexes);
   void writeBurialHistory(const std::vector<BurialHistoryTimeStep> & burHistTimesteps, int iD) ;	
};

#endif
