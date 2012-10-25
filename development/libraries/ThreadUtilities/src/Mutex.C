//------------------------------------------------------------//

#include "Mutex.h"

//------------------------------------------------------------//

POSIXThreads::Mutex::Mutex () {
  pthread_mutex_init ( &mutexObject, NULL );
}

//------------------------------------------------------------//

POSIXThreads::Mutex::~Mutex () {
  pthread_mutex_destroy ( &mutexObject );
}

//------------------------------------------------------------//

void POSIXThreads::Mutex::lock () {
  pthread_mutex_lock ( &mutexObject );
}

//------------------------------------------------------------//

void POSIXThreads::Mutex::unlock () {
  pthread_mutex_unlock ( &mutexObject );
}

//------------------------------------------------------------//
