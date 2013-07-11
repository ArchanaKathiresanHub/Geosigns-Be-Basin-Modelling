#ifndef _MIGRATION_RANKINGS_H_
#define _MIGRATION_RANKINGS_H_

#include "petsc.h"

#include <string>
using namespace std;

namespace DataAccess
{
   namespace Interface
   {
      class Grid;
   }
}

namespace migration
{
   void StartTime (void);
   long GetElapsedTime (void);
   long GetLapTime (void);

   void ReportProgress (const string & str1, const string & str2 = "", const string & str3 = "", double age = -1);

   int GetRank (void);
   string & GetRankString (void);
   int GetRank (int i, int j);
   
   void Serialize (void);
   void Deserialize (void);

   bool ComputeRanks (const DataAccess::Interface::Grid * grid);

   int NumProcessors (void);

}

#endif
