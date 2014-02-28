// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.


#include <cmath>
#include <sstream>

#include "FaureDesign.h"
#include "Case.h"
#include "ParameterBounds.h"
#include "DataStructureUtils.h"


namespace SUMlib {

FaureDesign::FaureDesign( vector<bool> const& selPar, unsigned int numOrdFactors,
   unsigned int nbOldRuns, unsigned int nbNewRuns ) : ExpDesignBase( selPar, numOrdFactors )
{
   m_nbOldRuns = nbOldRuns;
   m_nbNewRuns = nbNewRuns;

   FaureDesign::generate();
}

FaureDesign::~FaureDesign()
{
}

// Store the first 26 primes (2,3,...,97,101) in an array.
static const size_t prime[26] = {2,3,5,7,11,13,17,19,23,29,31,37,41,43,47,53,59,61,67,71,73,79,83,89,97,101};

/**
 *  Generate the design matrix.
 */
void FaureDesign::generate()
{
   DesignMatrix selDesign;
   /**
    * The Faure design uses the "van der Corput" sequence for the first dimension.
    * This sequence is permuted for the other dimensions.
    * All values are initially between 0 and 1; at the end they are scaled to [-1, +1].
    */

   // The base is the smallest prime which is equal or larger than the number of parameters.
   size_t base = prime[0];
   size_t i = 0;
   while( ( base < numSelFactors() ) && ( i < 25 ) )
   {
      i++;
      base = prime[i];
   }

   // Determine the sequence number at which the Faure sequence starts (new ED) or continues (augmenting ED).
   //size_t nSeq = int(pow(base,4.0)) + m_nbOldRuns;
   size_t nSeq = 1 + m_nbOldRuns;

   for ( size_t irun = 0; irun < m_nbNewRuns; ++irun )
   {
      vector<double> row( numSelFactors(), 0.0 );

      /**
      * nSeq is expanded in powers of the base.
      * The expansion leads to nbTerms terms with corresponding coefficients a[j].
      */
      size_t nbTerms = 1;
      size_t nTest = nSeq > 0 ? nSeq : 1;
      size_t nbLoops = 0;
      while ( nTest%base == 0 )
      {
         nTest /= base;
         nbLoops++;
      }
      if ( nTest == 1 )
      {
         nbTerms += nbLoops;
      }
      else
      {
         nbTerms += static_cast<size_t>( floor( log( double( nSeq ) )/log( double( base ) ) ) );
      }

      vector<size_t> a( nbTerms );
      size_t n_red = nSeq;
      for ( int j = nbTerms - 1; j >= 0; --j )
      {
         a[j] = n_red/int( pow( base, double( j ) ) );
         n_red -= a[j]*int( pow( base, double( j ) ) );
      }

      // Calculate the "van der Corput" numbers and store them in row[i].
      // reusing index i declared above
      for ( i = 0; i < numSelFactors(); ++i )
      {
         vector<size_t> a_old( nbTerms );
         for ( size_t j = 0; j < nbTerms; ++j )
         {
            row[i] += double( a[j] )/pow( base, double( j + 1 ) );
            a_old[j] = a[j];
         }

         // Permutation of a[j] in the other dimensions
         for ( size_t j = 0; j < nbTerms; ++j )
         {
            a[j] = a_old[j];
            for ( size_t k = j+1; k < nbTerms; ++k )
            {
               a[j] += fac(k)*a_old[k]/(fac(j)*fac(k-j));
            }
            a[j] = a[j]%base;
         }
      }
      selDesign.push_back( row );
      nSeq++;
   }

   // Scale selDesign values to [-1, 1].
   for ( i = 0; i < selDesign.size(); ++i )
   {
      for ( size_t j = 0; j < selDesign[i].size(); ++j )
      {
         selDesign[i][j] = -1.0 + 2*selDesign[i][j];
      }
   }

   // Expand design by creating a column for each unselected parameter.
   expandDesign( numFactors(), selDesign, m_design );
}

size_t FaureDesign::fac( size_t n )
{
   if ( n < 2 ) return 1;
   else
   {
      size_t k = n - 1;
      while ( k > 1 )
      {
         n*=k;
         k--;
      }
      return n;
   }
}

void FaureDesign::getCaseSetImpl( ParameterBounds const& bounds, Case const& center,
                                  bool replicate, vector<Case>& caseSet ) const
{
   fillCaseSet( bounds, center, caseSet );
}

unsigned int FaureDesign::getNbOfCases( ParameterBounds const& bounds, bool replicate ) const
{
   return m_design.size();
}

std::string FaureDesign::toString() const
{
   std::stringstream strm;
   DblMatrixToString( strm, m_design );
   return strm.str();
}

} // namespace SUMlib
