//------------------------------------------------------------//

#include "Barrier.h"

//------------------------------------------------------------//

POSIXThreads::Barrier::Barrier ( const int numberOfThreadsToWaitFor ) {
  pthread_mutex_init(&lock, NULL);
  pthread_cond_init(&cv, NULL);
  numberSleepers = 0;
  releasing = false;
  threadCount = numberOfThreadsToWaitFor;
}

//------------------------------------------------------------//

POSIXThreads::Barrier::Barrier () {
  pthread_mutex_init(&lock, NULL);
  pthread_cond_init(&cv, NULL);
  numberSleepers = 0;
  releasing = false;
  threadCount = 1;
}

//------------------------------------------------------------//

POSIXThreads::Barrier::~Barrier () {
  pthread_mutex_destroy(&lock);
  pthread_cond_destroy(&cv);
}

//------------------------------------------------------------//

void POSIXThreads::Barrier::setBarrierCount ( const int numberOfThreadsToWaitFor ) {
  threadCount = numberOfThreadsToWaitFor;
}

//------------------------------------------------------------//

void POSIXThreads::Barrier::wait () {

  bool release = false;

  pthread_mutex_lock(&lock);

  while (releasing) {
    //
    //
    // If previous cycle still releasing, wait 
    //
    pthread_cond_wait(&cv, &lock);
  } // end while loop

  numberSleepers = numberSleepers + 1;

  if ( numberSleepers == threadCount ) {
    releasing = true;
    release   = true;
  } else { 

    while (!releasing) {
      pthread_cond_wait(&cv, &lock);
    } // end loop

  } // end if

  numberSleepers = numberSleepers - 1;

  if (numberSleepers == 0) {
    releasing = false;
    //
    //
    // Wake up waiters (if any) for next cycle 
    //
    release = true;
  } // end if

  pthread_mutex_unlock(&lock);

  if (release) {
    pthread_cond_broadcast(&cv);
  } // end if

}

//------------------------------------------------------------//
