#ifndef __AllochMod_PThreadsAllochthonousLithology_HH__
#define __AllochMod_PThreadsAllochthonousLithology_HH__

#include <iostream>
#include <string>

// DataAccess
#include "Interface/AllochthonousLithology.h"
#include "Interface/ProjectHandle.h"

// TableIO
#include "database.h"

// Application
#include "AllochthonousLithology.h"
#include "MapIntervalInterpolator.h"

#include "hdf5.h"


namespace AllochMod {

  /** @addtogroup AllochMod
   *
   * @{
   */

  /// \file PThreadsAllochthonousLithology.h
  /// \brief Calculates the allochthonous lithology movement for a single formation in parallel using pthreads.


  using Numerics::FloatingPoint;

  /// \brief Calculates the allochthonous lithology movement for a single formation in parallel using pthreads.
  class PThreadsAllochthonousLithology : public AllochthonousLithology {

  public :

    /// Constructor.
    PThreadsAllochthonousLithology ( DataAccess::Interface::ProjectHandle * projectHandle, database::Record * record, const int initialNumberOfThreads );

  private :

    MapIntervalInterpolator* allocateIntervalInterpolator () const;

    const int numberOfThreads;

  };

  /** @} */

}

#endif // __AllochMod_PThreadsAllochthonousLithology_HH__
