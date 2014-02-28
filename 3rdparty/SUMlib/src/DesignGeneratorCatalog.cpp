// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#include <algorithm>
#include <sstream>
#include <cstring>

#include "DataStructureUtils.h"
#include "DesignGeneratorCatalog.h"
#include "Exception.h"

namespace SUMlib {

// http://www.itl.nist.gov/div898/handbook/pri/section3/pri3346.htm

const char* DesignGeneratorCatalog::theFactorialDesigns[] = {
   "2   4   999",
   "",
   "3   8   999",
   "",
   "3   4   3",
   "1   2   3",
   "",
   "4   16   999",
   "",
   "4   8   4",
   "1   2   3   4",
   "",
   "5   32   999",
   "",
   "5   16   5",
   "1   2   3   4   5",
   "",
   "5   8   3",
   "1   2   4",
   "1   3   5",
   "",
   "6   64   999",
   "",
   "6   32   6",
   "1   2   3   4   5   6",
   "",
   "6   16   4",
   "1   2   3   5",
   "2   3   4   6",
   "",
   "6   8   3",
   "1   2   4",
   "1   3   5",
   "2   3   6",
   "",
   "7   128   999",
   "",
   "7   64   7",
   "1   2   3   4   5   6   7",
   "",
   "7   32   4",
   "1   2   3   4   6",
   "1   2   4   5   7",
   "",
   "7   16   4",
   "1   2   3   5",
   "2   3   4   6",
   "1   3   4   7",
   "",
   "7   8   3",
   "1   2   4",
   "1   3   5",
   "2   3   6",
   "1   2   3   7",
   "",
   "8   128   8",
   "1   2   3   4   5   6   7   8",
   "",
   "8   64   5",
   "1   2   3   4   7",
   "1   2   5   6   8",
   "",
   "8   32   4",
   "1   2   3   6",
   "1   2   4   7",
   "2   3   4   5   8",
   "",
   "8   16   4",
   "2   3   4   5",
   "1   3   4   6",
   "1   2   3   7",
   "1   2   4   8",
   "",
   "9   128   6",
   "1   3   4   6   7   8",
   "2   3   5   6   7   9",
   "",
   "9   64   4",
   "1   2   3   4   7",
   "1   3   5   6   8",
   "3   4   5   6   9",
   "",
   "9   32   4",
   "2   3   4   5   6",
   "1   3   4   5   7",
   "1   2   4   5   8",
   "1   2   3   5   9",
   "",
   "9   16   3",
   "1   2   3   5",
   "2   3   4   6",
   "1   3   4   7",
   "1   2   4   8",
   "1   2   3   4   9",
   "",
   "10   128   5",
   "1   2   3   7   8",
   "2   3   4   5   9",
   "1   3   4   6   10",
   "",
   "10   64   4",
   "2   3   4   6   7",
   "1   3   4   6   8",
   "1   2   4   5   9",
   "1   2   3   5   10",
   "",
   "10   32   4",
   "1   2   3   4   6",
   "1   2   3   5   7",
   "1   2   4   5   8",
   "1   3   4   5   9",
   "2   3   4   5   10",
   "",
   "10   16   3",
   "1   2   3   5",
   "2   3   4   6",
   "1   3   4   7",
   "1   2   4   8",
   "1   2   3   4   9",
   "1   2   10",
   "",
   "11   128   5",
   "1   2   3   7   8",
   "2   3   4   5   9",
   "1   3   4   6   10",
   "1   2   3   4   5   6   7   11",
   "",
   "11   64   4",
   "3   4   5   7",
   "1   2   3   4   8",
   "1   2   6   9",
   "2   4   5   6   10",
   "1   4   5   6   11",
   "",
   "11   32   4",
   "1   2   3   6",
   "2   3   4   7",
   "3   4   5   8",
   "1   3   4   9",
   "1   4   5   10",
   "2   4   5   11",
   "",
   "11   16   3",
   "1   2   3   5",
   "2   3   4   6",
   "1   3   4   7",
   "1   2   4   8",
   "1   2   3   4   9",
   "1   2   10",
   "1   3   11",
   "",
   "12   128   4",
   "1   2   3   8",
   "1   2   4   9",
   "1   5   6   10",
   "2   4   6   7   11",
   "1   3   7   10   12",
   "",
   "12   64   4",
   "1   3   6   7",
   "2   3   4   6   8",
   "1   2   5   7   9",
   "1   4   5   7   10",
   "2   4   5   6   7   11",
   "2   3   4   5   7   12",
   "",
   "15   16   3",
   "1   2   5",
   "1   3   6",
   "1   4   7",
   "2   3   8",
   "2   4   9",
   "3   4   10",
   "1   2   3   11",
   "1   2   4   12",
   "1   3   4   13",
   "2   3   4   14",
   "1   2   3   4   15",
   "",
   ""
};

DesignGeneratorCatalog* DesignGeneratorCatalog::theCatalog = NULL;

const DesignGeneratorCatalog* DesignGeneratorCatalog::GetInstance()
{
   if ( theCatalog == NULL )
   {
      theCatalog = new DesignGeneratorCatalog;
   }
   return theCatalog;

}

DesignGeneratorCatalog::DesignGeneratorCatalog()
{
   // Initialisation is necessary only once
   if ( m_catalog.size() > 0 )
   {
      return;
   }

   int i = 0;
   while ( strlen( theFactorialDesigns[i] ) )
   {
      // The first line of a fact. design contains the nr. of parameters, the
      // nr. of runs and the resolution.
      std::istringstream linestr( theFactorialDesigns[i] );
      std::string nbparstr;
      int nbpar, nbruns, resolution;
      linestr >> nbpar >> nbruns >> resolution;

      ++i;

      // All subsequent nonempty lines contain generator data for aliasing
      intMatrix aliasingData;
      while ( strlen( theFactorialDesigns[i] ) )
      {
         // The sign is always first, and the sign is always 1(!)
         intVector columns( 1, 1 );
         int column;

         // Read the line
         std::istringstream dlinestr( theFactorialDesigns[i] );
         while ( dlinestr >> column )
         {
            columns.push_back( column );
         }
         aliasingData.push_back( columns );
         ++i;
      }
      // Create a new factorial design instance
      m_catalog.push_back( DesignGenerator( nbpar, nbruns, resolution, aliasingData ) );

      ++i;
   }
}

/// Function object for sorting the design generator catalog
struct DesignGeneratorSort
{
   bool operator()( DesignGenerator const& dg1, DesignGenerator const& dg2 ) const
   {
      return dg1 < dg2;
   }
} ;

void DesignGeneratorCatalog::getFactorialDesigns(
      const unsigned int nbPar,
      const unsigned int minimumNbRuns,
      const unsigned int minimumResolution,
      std::vector<DesignGenerator>& dgvec ) const
{
   dgvec.clear();

   std::vector<DesignGenerator>::const_iterator dgit;
   for ( dgit = m_catalog.begin(); dgit != m_catalog.end(); ++dgit )
   {
      if ( dgit->numParameters() == nbPar && dgit->numRuns() >= minimumNbRuns && dgit->resolution() >= minimumResolution )
      {
         dgvec.push_back( *dgit );
      }
   }
   std::sort( dgvec.begin(), dgvec.end() );
}

} // namespace SUMlib
