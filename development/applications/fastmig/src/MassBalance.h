//
// Copyright (C) 2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _MIGRATION_MASSBALANCE_H_
#define _MIGRATION_MASSBALANCE_H_

#include "Snapshot.h"

// #define DEBUG_MASSBALANCE 1

using namespace DataAccess;

namespace migration
{

   template <typename STREAM>
   class MassBalance
   {
   private:

      STREAM& m_massBalanceStream;

      double m_massBalance;

#ifdef DEBUG_MASSBALANCE
      static int m_highestMassBalanceIndex;
      int m_massBalanceNumber;
#endif

   public:

      MassBalance (STREAM& stream);

      void addComment (const string& comment);

      void printBalance () const;
      void printMassBalance (const string& name) const;
      void printMassBalance (const DataAccess::Interface::Snapshot* start,
         const DataAccess::Interface::Snapshot* end, const string& name) const;

      void addToBalance (const string& description, double quantity);
      void subtractFromBalance (const string& description, double quantity);
      void clear ();

      double balance () const;

   private:
      vector<string> m_comments;

      vector<string> m_additionDescriptions;
      vector<double> m_additionQuantities;

      vector<string> m_subtractionDescriptions;
      vector<double> m_subtractionQuantities;

   };

} // namespace migration

#endif
