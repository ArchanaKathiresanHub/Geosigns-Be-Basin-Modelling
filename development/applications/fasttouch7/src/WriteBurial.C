#include "WriteBurial.h"

WriteBurial::WriteBurial(const char * filename)
   : m_filename(filename)		
{

}

//write functions

void WriteBurial::writeIndexes(int firstI, int lastI, int firstJ, int lastJ) 
{
   m_filename << firstI << lastI << firstJ << lastJ; 
}

void WriteBurial::writeSnapshotsIndexes(const std::vector<size_t> & usedSnapshotsIndexes) 
{
   m_filename << usedSnapshotsIndexes.size(); 
   
   for ( int sn = 0; sn < usedSnapshotsIndexes.size(); ++sn )
   {
      m_filename << usedSnapshotsIndexes[sn]; 
   }
}

void WriteBurial::writeBurialHistory(const std::vector<BurialHistoryTimeStep > & burHistTimesteps, int iD) 
{
   m_filename << iD;
   m_filename << burHistTimesteps.size();

   for ( int bt = 0; bt < burHistTimesteps.size(); ++bt )
   {
      m_filename << burHistTimesteps[bt].time; 
      m_filename << burHistTimesteps[bt].temperature;
      m_filename << burHistTimesteps[bt].depth;
      m_filename << burHistTimesteps[bt].effStress;
      m_filename << burHistTimesteps[bt].waterSat;
      m_filename << burHistTimesteps[bt].overPressure;    
   }
}
