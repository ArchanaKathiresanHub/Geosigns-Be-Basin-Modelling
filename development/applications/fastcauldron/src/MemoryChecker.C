#include "MemoryChecker.h"

#include <unistd.h>
#include <boost/bind.hpp>
#include <iostream>
#include <sys/sysinfo.h>
#include <sstream>
#include <stdio.h>

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

   return statm.size * getpagesize ();

}

void MemoryChecker::checkMemory ( const MemoryChecker* tc ) {
      
   long nprocs = sysconf (_SC_NPROCESSORS_ONLN);

   if ( nprocs == 0 ) {
      return;
   }

   unsigned long memoryPerProcess;
   struct sysinfo inf;

   sysinfo ( &inf );
   memoryPerProcess = inf.totalram / nprocs;

   while ( not tc->exitLoop ()) {

      if ( tc->getMemoryUsed () > memoryPerProcess ) {
         std::cerr << " checking memory " << tc->getMemoryUsed () << std::endl;
      }

      sleep ( tc->m_timeBetweenSamples );
   }

}

