// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#include <cassert>
#include <cmath>
#include <sstream>

#include "DataStructureUtils.h"
#include "OptimisedLHD.h"
#include "Case.h"
#include "ParameterBounds.h"
#include "RandomGenerator.h"

namespace {
   // Initialisation of default values
   const unsigned int nrOfRandomStarts       = 5; //integer > 0
   const unsigned int powerInObjFunc         = 2; //integer > 0
}

namespace SUMlib {

OptimisedLHD::OptimisedLHD( vector<bool> const& selPar, unsigned int numOrdFactors,
   unsigned int nbRuns ) : ExpDesignBase( selPar, numOrdFactors )
{
   N = nbRuns;
   K = numSelFactors();
   rg = new RandomGenerator( 79 );

   OptimisedLHD::generate();
}

OptimisedLHD::~OptimisedLHD()
{
   delete rg;
}

/**
 *  Generate the design matrix.
 */
void OptimisedLHD::generate()
{
   /**
    * The unscaled Latin Hypercube uses integer values i from {0, 1,..., N-1} for each coordinate.
    * The first coordinate of point i equals i by convention, fixing the first column of the design.
    * To start with, the other columns are random permutations of {0, 1,..., N-1}.
    * These permutations are locally perturbed to minimise the sum of all squared inverse distances.
    */

   vector<size_t> coordinates( N );
   vector<vector<size_t> > optDesign( N, vector<size_t>( K ) ); //optimal design
   for ( size_t i = 0; i < N; ++i )
   {
      coordinates[i] = i;
      optDesign[i][0] = coordinates[i]; //fix the first column
   }
   vector<vector<size_t> > des( optDesign ); //temporary design
   vector<vector<size_t> > distances( N, vector<size_t>( N ) ); //mutual distances between points
   vector<size_t> dDist( N ); //distance differences between specific points
   double minObjF = 0.5 * N * N; //minimum objective function value, initially set to "huge"

   for ( size_t l = 0; l < nrOfRandomStarts; ++l )
   {
      createRandomLHD( coordinates, des );
      calcDistances( des, distances );
      double objF = calcObjFunc( distances );

      // Search locally for improved LHD's by swapping 2 coordinates in a design column
      for ( size_t i = 0; i < N; ++i ) //index of first coordinate
      {
         for ( size_t j = i + 1; j < N; ++j ) //index of second coordinate
         {
            for ( size_t k = 1; k < K; ++k ) //design column (not the first one!)
            {
               double newObjF = proposeSwap( i, j, k, objF, des, distances, dDist );
               if ( newObjF < objF )
               {
                  adjustDistances( i, j, dDist, distances );
                  objF = newObjF;

                  // Swap design elements
                  size_t temp = des[i][k];
                  des[i][k] = des[j][k];
                  des[j][k] = temp;
               }
            }
         }
      }

      // Update optimal LHD if a better one has been found
      if ( objF < minObjF )
      {
         optDesign = des;
         minObjF = objF;
      }
   }

   // Scale optimal design values to [-1, 1].
   DesignMatrix selDesign( N, vector<double>( K ) );
   for ( size_t i = 0; i < N; ++i )
   {
      for ( size_t k = 0; k < K; ++k )
      {
         selDesign[i][k] = -1.0 + 2.0*( optDesign[i][k] + 0.5 )/N;
      }
   }

   // Expand design by creating a column for each unselected parameter.
   expandDesign( numFactors(), selDesign, m_design );
}

void OptimisedLHD::getCaseSetImpl( ParameterBounds const& bounds, Case const& center,
                        bool replicate, vector<Case>& caseSet ) const
{
   fillCaseSet( bounds, center, caseSet );
}

unsigned int OptimisedLHD::getNbOfCases( ParameterBounds const& bounds, bool replicate ) const
{
   return m_design.size();
}

void OptimisedLHD::createRandomLHD( vector<size_t>& coordinates,
                        vector<vector<size_t> >& design ) const
{
   // Fill the design columns (except the first) by randomly permuted coordinates.
   for ( size_t k = 1; k < K; ++k )
   {
      for ( size_t last = N; last > 1; last-- )
      {
         double r = rg->uniformRandom();
         int randomIndex = int( r*last ); //random index in {0, 1, ..., last-1}
         int temp = coordinates[randomIndex];
         coordinates[randomIndex] = coordinates[last-1];
         coordinates[last-1] = temp;
         design[last-1][k] = coordinates[last-1];
      }
      // Nr of runs = nr of rows = N >= 1
      design[0][k] = coordinates[0];
   }
}

void OptimisedLHD::calcDistances( vector<vector<size_t> > const& design,
                        vector<vector<size_t> >& distances ) const
{
   for ( size_t i = 0; i < N; ++i )
   {
      for ( size_t j = i + 1; j < N; ++j )
      {
         distances[i][j] = 0;
         for ( size_t k = 0; k < K; ++k )
         {
            size_t diff_k = design[i][k] - design[j][k];
            distances[i][j] += diff_k * diff_k;
         }
      }
   }
}

double OptimisedLHD::calcObjFunc( vector<vector<size_t> > const& distances ) const
{
   double objFunc = 0.0;
   for ( size_t i = 0; i < N; ++i )
   {
      for ( size_t j = i + 1; j < N; ++j )
      {
         objFunc += calcObjTerm( distances[i][j] );
      }
   }
   return objFunc;
}

double OptimisedLHD::proposeSwap( size_t ix, size_t jx, size_t kx, double objF,
                        vector<vector<size_t> > const& design,
                        vector<vector<size_t> > const& distances,
                        vector<size_t>& dDist ) const
{
   assert( ix < jx );
   assert( jx < N );
   assert( kx > 0 );
   assert( kx < K );
   assert( dDist.size() == N );
   assert( objF > 0.0 );
   double objFunc = objF;
   for ( size_t i = 0; i < N; ++i )
   {
      size_t d1 = design[i][kx] - design[ix][kx];
      d1 *= d1;
      size_t d2 = design[i][kx] - design[jx][kx];
      d2 *= d2;
      dDist[i] = d2 - d1;
      size_t newDist;
      if ( i < ix )
      {
         objFunc -= calcObjTerm( distances[i][ix] );
         newDist = distances[i][ix] + dDist[i];
         objFunc += calcObjTerm( newDist );
      }
      else if ( ( i > ix ) && ( i != jx ) )
      {
         objFunc -= calcObjTerm( distances[ix][i] );
         newDist = distances[ix][i] + dDist[i];
         objFunc += calcObjTerm( newDist );
      }
      if ( ( i < jx ) && ( i != ix ) )
      {
         objFunc -= calcObjTerm( distances[i][jx] );
         newDist = distances[i][jx] - dDist[i];
         objFunc += calcObjTerm( newDist );
      }
      else if ( i > jx )
      {
         objFunc -= calcObjTerm( distances[jx][i] );
         newDist = distances[jx][i] - dDist[i];
         objFunc += calcObjTerm( newDist );
      }
   }
   assert( objFunc > 0.0 );
   return objFunc;
}

void OptimisedLHD::adjustDistances( size_t ix, size_t jx,
                        vector<size_t> const& dDist,
                        vector<vector<size_t> >& distances ) const
{
   assert( ix < jx );
   assert( jx < N );
   assert( dDist.size() == N );
   for ( size_t i = 0; i < N; ++i )
   {
      if ( i < ix )
      {
         distances[i][ix] += dDist[i];
      }
      else if ( ( i > ix ) && ( i != jx ) )
      {
         distances[ix][i] += dDist[i];
      }
      if ( ( i < jx ) && ( i != ix ) )
      {
         distances[i][jx] -= dDist[i];
      }
      else if ( i > jx )
      {
         distances[jx][i] -= dDist[i];
      }
   }
}

double OptimisedLHD::calcObjTerm( size_t distance ) const
{
   assert( distance > 0 );
   double objTerm;
   if ( powerInObjFunc == 2 )
   {
      objTerm = 1.0 / distance;
   }
   else
   {
      objTerm = 1.0 / pow( distance, 0.5*powerInObjFunc );
   }
   return objTerm;
}

std::string OptimisedLHD::toString() const
{
   std::stringstream strm;
   DblMatrixToString( strm, m_design );
   return strm.str();
}

} // namespace SUMlib
