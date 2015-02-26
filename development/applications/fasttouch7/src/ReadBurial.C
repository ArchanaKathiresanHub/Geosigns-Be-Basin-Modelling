#include "ReadBurial.h"

ReadBurial::ReadBurial(const char * filename)
	: m_filename(filename)		
{
}

//read functions

void ReadBurial::readIndexes(int * firstI, int * lastI, int * firstJ, int * lastJ, int * numLayers)
{
	m_filename >> *firstI >> *lastI >> *firstJ >> *lastJ >> *numLayers;
}

void ReadBurial::readNumTimeStepsID( size_t * numTimeSteps, int * iD)
{
	m_filename >> *iD;
	m_filename >> *numTimeSteps;
}

void ReadBurial::readSnapshotsIndexes(std::vector<size_t> & usedSnapshotsIndexes) 
{
   size_t usedSnapshotsIndexesSize = 0;
   m_filename >> usedSnapshotsIndexesSize;
   usedSnapshotsIndexes.resize(usedSnapshotsIndexesSize);
   for ( int ii = 0; ii < usedSnapshotsIndexes.size(); ++ii ) m_filename >> usedSnapshotsIndexes[ii];
   
}

void ReadBurial::readBurialHistory(std::vector<Geocosm::TsLib::burHistTimestep > & burHistTimesteps, int numTimeSteps) 
{

	for ( size_t bt = 0; bt < numTimeSteps; ++bt )
	{   								
		m_filename >> burHistTimesteps[bt].time; 
		m_filename >> burHistTimesteps[bt].temperature;
		m_filename >> burHistTimesteps[bt].depth;
		m_filename >> burHistTimesteps[bt].effStress;
		m_filename >> burHistTimesteps[bt].waterSat;
		m_filename >> burHistTimesteps[bt].overPressure; 	
	}
}
