#ifndef __RendezvousPrimitives_HH__
#define __RendezvousPrimitives_HH__

#include "Barrier.h"
#include "Mutex.h"

namespace POSIXThreads {

  /** @addtogroup POSIXThreads
   *
   * @{
   */

  /// \file Rendezvous.h
  /// \brief Provides protected data accessing classes for threads.

  /// \brief Permits the synchronisation of threads.
  class Rendezvous {

  public :

    /// \brief Simple constructor.
    Rendezvous ();

    /// \brief Simple destructor.
    ~Rendezvous ();

    // Should rename these, since no data is exchanged, perhaps even get rid of this class,
    // since it is possible to use only a barrier.
    void setData ();
    void getData ();


  private:

    /// \brief Part of the access controlling mechanism for the rendezvous.
    Mutex   acceptMutex;

    /// \brief Part of the access controlling mechanism for the rendezvous.
    Barrier acceptBarrier;

  };

  //------------------------------------------------------------//

  /// \brief Encapsulates a single data item that is to be shared between threads.
  ///
  /// The 'rendezvous' ...
  /// process1->setData ( data )    |    process2->getData ( data )
  /// It is guaranteed that writing of the data by process1 will complete
  /// before process2 can read it. These actions, by process1 and process2, 
  /// must occur together, not doing so will result in deadlock.
  template <typename DataType>
  class Rendezvous1 {

  public :

    /// \brief Simple constructor.
    Rendezvous1 ();

    /// \brief Simple destructor.
    ~Rendezvous1 () {}

    /// \brief Set the data that is to be shared between main programme and thread.
    void setData ( const DataType& threadData );

    /// \brief Get shared data.
    void getData (       DataType& threadData );

  private:

    /// \brief Part of the access controlling mechanism for the shared data.
    Mutex   acceptMutex;

    /// \brief Part of the access controlling mechanism for the shared data.
    Barrier acceptBarrier;

    /// \brief The data that is to be shared between threads.
    DataType sharedData;

  };

  //------------------------------------------------------------//

  /// \brief Encapsulates two data items that are to be shared between threads.
  template <typename DataType1, typename DataType2>
  class Rendezvous2 {

  public :

    /// \brief Simple constructor.
    Rendezvous2 ();

    /// \brief Simple destructor.
    ~Rendezvous2 () {}

    /// \brief Set the data that is to be shared between main programme and thread.
    void setData ( const DataType1& threadData1,
                   const DataType2& threadData2 );

    /// \brief Get shared data.
    void getData (       DataType1& threadData1,
                         DataType2& threadData2 );

  private:

    /// \brief Part of the access controlling mechanism for the shared data.
    Mutex   acceptMutex;

    /// \brief Part of the access controlling mechanism for the shared data.
    Barrier acceptBarrier;

    /// \brief The data that is to be shared between threads.
    DataType1 sharedData1;

    /// \brief The data that is to be shared between threads.
    DataType2 sharedData2;

  };

  /** @} */

}

//------------------------------------------------------------//

template <typename DataType>
POSIXThreads::Rendezvous1<DataType>::Rendezvous1 () : acceptBarrier ( 2 ) {}

//------------------------------------------------------------//

template <typename DataType>
void POSIXThreads::Rendezvous1<DataType>::setData  ( const DataType& threadData ) {

  // Lock the mutex around the sharedData variable, to prevent the thread from accessing it.
  acceptMutex.lock ();

  // Wait here until both the main programme and the thread are waiting on this barrier
  acceptBarrier.wait ();

  // Copy the data that is to be passed between the main programme and the thread
  sharedData = threadData;

  // Release the lock so that the thread can read the shared data
  acceptMutex.unlock ();

}

//------------------------------------------------------------//

template <typename DataType>
void POSIXThreads::Rendezvous1<DataType>::getData  ( DataType& threadData ) {

  // Wait here until both the thread and the main programme are waiting on this barrier.
  acceptBarrier.wait ();

  // Lock the mutex around the sharedData variable, to prevent the main programme from further accessing it.
  acceptMutex.lock ();

  // Copy the shared data to pass back to thread.
  threadData = sharedData;

  // Release the lock ready for the next interchange.
  acceptMutex.unlock ();

}

//------------------------------------------------------------//

template <typename DataType1, typename DataType2>
POSIXThreads::Rendezvous2<DataType1, DataType2>::Rendezvous2 () : acceptBarrier ( 2 ) {}

//------------------------------------------------------------//


template <typename DataType1, typename DataType2>
void POSIXThreads::Rendezvous2<DataType1, DataType2>::setData  ( const DataType1& threadData1,
                                                                 const DataType2& threadData2 ) {

  // Lock the mutex around the sharedData variable, to prevent the thread from accessing it.
  acceptMutex.lock ();

  // Wait here until both the main programme and the thread are waiting on this barrier
  acceptBarrier.wait ();

  // Copy the data that is to be passed between the main programme and the thread
  sharedData1 = threadData1;
  sharedData2 = threadData2;

  // Release the lock so that the thread can read the shared data
  acceptMutex.unlock ();

}

//------------------------------------------------------------//

template <typename DataType1, typename DataType2>
void POSIXThreads::Rendezvous2<DataType1, DataType2>::getData  ( DataType1& threadData1,
                                                                 DataType2& threadData2 ) {

  // Wait here until both the thread and the main programme are waiting on this barrier.
  acceptBarrier.wait ();

  // Lock the mutex around the sharedData variable, to prevent the main programme from further accessing it.
  acceptMutex.lock ();

  // Copy the shared data to pass back to thread.
  threadData1 = sharedData1;
  threadData2 = sharedData2;

  // Release the lock ready for the next interchange.
  acceptMutex.unlock ();

}

//------------------------------------------------------------//

#endif // __RendezvousPrimitives_HH__
