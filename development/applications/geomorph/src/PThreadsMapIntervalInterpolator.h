#ifndef __PThreadsMapIntervalInterpolator_HH__
#define __PThreadsMapIntervalInterpolator_HH__


#include "MapIntervalInterpolator.h"


namespace AllochMod {

  /** @addtogroup AllochMod
   *
   * @{
   */

  /// \file PThreadsIntervalInterpolator.h
  /// \brief A parallel interval interpolator using pthreads.

  /// \brief A parallel interval interpolator using pthreads.
  class PThreadsMapIntervalInterpolator : public MapIntervalInterpolator {

  public :

    PThreadsMapIntervalInterpolator ( const int initialNumberOfThreads );

  private :

    /// \brief Compute an interpolator for the given surface, exterior and interior point sets using the threaded version of the matrix and preconditioner.
    virtual void computeInterpolator ( const int debugLevel );

    /// The number of threads to use in the computation.
    const int numberOfThreads;

  };

  /** @} */

}

#endif // __PThreadsMapIntervalInterpolator_HH__
