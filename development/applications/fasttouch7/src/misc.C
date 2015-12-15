#include "mpi.h"
#include "misc.h"
#include "ProgressManager.h"
#include <iostream>
#include <string>

int fasttouch::GetRank()
{
   static int rank = -1;
   if ( rank < 0 )
   {
      MPI_Comm_rank(MPI_COMM_WORLD, &rank);
   }
   return rank;
}

int fasttouch::GetSize()
{
   static int size = -1;
   if ( size < 0 )
   { 
      MPI_Comm_size( MPI_COMM_WORLD, &size );
   }
   return size;
}

static utilities::ProgressManager FastTouchProgressManager;

void fasttouch::StartProgress()
{
   FastTouchProgressManager.start();
}

void fasttouch::ReportProgress(const std::string & message)
{
   if ( GetRank() == 0 ) std::cout << FastTouchProgressManager.report( message ) << std::endl;
}


