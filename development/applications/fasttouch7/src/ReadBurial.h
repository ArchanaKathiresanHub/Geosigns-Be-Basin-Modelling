#ifndef READBURIAL_H
#define READBURIAL_H

#pragma warning( push )
#pragma warning( disable : 1478 )
// Disable warning 1478 for touchstone 3rd party library
// warning #1478: class "std::auto_ptr" (declared at line xx of "tslFileName") was declared deprecated
#include "tslibI.h"
#pragma warning( pop ) 

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
   void readIndexes(int * firstI, int * lastI, int * firstJ, int * lastJ, int * numLayers, int * numActive) ;
   void readSnapshotsIndexes(std::vector<size_t> & usedSnapshotsIndexes); 
   void readNumTimeStepsID( size_t * numTimeSteps, int * iD) ;
   int readBurialHistory(std::vector<Geocosm::TsLib::burHistTimestep> & burHistTimesteps, size_t numTimeSteps); 	
};

#endif
