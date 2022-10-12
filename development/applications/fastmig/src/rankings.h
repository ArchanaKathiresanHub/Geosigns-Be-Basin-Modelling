//
// Copyright (C) 2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _MIGRATION_RANKINGS_H_
#define _MIGRATION_RANKINGS_H_

#include "petsc.h"

#include <string>

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

   void ReportProgress (const std::string & str1, const std::string & str2 = "", const std::string & str3 = "", double age = -1);

   int GetRank (void);
   std::string & GetRankString (void);
   int GetRank (int i, int j);
   void deleteRanks ();

   void Serialize (void);
   void Deserialize (void);

   bool ComputeRanks (const DataAccess::Interface::Grid * grid);

   int NumProcessors (void);

}

#endif
