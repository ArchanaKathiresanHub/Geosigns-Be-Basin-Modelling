// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#include <cmath>
#include <sstream>

#include "DataStructureUtils.h"
#include "HybridMC.h"
#include "Case.h"
#include "ParameterBounds.h"
#include "RandomGenerator.h"

using std::vector;

namespace SUMlib {

HybridMC::HybridMC( vector<bool> const& selPar, unsigned int numOrdFactors,
   unsigned int nbOldRuns, unsigned int nbNewRuns ) : ExpDesignBase( selPar, numOrdFactors )
{
   m_nbOldRuns = nbOldRuns;
   m_nbNewRuns = nbNewRuns;
   rg = new RandomGenerator( 79 );

   HybridMC::generate();
}

HybridMC::~HybridMC()
{
   delete rg;
}

/**
 *  Generate the design matrix.
 */
void HybridMC::generate()
{
   DesignMatrix selDesign;
   selDesign.resize( m_nbNewRuns );

   /**
    * The Hybrid Quasi-Monte Carlo design uses the "van der Corput" sequence for the first dimension.
    * This sequence (in base 2) is randomly permuted for the other dimensions.
    * All values are between 0 and 1.
    */

   static const size_t base = 2;

   // Determine the number at which the sequence starts (new ED) or continues (augmenting ED).
   size_t nSeq = base + m_nbOldRuns;

   vector<double> column( m_nbNewRuns, 0.0 );

   for ( size_t irun = 0; irun < m_nbNewRuns; ++irun )
   {
      /**
      * nSeq is expanded in powers of the base.
      * The expansion leads to nbTerms terms with corresponding coefficients a[j].
      */
      size_t nbTerms = 1;
      size_t nTest = nSeq;
      size_t nbLoops = 0;
      while ( nTest%base == 0 )
      {
         nTest /= base;
         nbLoops++;
      }
      if (nTest == 1)
      {
         nbTerms += nbLoops;
      }
      else
      {
         nbTerms += static_cast<size_t>(floor( log(double(nSeq))/log(double(base)) ));
      }

      vector<size_t> a( nbTerms );
      size_t n_red = nSeq;
      for ( int j = static_cast<int>(nbTerms) - 1; j >= 0; --j)
      {
         a[j] = n_red/int( pow( base, double( j ) ) );
         n_red -= a[j]*int( pow( base, double( j ) ) );
      }

      // Calculate the "van der Corput" numbers and store them in column[irun].
      for ( size_t j = 0; j < nbTerms; ++j )
      {
         column[irun] += double( a[j] )/pow( base, double( j + 1 ) );
      }
      nSeq++;
      selDesign[irun].resize( numSelFactors() );
      selDesign[irun][0] = column[irun];
   }

   // Permute randomly for the other dimensions.
   for ( size_t dim = 2; dim <= numSelFactors(); ++dim )
   {
      for ( size_t last = column.size(); last > 1; last-- )
      {
         double r = rg->uniformRandom();
         int randomIndex = int( r*last ); //random index in {0, 1, ..., last-1}
         double temp = column[randomIndex];
         column[randomIndex] = column[last-1];
         column[last-1] = temp;
         selDesign[last-1][dim-1] = column[last-1];
      }
      // [ABU] assumes nr of new runs >= 1
      selDesign[0][dim-1] = column[0];
   }

   // Scale selDesign values to [-1, 1].
   for ( size_t i = 0; i < selDesign.size(); ++i )
   {
      for ( size_t j = 0; j < selDesign[i].size(); ++j )
      {
         selDesign[i][j] = -1.0 + 2*selDesign[i][j];
      }
   }

   // Expand design by creating a column for each unselected parameter.
   expandDesign( numFactors(), selDesign, m_design );
}

void HybridMC::getCaseSetImpl( ParameterBounds const& bounds, Case const& center,
                               bool, vector<Case>& caseSet ) const
{
   fillCaseSet( bounds, center, caseSet );
}

unsigned int HybridMC::getNbOfCases( ParameterBounds const&, bool ) const
{
   return static_cast<unsigned int>( m_design.size() );
}

std::string HybridMC::toString() const
{
   std::stringstream strm;
   DblMatrixToString( strm, m_design );
   return strm.str();
}

} // namespace SUMlib
