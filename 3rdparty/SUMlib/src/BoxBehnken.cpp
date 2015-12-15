// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#include <sstream>

#include "BoxBehnken.h"
#include "DataStructureUtils.h"
#include "Exception.h"
#include "ParameterBounds.h"
#include "SUMlib.h"

using std::vector;

namespace SUMlib {

BoxBehnken::BoxBehnken( vector<bool> const& selPar, unsigned int nbOfOrdPar ) :
   ExpDesignBase( selPar, nbOfOrdPar )
{
   if (numSelOrdFactors() == 6 || numSelOrdFactors() == 7)
   {
      m_nbRuns = 1 + 8*numSelOrdFactors();
   }
   else if (numSelOrdFactors() == 9)
   {
      m_nbRuns = 97;
   }
   else if (numSelOrdFactors() > 9 && numSelOrdFactors() < 13)
   {
      m_nbRuns = 1 + 16*numSelOrdFactors();
   }
   else
   {
      m_nbRuns = 1 + 2*numSelOrdFactors()*(numSelOrdFactors() - 1);
   }
   BoxBehnken::generate();
}

BoxBehnken::~BoxBehnken()
{
}

/**
 *  Generate the design matrix.
 */
void BoxBehnken::generate()
{
   /**
    * In most Box-Behnken designs all combinations of 2 factors are considered.
    * To each combination of 2 factors is assigned (1,1),(1,-1),(-1,1) and (-1,-1),
    * leaving the remaining factors at the center values (design radius = sqrt(2)).
    *
    * In case of 6, 7, 9, 10, 11 or 12 parameters, less combinations are required
    * by assuming a design radius > sqrt(2).
    */

   // First we add the center point
   vector<double> centerPoint( numSelOrdFactors(), 0.0 );
   m_selDesign.push_back( centerPoint );

   if ( numSelOrdFactors() == 6 )
   {
      for ( unsigned int i = 0; i < 3; ++i )
         addDesignRows(3, i, 1+i, 3+i, -1, -1);

      addDesignRows(3, 0, 3, 4, -1, -1);
      addDesignRows(3, 1, 4, 5, -1, -1);
      addDesignRows(3, 0, 2, 5, -1, -1);
   }
   else if ( numSelOrdFactors() == 7 )
   {
      addDesignRows(3, 3, 4, 5, -1, -1);
      addDesignRows(3, 0, 5, 6, -1, -1);
      addDesignRows(3, 1, 4, 6, -1, -1);
      addDesignRows(3, 0, 1, 3, -1, -1);
      addDesignRows(3, 2, 3, 6, -1, -1);
      addDesignRows(3, 0, 2, 4, -1, -1);
      addDesignRows(3, 1, 2, 5, -1, -1);
   }
   else if ( numSelOrdFactors() == 9 )
   {
      for ( unsigned int i = 0; i < 3; ++i )
         addDesignRows(3, i, 3+i, 6+i, -1, -1);

      for ( unsigned int i = 0; i < 3; ++i )
         addDesignRows(3, 3*i, 1+3*i, 2+3*i, -1, -1);

      addDesignRows(3, 0, 4, 8, -1, -1);
      addDesignRows(3, 2, 3, 7, -1, -1);
      addDesignRows(3, 1, 5, 6, -1, -1);
      addDesignRows(3, 0, 5, 7, -1, -1);
      addDesignRows(3, 1, 3, 8, -1, -1);
      addDesignRows(3, 2, 4, 6, -1, -1);
   }
   else if ( numSelOrdFactors() == 10 )
   {
      addDesignRows(4, 1, 5, 6, 9, -1);
      addDesignRows(4, 0, 1, 4, 9, -1);
      addDesignRows(4, 1, 2, 6, 7, -1);
      addDesignRows(4, 1, 3, 5, 8, -1);
      addDesignRows(4, 0, 7, 8, 9, -1);
      addDesignRows(4, 2, 3, 4, 9, -1);
      addDesignRows(4, 0, 3, 6, 7, -1);
      addDesignRows(4, 2, 4, 6, 8, -1);
      addDesignRows(4, 0, 2, 5, 8, -1);
      addDesignRows(4, 3, 4, 5, 7, -1);
   }
   else if ( numSelOrdFactors() == 11 )
   {
      addDesignRows(5, 2, 6, 7, 8, 10);
      addDesignRows(5, 0, 3, 7, 8, 9);
      addDesignRows(5, 1, 4, 8, 9, 10);
      addDesignRows(5, 0, 2, 5, 9, 10);
      addDesignRows(5, 0, 1, 3, 6, 10);

      for ( unsigned int i = 0; i < 4; ++i )
         addDesignRows(5, i, 1+i, 2+i, 4+i, 7+i);

      addDesignRows(5, 0, 4, 5, 6, 8);
      addDesignRows(5, 1, 5, 6, 7, 9);
   }
   else if ( numSelOrdFactors() == 12 )
   {
      for ( unsigned int i = 0; i < 6; ++i )
         addDesignRows(4, i, 1+i, 4+i, 6+i, -1);

      addDesignRows(4, 0, 6, 7, 10, -1);
      addDesignRows(4, 1, 7, 8, 11, -1);

      for ( unsigned int i = 0; i < 3; ++i )
         addDesignRows(4, i, 2+i, 8+i, 9+i, -1);

      addDesignRows(4, 0, 3, 5, 11, -1);
   }
   else if ( numSelOrdFactors() > 1 )
   {
      for ( unsigned int i = 0; i < numSelOrdFactors()-1; ++i )
         for ( unsigned int j = i+1; j < numSelOrdFactors(); ++j )
            addDesignRows(2, i, j, -1, -1, -1);
   }

   // Expand design by creating a column for each unselected parameter
   expandDesign( numOrdFactors(), m_selDesign, m_design );
}

/**
 * Generates a subset of all necessary design rows, depending on the squared design radius R2.
 * The squared radius R2 can take the value 2, 3, 4 or 5.
 * The indices i1, i2, i3, i4, i_alias correspond to the active column numbers (i.e. parameter indices).
 * The number of active columns = R2; i1 and i2 are always active.
 * The column number = -1 if not active. The 5th column, if active, is aliased (product of the other 4).
 */
void BoxBehnken::addDesignRows( unsigned int R2, int i1, int i2, int i3, int i4, int i_alias )
{
   for ( int k4 = 1; k4 < 3; ++k4 )
   {
      for ( int k3 = 1; k3 < 3; ++k3 )
      {
         for ( int k2 = 1; k2 < 3; ++k2 )
         {
            for ( int k1 = 1; k1 < 3; ++k1 )
            {
               vector<double> row( numSelOrdFactors(), 0.0 );
               int val1 = -1 + 2*(k1%2);
               row[i1] = val1;
               int val2 = -1 + 2*(k2%2);
               row[i2] = val2;
               int val3 = -1 + 2*(k3%2);
               int val4 = -1 + 2*(k4%2);
               int val5 = val1*val2*val3*val4;
               if ( R2 == 2 && k3 == 1 && k4 == 1 )
               {
                  m_selDesign.push_back( row );
               }
               else if ( R2 == 3 && k4 == 1 )
               {
                  row[i3] = val3;
                  m_selDesign.push_back( row );
               }
               else if ( R2 > 3 )
               {
                  row[i3] = val3;
                  row[i4] = val4;
                  if (R2 == 5)
                  {
                     row[i_alias] = val5;
                  }
                  m_selDesign.push_back( row );
               }
            }
         }
      }
   }
}

void BoxBehnken::getCaseSetImpl( ParameterBounds const& bounds, Case const& center,
                                 bool replicate, vector<Case>& caseSet ) const
{
   fillCaseSet( bounds, center, replicate, caseSet );
}

std::string BoxBehnken::toString() const
{
   std::stringstream strm;
   DblMatrixToString( strm, m_design );
   return strm.str();
}

} // namespace SUMlib
