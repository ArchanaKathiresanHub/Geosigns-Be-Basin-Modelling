//------------------------------------------------------------//

#ifndef __POSIXThreadsMutex_HH__
#define __POSIXThreadsMutex_HH__

//------------------------------------------------------------//

#include <pthread.h>

//------------------------------------------------------------//

namespace POSIXThreads {

  /** @addtogroup POSIXThreads
   *
   * @{
   */


  /// \file Mutex.h
  /// \brief Provides a mutex class for shared memory architecture.

  /// \brief POSIX threads mutex object.
  class Mutex {

  public :

    Mutex ();
    ~Mutex ();

    /// \brief Obtain the lock.
    ///
    /// If the mutex has been locked by another process then the
    /// process requiring the lock will have to wait.
    void lock ();

    /// Unlock the mutex.
    void unlock ();

  private:

    /// \brief pthreads implementation of the mutex object.
    pthread_mutex_t mutexObject;

  }; // end struct Mutex


  /** @} */


}

//------------------------------------------------------------//

#endif // __POSIXThreadsMutex_HH__

//------------------------------------------------------------//
