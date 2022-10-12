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

      void addComment (const std::string& comment);

      void printBalance () const;
      void printMassBalance (const std::string& name) const;
      void printMassBalance (const DataAccess::Interface::Snapshot* start,
         const DataAccess::Interface::Snapshot* end, const std::string& name) const;

      void addToBalance (const std::string& description, double quantity);
      void subtractFromBalance (const std::string& description, double quantity);
      void clear ();

      double balance () const;

   private:
      std::vector<std::string>m_comments;

      std::vector<std::string>m_additionDescriptions;
      std::vector<double> m_additionQuantities;

      std::vector<std::string>m_subtractionDescriptions;
      std::vector<double> m_subtractionQuantities;

   };

} // namespace migration

#endif
