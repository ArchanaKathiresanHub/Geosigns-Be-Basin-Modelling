// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#include <sstream>
#include <iomanip>

#include "DataStructureUtils.h"
#include "ParameterBounds.h"
#include "Tornado.h"
#include "SUMlib.h"

using std::vector;

namespace SUMlib {

Tornado::Tornado( vector<bool> const& selPar, unsigned int nbOfOrdPar ) :
   ExpDesignBase( selPar, nbOfOrdPar )
{
   Tornado::generate();
}


Tornado::~Tornado()
{
}

/**
 *  Generate the design matrix.
 */
void Tornado::generate()
{
   DesignMatrix selDesign;

   /**
    * In the Tornado design each factor is treated separately
    * by assigning a low case (-1) and a high case (1),
    * leaving the remaining factors at the center values.
    */

   // Firstly, we add the center point.
   vector<double> centerPoint( numSelOrdFactors(), 0.0 );
   selDesign.push_back( centerPoint );

   // Then we add the low and high cases.
   for ( unsigned int i = 0; i < numSelOrdFactors(); ++i )
   {
      vector<double> row( numSelOrdFactors(), 0.0 );
      row[i] = -1.0;
      selDesign.push_back( row );
      row[i] = 1.0;
      selDesign.push_back( row );
   }

   // Expand design by creating a column for each unselected parameter.
   expandDesign( numOrdFactors(), selDesign, m_design );
}

void Tornado::getCaseSetImpl( ParameterBounds const& bounds, Case const& center,
                              bool replicate, vector<Case>& caseSet ) const
{
   fillCaseSet( bounds, center, replicate, caseSet );
}

std::string Tornado::toString() const
{
   std::stringstream s;

   DblMatrixToString( s, m_design );

   return s.str();
}

} // namespace SUMlib
