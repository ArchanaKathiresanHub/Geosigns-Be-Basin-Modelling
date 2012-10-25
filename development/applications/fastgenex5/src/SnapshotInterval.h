#ifndef _GENEXSIMULATION_SNAPSHOTINTERVAL_H_
#define _GENEXSIMULATION_SNAPSHOTINTERVAL_H
/*
#ifdef sgi
   #ifdef _STANDARD_C_PLUS_PLUS
      #include <iostream>
   #else // !_STANDARD_C_PLUS_PLUS
      #include<iostream.h>
   #endif // _STANDARD_C_PLUS_PLUS
#else // !sgi
   #include <iostream>
#endif // sgi
*/

#include <vector>

namespace DataAccess
{
   namespace Interface
   {
      class Snapshot;
   }
}
#include "GenexSimulation.h"

namespace GenexSimulation
{

class SnapshotInterval
{
public:
   /// Constructor
   SnapshotInterval ( const DataAccess::Interface::Snapshot *start,
                      const DataAccess::Interface::Snapshot *end);
   /// Destructor
   virtual ~SnapshotInterval ();

   /// Get the snapshot at the start of the interval 
   const DataAccess::Interface::Snapshot *getStart(void) const;
   /// Get the snapshot at the end of the interval 
   const DataAccess::Interface::Snapshot *getEnd  (void) const; 
  
private:
   const DataAccess::Interface::Snapshot *m_start;
   const DataAccess::Interface::Snapshot *m_end;
};
inline const DataAccess::Interface::Snapshot *SnapshotInterval::getStart(void) const
{
   return m_start; 
}
inline const DataAccess::Interface::Snapshot *SnapshotInterval::getEnd  (void) const
{
   return m_end;
} 

}
#endif
