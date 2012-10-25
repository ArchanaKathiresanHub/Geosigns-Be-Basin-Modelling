#ifndef GENEXSIMULATION_H
#define GENEXSIMULATION_H

#include <string>
namespace GenexSimulation
{
   ///helper function to determine the rank of the processor
   int GetRank (void);
   namespace Constants
   {
      const double ZERO = 0.001;
   }
}
#endif 
