// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#include <cassert>
#include <cmath>
#include <sstream>

#include "DataStructureUtils.h"
#include "DesignGeneratorCatalog.h"
#include "Exception.h"
#include "FactDesign.h"
#include "ParameterBounds.h"
#include "SUMlib.h"

using std::vector;

namespace SUMlib {

// http://www.itl.nist.gov/div898/handbook/pri/section3/pri3346.htm
//
FactDesign::FactDesign( vector<bool> const& selPar, unsigned int nbOfOrdPar,
                        DesignGenerator const& dg, FactDesignType type ) :
   ExpDesignBase( selPar, nbOfOrdPar ),
   m_type( type ),
   m_designGenerators( dg.data() )
{
   FactDesign::generate();
}

// Recommended constructor!
FactDesign::FactDesign( vector<bool> const& selPar, unsigned int nbOfOrdPar, FactDesignType type,
                        vector<AliasingData> const& gen ) :
   ExpDesignBase( selPar, nbOfOrdPar ),
   m_type( type ),
   m_designGenerators( gen )
{
   FactDesign::generate();
}

FactDesign::~FactDesign()
{
}

/**
 *  Generate the design matrix.
 *
 */

void FactDesign::generate()
{
   checkDesignGenerators();
   m_selDesign.clear();

   // A single design row, once set, to be copied to m_design
   vector<double> des( numSelOrdFactors() );

   // First include center point
   for ( unsigned int i = 0; i < numSelOrdFactors(); ++i )
   {
      des[i] = 0;
   }
   m_selDesign.push_back( des );

   // Now generate the non-confounded (= not aliased) columns
   if ( numSelOrdFactors() > 0 )
   {
      generateRuns( numSelOrdFactors() - static_cast<unsigned int>( m_designGenerators.size() ) - 1, des);
   }

   // Add confounded/aliased columns using the design generators
   for ( unsigned int i = 0; i < m_designGenerators.size(); ++i )
   {
      AliasingData &gen = m_designGenerators[i];

      // Last element is the column index to be aliased
      unsigned int c = gen.back() - 1;

      // First element of gen is the sign (+ or - 1)
      for ( unsigned int r = 0; r < m_selDesign.size(); ++r )
      {
         m_selDesign[r][c] = gen[0] < 0 ? -1 : 1;
      }

      // In between elements are column indices + 1
      for ( unsigned int genc = 1; genc < gen.size()-1; ++genc )
      {
         for ( unsigned int r = 0; r < m_selDesign.size(); ++r )
         {
            m_selDesign[r][c] *= m_selDesign[r][ gen[genc] - 1 ];
         }
      }
   }

   // Add star points for a central composite design
   if ( m_type == FACE_CENTERED || m_type == INSCRIBED || m_type == CIRCUMSCRIBED )
   {
      // Face centered
      double alfa = 1;

      if ( m_type == INSCRIBED || m_type == CIRCUMSCRIBED )
      {
         int n = numSelOrdFactors() - static_cast<unsigned int>( m_designGenerators.size() );
         alfa = pow( 2.0, 1.0*n );
         alfa = pow(alfa, 0.25 );
      }

      // Add (circumscribed or face centered) star points
      for ( unsigned int r = 0; r < numSelOrdFactors(); ++r )
      {
         for ( unsigned int i = 0; i < numSelOrdFactors(); ++i )
         {
            des[i] = ( i == r ) ? alfa : 0;
         }
         m_selDesign.push_back( des );

         for ( unsigned int i = 0; i < numSelOrdFactors(); ++i )
         {
            des[i] = ( i == r ) ? -alfa : 0;
         }
         m_selDesign.push_back( des );
      }

      if ( m_type == INSCRIBED || m_type == CIRCUMSCRIBED )
      {
         // Scale back to the proper size
         for ( unsigned int r = 0; r < m_selDesign.size(); ++r )
         {
            vector<double> &run = m_selDesign[r];
            for ( unsigned int c = 0; c < run.size(); ++c )
            {
               run[c] /= alfa;
               if ( m_type == INSCRIBED )
               {
                  run[c] /= alfa;
               }
            }
         }
      }
   }

   // Expand design by creating a column for each unselected parameter.
   expandDesign( numOrdFactors(), m_selDesign, m_design );
}

// Generate design columns for all the unaliased/unconfounded columns.
// The level is the number of unconfounded columns (to be still handled).
void FactDesign::generateRuns( unsigned int level, vector<double> &des )
{
   if ( des.size() <= level )
   {
      THROW2( IndexOutOfBounds, "Level must be smaller than current design size." );
   }
   if ( level == 0 )
   {
      // Bottom level negative shift
      des[level] = -1;
      m_selDesign.push_back( des );

      // Bottom level positive shift
      des[level] = +1;
      m_selDesign.push_back( des );
   }
   else
   {
      // The negative shift of this factor and all levels below
      des[level] = -1;
      generateRuns( level-1, des );

      // The positive shift of this factor and all levels below
      des[level] = +1;
      generateRuns( level-1, des );
   }
}

void FactDesign::checkDesignGenerators()
{
   if ( numSelOrdFactors() < 3 ) m_designGenerators.clear(); //no aliasing needed
   if ( numSelOrdFactors() > 0 && m_designGenerators.size() >= numSelOrdFactors() )
   {
      // Not all factors can be confounded
      THROW2( IndexOutOfBounds, "Too many design generators supplied." );
   }
   for ( unsigned int i = 0; i < m_designGenerators.size(); ++i )
   {
      if ( m_designGenerators[i].empty() )
      {
         THROW2( InvalidState, "Design generator cannot be empty." );
      }

      // Last element corresponds to the factor that must be aliased
      if ( m_designGenerators[i].back() >= 0 )
      {
         size_t lastElement = m_designGenerators[i].back();
         if ( lastElement > numSelOrdFactors() )
         {
            THROW2( IndexOutOfBounds, "Parameter index out of bounds." );
         }
      }

      // Generator index 1 corresponds to a positive parameter index
      if ( m_designGenerators[i][1] <= 0 )
      {
         THROW2( InvalidState, "Parameter index out of bounds." );
      }

      // Generators must be ordered from low to high
      for ( unsigned int j = 2; j < m_designGenerators[i].size(); ++j )
      {
         if ( m_designGenerators[i][j] <= m_designGenerators[i][j - 1] )
         {
            THROW2( InvalidState, "Design generators must be strictly ordered from low to high." );
         }
      }
   }
}

void FactDesign::getCaseSetImpl( ParameterBounds const& bounds, Case const& center,
                                 bool replicate, vector<Case>& caseSet ) const
{
   fillCaseSet( bounds, center, replicate, caseSet );
}

std::string FactDesign::toString() const
{
   std::stringstream s;
   DblMatrixToString( s, m_design );
   return s.str();
}

} // namespace SUMlib
