#ifdef sgi
   #ifdef _STANDARD_C_PLUS_PLUS
      #include<iostream>
      #include <iomanip>
      using namespace std;
      #define USESTANDARD
   #else // !_STANDARD_C_PLUS_PLUS
      #include<iostream.h>
      #include <iomanip.h>
   #endif // _STANDARD_C_PLUS_PLUS
#else // !sgi
   #include <iostream>
   #include <iomanip>
   using namespace std;
   #define USESTANDARD
#endif // sgi

#include <assert.h>
#include <unistd.h>
#include <time.h>

#include "rankings.h"
#include "array.h"

#include "Interface/Grid.h"

#include "migration.h"
#include "RequestDefs.h"

namespace migration
{
   static int ** Ranks = 0;
   static bool UseProxiesAllowed = true;
}

int migration::NumProcessors (void)
{
   static int numProcessors = -1;
   if (numProcessors  < 0)
   {
      MPI_Comm_size (PETSC_COMM_WORLD, &numProcessors);
   }

   return numProcessors;
}

int migration::GetRank (void)
{
   static int rank = -1;
   if (rank  < 0)
   {
      int mpiRank;
      MPI_Comm_rank (PETSC_COMM_WORLD, &mpiRank);
      rank = mpiRank;
   }

   return rank;
}

void migration::Serialize (void)
{
   int rank;
   for (rank = -1; rank < GetRank (); ++rank)
   {
      MPI_Barrier (PETSC_COMM_WORLD);
   }
}

void migration::Deserialize (void)
{
   int rank;
   for (rank = GetRank (); rank <= NumProcessors (); ++rank)
   {
      MPI_Barrier (PETSC_COMM_WORLD);
   }
}


long SecondsAtStart = 0;
long SecondsAtLapTime = 0;

void migration::StartTime (void)
{
   timespec tp;
   clock_gettime(CLOCK_REALTIME, &tp);
   SecondsAtStart = SecondsAtLapTime = tp.tv_sec;
}

long migration::GetElapsedTime (void)
{
   timespec tp;
   clock_gettime(CLOCK_REALTIME, &tp);
   return tp.tv_sec - SecondsAtStart;
}


long migration::GetLapTime (void)
{
   timespec tp;
   clock_gettime(CLOCK_REALTIME, &tp);
   long lapTime = tp.tv_sec - SecondsAtLapTime;
   SecondsAtLapTime = tp.tv_sec;
   return lapTime;
}

/// produces a formatted progress statement on cout
void migration::ReportProgress (const string & str1, const string & str2, const string & str3, double age)
{
   if (GetRank () == 0)
   {
      long time = GetElapsedTime ();
      cout << "o Migration:: " << str1 << str2 << str3;
      int len = str1.size () + str2.size () + str3.size ();

      while (len < 32)
      {
	 cout << " ";
	 ++len;
      }

      if (age >= 0)
      {
	 cout << "Snapshot: " << setfill (' ') << setw (6) << age << " Ma   ";
      }
      else
      {
	 while (len < 54)
	 {
	    cout << " ";
	    ++len;
	 }
      }

      cout << "Time: " << setw (4) << setfill (' ') << time / 60 << ":" << setw (2) << setfill ('0') << time % 60 << endl;
      cout.flush ();
   }
}

string & migration::GetRankString (void)
{
   static string fullRankString;

   timespec tp;
   clock_gettime(CLOCK_REALTIME, &tp);

   char timestr[32];
   sprintf (timestr, "%9ld.%9ld\t", tp.tv_sec, tp.tv_nsec);

   static string rankString = "";
   if (rankString == "")
   {
      for (int rank = GetRank () - 1; rank >=0; --rank)
      {
	 rankString += "    ";
      }
      char tmp[4];
      sprintf (tmp, "%d", GetRank ());
      rankString += tmp;
   }

   fullRankString = "";
   fullRankString += timestr;
   fullRankString += rankString;
   return fullRankString;
}


int migration::GetRank (int i, int j)
{
   if (!Ranks)
   {
      return -1;
   }
   else
   {
      return Ranks[i][j];
   }
}

bool migration::ComputeRanks (const DataAccess::Interface::Grid * grid)
{
   int numI = grid->numIGlobal ();
   int numJ = grid->numJGlobal ();

   int sendbuf[4];
   sendbuf[0] = grid->firstI (false);
   sendbuf[1] = grid->lastI (false);
   sendbuf[2] = grid->firstJ (false);
   sendbuf[3] = grid->lastJ (false);

   int * rcvbuf = Array<int>::create1d (NumProcessors () * 4);

   char hostname[128];
   gethostname (hostname, 128);

   PetscSynchronizedPrintf (PETSC_COMM_WORLD, "Rank: %2d, Host: %s, I: %3d - %3d, J: %3d - %3d\n", GetRank (), hostname, sendbuf[0], sendbuf[1], sendbuf[2], sendbuf[3]);
   PetscSynchronizedFlush (PETSC_COMM_WORLD);

   MPI_Allgather (sendbuf, 4, MPI_INT, rcvbuf, 4, MPI_INT, PETSC_COMM_WORLD);
   Ranks = Array<int>::create2d (numI, numJ);

   int rank, i, j;
   
   for (i = 0; i < numI; ++i)
   {
      for (j = 0; j < numJ; ++j)
      {
	 Ranks[i][j] = -1;
      }
   }

   for (rank = 0; rank < NumProcessors (); ++rank)
   {
      for (i = rcvbuf[rank * 4 + 0]; i<= rcvbuf[rank * 4 + 1]; ++i)
      {
	 for (j = rcvbuf[rank * 4 + 2]; j <= rcvbuf[rank * 4 + 3]; ++j)
	 {
	    assert (Ranks[i][j] == -1);
	    Ranks[i][j] = rank;
	 }
      }
   }

   for (i = 0; i < numI; ++i)
   {
      for (j = 0; j < numJ; ++j)
      {
	 assert (Ranks[i][j] != -1);
      }
   }

   Array<int>::delete1d (rcvbuf);

   return true;
}



