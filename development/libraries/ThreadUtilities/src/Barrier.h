#ifndef __PosixBarrier_HH__
#define __PosixBarrier_HH__

#include <pthread.h>

/*! \defgroup POSIXThreads POSIXThreads
 *
 */

namespace POSIXThreads {

  /** @addtogroup POSIXThreads
   *
   * @{
   */

  /// \file Barrier.h
  /// \brief Provides a barrier class for use with pthreads.

  /// \namespace POSIXThreads
  /// \brief Provides posix threads related types.

  /// \brief Synchronise a number of processes at the barrier.
  class Barrier {

  public :

    /// \brief Default constructor.
    Barrier ();

    /// \brief constructor.
    ///
    /// \param numberOfThreadsToWaitFor how many threads are to be waited for.
    Barrier  ( const int numberOfThreadsToWaitFor );

    ~Barrier ();

    /// \brief Set the number of threads to wait for.
    ///
    ///
    /// \param numberOfThreadsToWaitFor how many threads are to be waited for.
    void setBarrierCount ( const int numberOfThreadsToWaitFor );

    /// \brief Wait here until the required number of threads are waiting.
    void wait ();

  private:

    /// \brief lock for structure            
    pthread_mutex_t lock;

    /// \brief waiting list control          
    pthread_cond_t  cv;

    /// \brief Number of threads to wait for 
    int             threadCount;

    /// \brief Number of threads to waiting  
    int             numberSleepers;

    /// \brief Still waking up sleepers      
    int             releasing;

  };

  /** @} */

}

#endif // __PosixBarrier_HH__
