#ifndef _GENEXSIMULATION_SNAPSHOTINTERVAL_H_
#define _GENEXSIMULATION_SNAPSHOTINTERVAL_H

#include <vector>

namespace DataAccess
{
   namespace Interface
   {
      class Snapshot;
   }
}

namespace Genex6
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
