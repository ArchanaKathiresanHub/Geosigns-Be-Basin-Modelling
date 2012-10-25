#include "PThreadsAllochthonousLithology.h"

#include "PThreadsMapIntervalInterpolator.h"



AllochMod::PThreadsAllochthonousLithology::PThreadsAllochthonousLithology (  DataAccess::Interface::ProjectHandle * projectHandle,
                                                                             database::Record * record,
                                                                             const int initialNumberOfThreads ) :
  AllochthonousLithology ( projectHandle, record ),
  numberOfThreads ( initialNumberOfThreads ) {
}

AllochMod::MapIntervalInterpolator* AllochMod::PThreadsAllochthonousLithology::allocateIntervalInterpolator () const {
  return new PThreadsMapIntervalInterpolator ( numberOfThreads );
}
