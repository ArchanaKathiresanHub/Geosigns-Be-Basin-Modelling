#ifdef sgi
   #ifdef _STANDARD_C_PLUS_PLUS
      #include <iostream>
   #else // !_STANDARD_C_PLUS_PLUS
      #include<iostream.h>
   #endif // _STANDARD_C_PLUS_PLUS
#else // !sgi
   #include <iostream>
#endif // sgi

#include "SnapshotInterval.h"

#include <vector>
#include "Interface/Snapshot.h"
#include <math.h>



namespace Genex6
{

using DataAccess::Interface::Snapshot;

SnapshotInterval::SnapshotInterval ( const Snapshot *start,
                                     const Snapshot *end):
m_start(start),
m_end(end)
{

}
SnapshotInterval::~SnapshotInterval ()
{

}
  
}//end of namespace GenerationExpulsion




