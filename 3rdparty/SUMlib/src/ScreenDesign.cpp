// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#include <cassert>
#include <sstream>
#include <string>

#include "Exception.h"
#include "ParameterBounds.h"
#include "ScreenDesign.h"

using std::vector;

namespace SUMlib {

ScreenDesign::ScreenDesign( vector<bool> const& selPar, unsigned int nbOfOrdPar,
                            bool addCenterPoint, bool addMirror ) :
   ExpDesignBase( selPar, nbOfOrdPar ),
   m_addCenterPoint( addCenterPoint ),
   m_addMirror( addMirror ),
   m_nbRuns( 0 )
{
   if ( numSelOrdFactors() >= 48 )
   {
      THROW2( ValueOutOfBounds, "ScreenDesign: number of factors must be < 48" );
   }

   if ( numSelOrdFactors() == 0 )
   {
      m_addMirror = false;
   }
   else if ( numSelOrdFactors() == 1 )
   {
      m_nbRuns = 2;
   }
   else
   {
      m_nbRuns = numSelOrdFactors() + 4 - ( numSelOrdFactors()%4 );
   }
   ScreenDesign::generate();
}

ScreenDesign::~ScreenDesign()
{
}

/**
 *  Generate the design matrix.
 *
 */
void ScreenDesign::generate()
{
   DesignMatrix selDesign;
   intMatrix tempDesign;
   /**
    * Hadamard 2x2 matrix (Had2) needed for the construction of larger Hadamard matrices.
    * These larger ones are needed for the construction of the Plackett-Burman design.
    * Had12 and Had20 are constructed differently by using a generating row.
    */
   intMatrix Had2(2), Had4(4), Had8(8), Had12(12), Had16(16), Had20(20), Had24(24);
   Had2[0].resize(2, 1);
   Had2[1].resize(2, 1);
   Had2[1][1] = -1;
   Had4 = Hadconstr( Had2 );
   Had8 = Hadconstr( Had4 );
   int row11[11] = {1,-1,1,-1,-1,-1,1,1,1,-1,1};
   vector<int> genRow;
   for ( unsigned int i = 0; i < 11; i++ )
      genRow.push_back( row11[i] );
   intMatrix PB12 = PBconstr( genRow );
   genRow.clear();
   Had12 = PB2Had( PB12 );
   Had16 = Hadconstr( Had8 );
   int row19[19] = {1,-1,1,1,-1,-1,-1,-1,1,-1,1,-1,1,1,1,1,-1,-1,1};
   for (unsigned int i = 0; i < 19; i++)
      genRow.push_back( row19[i] );
   intMatrix PB20 = PBconstr( genRow );
   genRow.clear();
   Had20 = PB2Had( PB20 );
   Had24 = Hadconstr( Had12 );

   // Create Plackett-Burman design with number of columns = m_nbRuns - 1.
   intMatrix PBdesign( m_nbRuns );
   RowResize( PBdesign, m_nbRuns-1 );

   // [TODO] to be replaced by switch statement
   if ( m_nbRuns == 2 )
      PBdesign = Had2PB( Had2 );
   else if ( m_nbRuns == 4 )
      PBdesign = Had2PB( Had4 );
   else if ( m_nbRuns == 8 )
      PBdesign = Had2PB( Had8 );
   else if ( m_nbRuns == 12 )
      PBdesign = PB12;
   else if ( m_nbRuns == 16 )
      PBdesign = Had2PB( Had16 );
   else if ( m_nbRuns == 20 )
      PBdesign = PB20;
   else if ( m_nbRuns == 24 )
      PBdesign = Had2PB( Had24 );
   else if ( m_nbRuns == 28 )
   {
      int subPB[27][27] =
      {
         {-1,-1,1,-1,1,1,-1,-1,1,1,1,-1,1,1,-1,-1,1,-1,1,1,-1,1,-1,1,-1,1,-1},
         {-1,-1,1,-1,1,1,-1,1,-1,1,-1,1,-1,-1,1,-1,1,1,-1,-1,1,1,1,-1,1,1,-1},
         {-1,-1,1,1,-1,-1,1,-1,1,1,-1,1,-1,1,-1,1,-1,-1,1,-1,1,1,-1,-1,1,1,1},
         {-1,-1,1,1,-1,-1,1,1,1,-1,1,1,-1,-1,1,-1,1,1,-1,1,-1,1,-1,1,-1,-1,1},
         {-1,-1,1,1,-1,1,-1,1,-1,1,-1,-1,1,-1,1,1,-1,-1,1,1,1,-1,1,1,-1,-1,1},
         {-1,-1,1,1,1,-1,1,1,-1,-1,1,-1,1,1,-1,1,-1,1,-1,1,-1,-1,1,-1,1,1,-1},
         {-1,1,-1,-1,1,-1,1,1,-1,-1,1,1,1,-1,1,1,-1,-1,1,-1,1,1,-1,1,-1,1,-1},
         {-1,1,-1,-1,1,-1,1,1,-1,1,-1,1,-1,1,-1,-1,1,-1,1,1,-1,-1,1,1,1,-1,1},
         {-1,1,-1,-1,1,1,1,-1,1,1,-1,-1,1,-1,1,1,-1,1,-1,1,-1,1,-1,-1,1,-1,1},
         {-1,1,-1,1,-1,-1,1,-1,1,1,-1,-1,1,1,1,-1,1,1,-1,-1,1,-1,1,1,-1,1,-1},
         {-1,1,-1,1,-1,1,-1,-1,1,-1,1,1,-1,-1,1,1,1,-1,1,1,-1,-1,1,-1,1,1,-1},
         {-1,1,-1,1,-1,1,-1,1,-1,-1,1,-1,1,1,-1,-1,1,1,1,-1,1,1,-1,-1,1,-1,1},
         {-1,1,1,-1,1,1,-1,-1,1,-1,1,1,-1,1,-1,1,-1,1,-1,-1,1,-1,1,1,-1,-1,1},
         {1,-1,-1,-1,-1,1,1,-1,-1,-1,1,-1,-1,1,1,-1,-1,-1,-1,1,1,1,1,1,1,1,1},
         {1,-1,-1,-1,-1,1,1,1,1,1,1,1,1,-1,-1,-1,-1,1,1,-1,-1,-1,1,-1,-1,1,1},
         {1,-1,-1,-1,1,-1,-1,1,1,-1,-1,-1,-1,1,1,1,1,1,1,1,1,-1,-1,-1,-1,1,1},
         {1,-1,-1,1,1,-1,-1,-1,-1,1,1,1,1,1,1,1,1,-1,-1,-1,-1,1,1,-1,-1,-1,1},
         {1,-1,-1,1,1,-1,-1,-1,1,-1,-1,1,1,-1,-1,-1,-1,1,1,1,1,1,1,1,1,-1,-1},
         {1,-1,-1,1,1,1,1,1,1,1,1,-1,-1,-1,-1,1,1,-1,-1,-1,1,-1,-1,1,1,-1,-1},
         {1,-1,1,-1,-1,1,1,-1,-1,-1,-1,1,1,1,1,1,1,1,1,-1,-1,-1,-1,1,1,-1,-1},
         {1,1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,1,-1},
         {1,1,1,-1,-1,-1,-1,1,1,-1,-1,-1,1,-1,-1,1,1,-1,-1,-1,-1,1,1,1,1,1,1},
         {1,1,1,-1,-1,-1,-1,1,1,1,1,1,1,1,1,-1,-1,-1,-1,1,1,-1,-1,-1,1,-1,-1},
         {1,1,1,-1,-1,-1,1,-1,-1,1,1,-1,-1,-1,-1,1,1,1,1,1,1,1,1,-1,-1,-1,-1},
         {1,1,1,1,1,-1,-1,-1,-1,1,1,-1,-1,-1,1,-1,-1,1,1,-1,-1,-1,-1,1,1,1,1},
         {1,1,1,1,1,1,1,-1,-1,-1,-1,1,1,-1,-1,-1,1,-1,-1,1,1,-1,-1,-1,-1,1,1},
         {1,1,1,1,1,1,1,1,1,-1,-1,-1,-1,1,1,-1,-1,-1,1,-1,-1,1,1,-1,-1,-1,-1}
      }; // subPB
      for ( unsigned int j = 0; j < 27; j++ )
         PBdesign[0][j] = -1;
      for ( unsigned int i = 0; i < 27; i++ )
         for ( unsigned int j = 0; j < 27; j++ )
            PBdesign[i+1][j] = subPB[i][j];
   }
   else if ( m_nbRuns == 32 )
   {
      intMatrix Had32 = Hadconstr( Had16 );
      PBdesign = Had2PB( Had32 );
   }
   else if ( m_nbRuns == 36 )
   {
      int row35[35] = {-1,1,-1,1,1,1,-1,-1,-1,1,1,1,1,1,-1,1,1,1,-1,-1,1,-1,-1,-1,-1,1,-1,1,-1,1,1,-1,-1,1,-1};
      for ( unsigned int i = 0; i < 35; i++ )
         genRow.push_back( row35[i] );
      PBdesign = PBconstr( genRow );
      genRow.clear();
   }
   else if ( m_nbRuns == 40 )
   {
      intMatrix Had40 = Hadconstr( Had20 );
      PBdesign = Had2PB( Had40 );
   }
   else if ( m_nbRuns == 44 )
   {
      int row43[43] = {1,1,-1,-1,1,-1,1,-1,-1,1,1,1,-1,1,1,1,1,1,-1,-1,-1,1,-1,
         1,1,1,-1,-1,-1,-1,-1,1,-1,-1,-1,1,1,-1,1,-1,1,1,-1};
      for ( unsigned int i = 0; i < 43; i++ )
         genRow.push_back( row43[i] );
      PBdesign = PBconstr( genRow );
      genRow.clear();
   }
   else if ( m_nbRuns == 48 )
   {
      intMatrix Had48 = Hadconstr( Had24 );
      PBdesign = Had2PB( Had48 );
   }

   unsigned int oldNbRuns = m_nbRuns;

   // Firstly, we check the need for a center point.
   if ( m_addCenterPoint )
   {
      vector<int> centerPointRun( numSelOrdFactors(), 0 );
      tempDesign.push_back( centerPointRun );
      m_nbRuns++;
   }

   /**
   * Secondly, we check the need for a mirror design.
   * Finally, enforce that the number of columns equals the number of factors in tempDesign.
   */
   for ( unsigned int i = 0; i < oldNbRuns; i++ )
   {
      vector<int> row( numSelOrdFactors() ), mirrorRow( numSelOrdFactors() );
      for ( unsigned int j = 0; j < numSelOrdFactors(); j++ )
      {
         row[j] = PBdesign[i][j];
         mirrorRow[j] = -row[j];
      }
      tempDesign.push_back( row );
      if ( m_addMirror )
      {
         tempDesign.push_back( mirrorRow );
         m_nbRuns++;
      }
   }

   // Remove duplicate rows in case of addMirror.
   if ( m_addMirror )
   {
      for ( intMatrix::iterator r1 = tempDesign.begin(); r1 != tempDesign.end(); ++r1 )
      {
         for ( intMatrix::iterator r2 = r1 + 1; r2 != tempDesign.end(); ++r2 )
         {
            // Duplicate found?
            if ( *r1 == *r2 )
            {
               tempDesign.erase( r2 );
               m_nbRuns--;
               break;
            }
         } // for all rows following r1
      } // for all rows
   }

   // Convert from intMatrix to dblMatrix
   selDesign.resize( tempDesign.size() );
   for ( unsigned int i = 0; i < selDesign.size(); i++ )
   {
      selDesign[i].resize( tempDesign[i].size() );
      for ( unsigned int j = 0; j < selDesign[i].size(); j++ )
      {
         selDesign[i][j] = double( tempDesign[i][j] );
      }
   }

   // Expand design by creating a column for each unselected parameter.
   expandDesign( numOrdFactors(), selDesign, m_design );
}

intMatrix ScreenDesign::Hadconstr( const intMatrix &Had )
{
   intMatrix newHad;
   // Create first Had.size() rows in newHad.
   for ( unsigned int i = 0; i < Had.size(); i++ )
   {
      vector<int> row( Had[i] );

      // Concatenate row to row.
      for ( unsigned int j = 0; j < Had[i].size(); j++ )
         row.push_back( Had[i][j] );

      newHad.push_back( row );
   }
   // Create second Had.size() rows in newHad.
   for ( unsigned int i = 0; i < Had.size(); i++ )
   {
      vector<int> row( Had[i] );

      // Concatenate -row to row.
      for ( unsigned int j = 0; j < Had[i].size(); j++ )
         row.push_back( -Had[i][j] );

      newHad.push_back( row );
   }
   return newHad;
}

intMatrix ScreenDesign::PBconstr( vector<int>& row )
{
   intMatrix PB;
   vector<int> firstRow( row.size(), -1 );
   PB.push_back( firstRow );
   PB.push_back( row );
   for ( unsigned int i = 1; i < row.size(); i++ )
   {
      int tmp = row.back();
      for ( unsigned int j = row.size()-1; j > 0; j-- )
         row[j] = row[j-1];
      row[0] = tmp;
      PB.push_back( row );
   }
   return PB;
}

intMatrix ScreenDesign::PB2Had( const intMatrix &PB )
{
   intMatrix Had( PB.size() );
   for ( unsigned int i = 0; i < PB.size(); i++ )
   {
      Had[i].resize( Had.size() );
      Had[i][0] = 1;
      for ( unsigned int j = 0; j < PB[i].size(); j++ )
         Had[i][j+1] = PB[i][j];
   }
   return Had;
}

intMatrix ScreenDesign::Had2PB( const intMatrix &Had )
{
   intMatrix PB( Had.size() );
   for ( unsigned int i = 0; i < Had.size(); i++ )
   {
      PB[i].resize( Had.size()-1 );
      for ( unsigned int j = 1; j < Had.size(); j++ )
         PB[i][j-1] = Had[i][j];
   }
   return PB;
}

void ScreenDesign::RowResize( intMatrix &A, int L )
{
   for ( unsigned int i = 0; i < A.size(); i++ )
      if ( L >= 0 ) A[i].resize( L );
}

void ScreenDesign::getCaseSetImpl( ParameterBounds const& bounds, Case const& center,
                                   bool replicate, vector<Case>& caseSet ) const
{
   fillCaseSet( bounds, center, replicate, caseSet );
}

std::string ScreenDesign::toString() const
{
   std::stringstream strm;
   DblMatrixToString( strm, m_design );
   return strm.str();
}

} // namespace SUMlib
