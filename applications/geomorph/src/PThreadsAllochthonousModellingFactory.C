#include "PThreadsAllochthonousModellingFactory.h"

#include "PThreadsAllochthonousLithology.h"

#include "Interface/ProjectHandle.h"
#include "Interface/AllochthonousLithology.h"


AllochMod::PThreadsAllochthonousModellingFactory::PThreadsAllochthonousModellingFactory ( const int initialNumberOfThreads ) :
  numberOfThreads ( initialNumberOfThreads ) {
}

DataAccess::Interface::AllochthonousLithology *
AllochMod::PThreadsAllochthonousModellingFactory::produceAllochthonousLithology (DataAccess::Interface::ProjectHandle * projectHandle, database::Record * record)
{
   return new PThreadsAllochthonousLithology (projectHandle, record, numberOfThreads );
}
