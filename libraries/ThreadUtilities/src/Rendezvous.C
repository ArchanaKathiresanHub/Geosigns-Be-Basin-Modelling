//------------------------------------------------------------//

#include "Rendezvous.h"

//------------------------------------------------------------//

POSIXThreads::Rendezvous::Rendezvous () {
  acceptBarrier.setBarrierCount ( 2 );
}

//------------------------------------------------------------//

POSIXThreads::Rendezvous::~Rendezvous () {
}

//------------------------------------------------------------//

void POSIXThreads::Rendezvous::setData () {
  acceptBarrier.wait ();
}

//------------------------------------------------------------//

void POSIXThreads::Rendezvous::getData () {
  acceptBarrier.wait ();
}

//------------------------------------------------------------//
