//                                                                      
// Copyright (C) 2012-2017 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

#include "WriteBurial.h"

WriteBurial::WriteBurial(const char * filename)
   : m_filename(filename)		
{

}

//write functions

void WriteBurial::writeIndexes(int firstI, int lastI, int firstJ, int lastJ, int numLayers, int numActive) 
{
   m_filename << firstI << lastI << firstJ << lastJ << numLayers << numActive; 
}

void WriteBurial::writeSnapshotsIndexes(const std::vector<size_t> & usedSnapshotsIndexes) 
{
   m_filename << usedSnapshotsIndexes.size(); 
   
   for ( size_t sn = 0; sn < usedSnapshotsIndexes.size(); ++sn )
   {
      m_filename << usedSnapshotsIndexes[sn]; 
   }
}

void WriteBurial::writeBurialHistory(const std::vector<BurialHistoryTimeStep > * burHistTimesteps, int iD)
{

   size_t size;

   if ( burHistTimesteps == nullptr )
   {
      size = 0;
   }
   else
   {
      size = burHistTimesteps->size();
   }

   m_filename << iD;   //the  global i j position
   m_filename << size; //the number of time steps to read

   for ( size_t bt = 0; bt < size; ++bt )
   {
      m_filename << (*burHistTimesteps)[bt].time;
      m_filename << (*burHistTimesteps)[bt].temperature;
      m_filename << (*burHistTimesteps)[bt].depth;
      m_filename << (*burHistTimesteps)[bt].effStress;
      m_filename << (*burHistTimesteps)[bt].waterSat;
      m_filename << (*burHistTimesteps)[bt].overPressure;
   }
}

