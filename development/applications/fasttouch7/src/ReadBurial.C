//                                                                      
// Copyright (C) 2012-2017 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

#include "ReadBurial.h"

ReadBurial::ReadBurial(const char * filename)
	: m_filename(filename)		
{
}

//read functions

void ReadBurial::readIndexes(int * firstI, int * lastI, int * firstJ, int * lastJ, int * numLayers, int * numActive)
{
	m_filename >> *firstI >> *lastI >> *firstJ >> *lastJ >> *numLayers >> *numActive;
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
   for ( size_t ii = 0; ii < usedSnapshotsIndexes.size(); ++ii ) m_filename >> usedSnapshotsIndexes[ii];
   
}

int ReadBurial::readBurialHistory(std::vector<Geocosm::TsLib::burHistTimestep > & burHistTimesteps, size_t numTimeSteps) 
{
   int numActive = 0;
	for ( size_t bt = 0; bt < numTimeSteps; ++bt )
	{   								
		m_filename >> burHistTimesteps[bt].time; 
		m_filename >> burHistTimesteps[bt].temperature;
		m_filename >> burHistTimesteps[bt].depth;
		m_filename >> burHistTimesteps[bt].effStress;
		m_filename >> burHistTimesteps[bt].waterSat;
		m_filename >> burHistTimesteps[bt].overPressure; 
		
		// if temperature is defined also ves and pressure are, so it is valid timestep
		if (burHistTimesteps[bt].temperature != 99999) numActive +=1;	
	}
	
	return numActive;
}
