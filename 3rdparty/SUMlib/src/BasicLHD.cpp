// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#include <cmath>
#include <sstream>

#include "BasicLHD.h"

#include "DataStructureUtils.h"
#include "Case.h"
#include "ParameterBounds.h"
#include "RandomGenerator.h"

namespace SUMlib {

BasicLHD::BasicLHD( vector<bool> const& selPar, unsigned int numOrdFactors,
   unsigned int nbRuns ) : ExpDesignBase( selPar, numOrdFactors )
{
   m_nbRuns = nbRuns;
   rg = new RandomGenerator( 79 );

   BasicLHD::generate();
}

BasicLHD::~BasicLHD()
{
   delete rg;
}

/**
 *  Generate the design matrix.
 */
void BasicLHD::generate()
{
   DesignMatrix selDesign;
   selDesign.resize( m_nbRuns );

   /**
    * The basic Latin Hypercube uses a collection of evenly distributed points for the first dimension.
    * This collection is randomly permuted for the other dimensions.
    * All values are between -1 and 1.
    */

   vector<double> column( m_nbRuns, 0.0 );

   for ( size_t irun = 0; irun < m_nbRuns; ++irun )
   {
      column[irun] = -1.0 + 2.0*( irun + 0.5 )/m_nbRuns;

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
      // Nr of runs >= 1
      selDesign[0][dim-1] = column[0];
   }

   // Expand design by creating a column for each unselected parameter.
   expandDesign( numFactors(), selDesign, m_design );
}

void BasicLHD::getCaseSetImpl( ParameterBounds const& bounds, Case const& center,
                               bool replicate, vector<Case>& caseSet ) const
{
   fillCaseSet( bounds, center, caseSet );
}

unsigned int BasicLHD::getNbOfCases( ParameterBounds const& bounds, bool replicate ) const
{
   return m_design.size();
}

std::string BasicLHD::toString() const
{
   std::stringstream strm;
   DblMatrixToString( strm, m_design );
   return strm.str();
}

} // namespace SUMlib
