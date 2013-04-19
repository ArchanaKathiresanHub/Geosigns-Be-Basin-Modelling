#ifdef sgi
  #ifdef _STANDARD_C_PLUS_PLUS
      #include<iostream>
     using namespace std;
      #define USESTANDARD
  #else // !_STANDARD_C_PLUS_PLUS
      #include<iostream.h>
  #endif // _STANDARD_C_PLUS_PLUS
#else // !sgi
   #include <iostream>
   using namespace std;
   #define USESTANDARD
#endif // sgi

#include "petscvec.h"
#include "petscda.h"

#include "GenexSimulation.h"

int Genex6::GetRank (void)
{
   static int rank = -1;
   if (rank < 0) {
      int mpiRank;
      MPI_Comm_rank (PETSC_COMM_WORLD, &mpiRank);
      rank = mpiRank;
   }

   return rank;
}
