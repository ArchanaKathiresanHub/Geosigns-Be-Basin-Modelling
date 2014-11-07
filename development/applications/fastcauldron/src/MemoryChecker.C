#include "MemoryChecker.h"

#include <boost/bind.hpp>
#include <iostream>
#include <sstream>

#ifndef _MSC_VER
#include <sys/sysinfo.h>
#endif

#include "NumericFunctions.h"
#include "System.h"

const unsigned int MemoryChecker::DefaultTimeBetweenSamples = 30;
const unsigned int MemoryChecker::MinimumTimeBetweenSamples = 1;
const unsigned int MemoryChecker::MaximumTimeBetweenSamples = 60;


MemoryChecker::MemoryChecker ( const unsigned int timeBetweenSamples ):
   m_timeBetweenSamples ( NumericFunctions::Maximum ( MinimumTimeBetweenSamples, NumericFunctions::Minimum ( timeBetweenSamples, MaximumTimeBetweenSamples ))),
   m_exit ( false ),
   m_thread ( boost::bind ( &checkMemory, this))
{ 
}

MemoryChecker::~MemoryChecker () {
   m_exit = true;
   m_thread.join ();
}

bool MemoryChecker::exitLoop () const {
   return m_exit;
}

unsigned long MemoryChecker::getMemoryUsed () const {

   StatM statm;

   getStatM ( statm );

   return statm.size * getPageSize ();

}

void MemoryChecker::checkMemory ( const MemoryChecker* mc ) {
      
   long nprocs = getNumberOfCoresOnline ();

   if ( nprocs == 0 ) {
      // If the number of cores is 0 (i.e. unknown) then we cannot calculate the memory per process.
      // This is probably only a result of the windows build.
      return;
   }

   unsigned long memoryPerProcess = 0;

#ifndef _MSC_VER
   struct sysinfo inf;

   sysinfo ( &inf );
   memoryPerProcess = inf.totalram / nprocs;
#endif

   if ( memoryPerProcess == 0 ) {
      // If the memory per process is zero then terminate the thread.
      return;
   }

   while ( not mc->exitLoop ()) {

      if ( mc->getMemoryUsed () > memoryPerProcess ) {
         std::cerr << " checking memory " << mc->getMemoryUsed () << std::endl;
      }

      sleep ( mc->m_timeBetweenSamples );
   }

}

