#include <cassert>
#include <time.h>
#include <iostream>
#include <string>
#include <utility> 
#include "omp.h"

#include "computeProperty.h"

std::string getCompilerName()
{
   std::string ver( __VERSION__ );
   return ver.substr( 0, 5 );
}

void testOMP () {

  int myId;
#pragma omp parallel private( myId )
  {
     myId = omp_get_thread_num();
     
     // std::cout << "Hello World from thread " <<   myId << std::endl;

#pragma omp barrier

     if ( myId == 0 ) {
        
      int nthreads = omp_get_num_threads();
      std::cout <<  "There are " << nthreads << " threads" << std::endl;
    }
  }
}

int main ( int argc, char ** argv )
{

   testOMP();

   double CPS = static_cast<double>( CLOCKS_PER_SEC );
   size_t bufSize = 1024 * 1024 * 128;
 
   Properties computeProps ( bufSize );
   clock_t t1 = clock();
   computeProps.computeDensity( bufSize );
   double totalTime = static_cast<double>( (clock() - t1 ) ) / CPS;

   std::cout << getCompilerName() << " = " << totalTime << " (Density)"<< std::endl;

   
   t1 = clock();
   computeProps.computePermeability( 1024 * 1024 * 128, Homogeneous );
   totalTime = static_cast<double>( (clock() - t1 ) ) / CPS;

   std::cout << getCompilerName() << " = " << totalTime << " (Permeability Homogeneous)"<< std::endl;

   t1 = clock();
   computeProps.computePermeability( 1024 * 1024 * 128, Layered );
   totalTime = static_cast<double>( (clock() - t1 ) ) / CPS;

   std::cout << getCompilerName() << " = " << totalTime << " (Permeability Layered)" << std::endl << std::endl;
}
