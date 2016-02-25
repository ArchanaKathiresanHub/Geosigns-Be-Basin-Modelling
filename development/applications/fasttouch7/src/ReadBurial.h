#ifndef READBURIAL_H
#define READBURIAL_H

#include "tslibI.h"
#include "DataPipe.h"
#include <vector>

//this class defines how burial histories should be written
class ReadBurial 
{
 private:
   DataPipe  m_filename;
 public:
   ReadBurial(const char * filename);
   //read methods
   void readIndexes(int * firstI, int * lastI, int * firstJ, int * lastJ, int * numLayers) ;
   void readSnapshotsIndexes(std::vector<size_t> & usedSnapshotsIndexes); 
   void readNumTimeStepsID( size_t * numTimeSteps, int * iD) ;
   void readBurialHistory(std::vector<Geocosm::TsLib::burHistTimestep > & burHistTimesteps, size_t numTimeSteps); 	
};

#endif
