// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#include <algorithm>
//#include <vector>
#include <cassert>

#include "Exception.h"
#include "ExpDesignBase.h"
#include "ParameterBounds.h"
#include "SUMlib.h"

using std::vector;

namespace SUMlib {

ExpDesignBase::ExpDesignBase( vector<bool> const& selectedFactors, unsigned int numOrdinalFactors ) :
   m_numFactors( selectedFactors.size() ),
   m_numOrdFactors( numOrdinalFactors ),
   m_selFactors( selectedFactors ),
   m_numSelFactors( unsigned( std::count( selectedFactors.begin(), selectedFactors.end(), true )) ),
   m_numSelOrdFactors( 0 )
{
   if ( numOrdFactors() > numFactors() )
   {
      THROW2( ValueOutOfBounds, "ExpDesignBase: number of ordinal factors cannot exceed total number of factors" );
   }
   if ( numSelFactors() < 1 )
   {
      THROW2( ValueOutOfBounds, "ExpDesignBase: number of factors must be > 0" );
   }
   for ( unsigned int i = 0; i < numOrdFactors(); ++i )
   {
      if ( selFactors()[i] ) m_numSelOrdFactors++;
   }
}

ExpDesignBase::~ExpDesignBase()
{
}

unsigned int ExpDesignBase::numFactors() const
{
   return m_numFactors;
}

unsigned int ExpDesignBase::numSelFactors() const
{
   return m_numSelFactors;
}

unsigned int ExpDesignBase::numOrdFactors() const
{
   return m_numOrdFactors;
}

unsigned int ExpDesignBase::numSelOrdFactors() const
{
   return m_numSelOrdFactors;
}

std::vector<bool> const& ExpDesignBase::selFactors() const
{
   return m_selFactors;
}

void ExpDesignBase::setAddCenterPoint( bool /* addCenterPoint */ )
{
   throw "undefined operation";
}

bool ExpDesignBase::getAddCenterPoint( ) const
{
   throw "undefined operation";
}

void ExpDesignBase::setAddMirror( bool /* addMirror */ )
{
   throw "undefined operation";
}

bool ExpDesignBase::getAddMirror( ) const
{
   throw "undefined operation";
}

unsigned int ExpDesignBase::getNbOfCases( ParameterBounds const& bounds, bool replicate ) const
{
   // Check number of categorical parameters
   unsigned int numCatFactors = bounds.sizeCat();
   if ( numFactors() - numOrdFactors() != numCatFactors )
   {
      THROW2( DimensionMismatch, "Supplied number of categorical parameters does not match." );
   }

   // Count number of categorical combinations
   unsigned int nbOfCases = 1;
   for ( unsigned int i = 0; i < numCatFactors; ++i )
   {
      if ( selFactors()[i + numOrdFactors()] )
      {
         nbOfCases *= bounds.catValues( i ).size();
      }
   }

   // Return total number of cases
   if ( replicate ) nbOfCases *= m_design.size();
   else nbOfCases += m_design.size() - 1;
   return nbOfCases;
}

void ExpDesignBase::expandDesign( unsigned int n, DesignMatrix const& designIn, DesignMatrix& designOut )
{
   designOut.clear();
   designOut.resize( designIn.size() );
   for ( unsigned int i = 0; i < designOut.size(); ++i )
   {
      if ( n < designIn[i].size() )
      {
         THROW2( DimensionMismatch, "Number of selected parameters exceeds total number of parameters." );
      }
      unsigned int parIndex = 0;
      designOut[i].resize( n, 0.0 );
      for ( unsigned int j = 0; j < n; ++j )
      {
         if ( selFactors()[j] )
         {
            designOut[i][j] = designIn[i][parIndex];
            parIndex++;
         }
      }
   }

   // If either all parameters are categorical or if all ordinal parameters have been
   // frozen (i.e. unselected), designOut may be empty. Either way, designOut
   // needs an ordinal base case as starting point for the categorical extension.
   if ( designOut.empty() )
   {
      designOut.push_back( vector<double>( n, 0.0 ) );
   }
   // Now always designOut.size() > 0 holds !
}

void ExpDesignBase::getCaseSet( ParameterBounds const& bounds, Case const& center,
                                bool replicate, vector<Case>& caseSet ) const
{
   checkBoundsAndCenter( bounds, center );
   getCaseSetImpl( bounds, center, replicate, caseSet );
}

void ExpDesignBase::fillCaseSet( ParameterBounds const& bounds, Case const& center,
                                 bool replicate, vector<Case>& caseSet ) const
{
   checkDesignDimensions( numOrdFactors(), m_design );

   // Gather all subdesigns associated with the different parameter types
   DesignMatrix conDesign;
   vector<vector<int> > disDesign;
   createOrdDesign( bounds, center, conDesign, disDesign );
   vector<vector<unsigned int> > catDesign;
   calcCatCombinations( bounds, catDesign );

   assert( conDesign.size() == disDesign.size() );
   assert( catDesign.size() > 0 );

   // Assemble the case set from the subdesigns
   caseSet.clear();
   if ( replicate ) //replicate ordinal design for each categorical combination
   {
      caseSet.resize( m_design.size() * catDesign.size() );
      for ( unsigned int jcase = 0; jcase < catDesign.size(); ++jcase )
      {
         for ( unsigned int icase = 0; icase < m_design.size(); ++icase )
         {
            Case c( conDesign[icase], disDesign[icase], catDesign[jcase] );
            caseSet[ jcase * m_design.size() + icase ] = c;
         }
      }
   }
   else //if ( !replicate )
   {
      caseSet.resize( m_design.size() + catDesign.size() - 1 );

      // Combine ordinal design only with categorical base/default case
      unsigned int icase;
      for ( icase = 0; icase < m_design.size(); ++icase )
      {
         Case c( conDesign[icase], disDesign[icase], center.categoricalPart() );
         caseSet[icase] = c;
      }

      // Combine ordinal base case with remaining (non-default) categorical combinations
      for ( unsigned int i = 0; i < catDesign.size(); ++i )
      {
         if ( catDesign[i] != center.categoricalPart() )
         {
            Case c( center.continuousPart(), center.discretePart(), catDesign[i] );
            caseSet[icase++] = c;
         }
      }
      assert( icase == m_design.size() + catDesign.size() - 1 );
   }
}

void ExpDesignBase::fillCaseSet( ParameterBounds const& bounds, Case const& center,
                                 vector<Case>& caseSet ) const
{
   checkDesignDimensions( numFactors(), m_design );

   // Gather all subdesigns associated with the different parameter types
   DesignMatrix conDesign;
   vector<vector<int> > disDesign;
   createOrdDesign( bounds, center, conDesign, disDesign );
   vector<vector<unsigned int> > catDesign;
   createCatDesign( bounds, catDesign );

   assert( conDesign.size() == disDesign.size() && disDesign.size() == catDesign.size() );

   // Combine the three subdesigns
   caseSet.resize( m_design.size() );
   for ( unsigned int icase = 0; icase < m_design.size(); ++icase )
   {
      Case c( conDesign[icase], disDesign[icase], catDesign[icase] );
      caseSet[icase] = c;
   }
}

void ExpDesignBase::createOrdDesign( ParameterBounds const& bounds, Case const& center,
                    DesignMatrix& conDes, vector<vector<int> >& disDes ) const
{
   unsigned int numConFactors = center.sizeCon();
   unsigned int numDisFactors = numOrdFactors() - numConFactors;
   conDes.assign( m_design.size(), vector<double>( numConFactors ) );
   disDes.assign( m_design.size(), vector<int>( numDisFactors ) );

   vector<double> dpNeg( numOrdFactors() ), dpPos( numOrdFactors() );
   for ( unsigned int i = 0; i < numOrdFactors(); ++i )
   {
      dpNeg[i] = center.ordinalPar( i ) - bounds.low().ordinalPar( i );
      dpPos[i] = bounds.high().ordinalPar( i ) - center.ordinalPar( i );
   }

   // Loop over the cases.
   for ( unsigned int icase = 0; icase < m_design.size(); ++icase )
   {
      // Loop over the continuous parameters.
      for ( unsigned int i = 0; i < numConFactors; ++i )
      {
         // Set parameter as a shift from the center.
         if ( m_design[icase][i] < 0 )
         {
            conDes[icase][i] = center.ordinalPar( i ) + m_design[icase][i] * dpNeg[i];

            // Make sure the parameter >= min. It can only be < min due to a round-off error.
            conDes[icase][i] = std::max( conDes[icase][i], bounds.low().ordinalPar( i ) );
         }
         else
         {
            conDes[icase][i] = center.ordinalPar( i ) + m_design[icase][i] * dpPos[i];

            // Make sure the parameter <= max. It can only be > max due to a round-off error.
            conDes[icase][i] = std::min( conDes[icase][i], bounds.high().ordinalPar( i ) );
         }
      }
      // Loop over the discrete parameters.
      for ( unsigned int i = numConFactors; i < numOrdFactors(); ++i )
      {
         unsigned int disIdx = i - numConFactors;
         double val;
         // Set parameter as a shift from the center.
         if ( m_design[icase][i] < 0 )
         {
            val = center.ordinalPar( i ) + m_design[icase][i] * dpNeg[i];
         }
         else
         {
            val = center.ordinalPar( i ) + m_design[icase][i] * dpPos[i];
         }
         disDes[icase][disIdx] = convertOrdToDis( bounds, i, val );
      }
   }
}

void ExpDesignBase::calcCatCombinations( ParameterBounds const& bounds,
                                         vector<vector<unsigned int> >& catCombiSet ) const
{
   catCombiSet.clear();
   unsigned int numCatFactors = bounds.sizeCat();
   if ( numCatFactors == 0 )
   {
      catCombiSet.assign( 1, vector<unsigned int>() ); //assign a single empty vector
      return; //nothing to be done anymore
   }
   vector<vector<unsigned int> > catValues = bounds.catValues(); //retrieve categorical values

   // An index for each categorical parameter is needed to find all categorical combinations.
   // A corresponding maxIndex is needed to know how many values are stored in catValues.
   IndexList maxIndex( numCatFactors, 0 );
   for ( unsigned int i = 0; i < numCatFactors; ++i )
   {
      if ( selFactors()[i + numOrdFactors()] )
      {
         maxIndex[i] = catValues[i].size() - 1;
      }
      //else 0 because ith categorical par has been frozen at first value (i.e. base value)
   }
   IndexList index( numCatFactors, 0 ); //index[i] <= maxIndex[i] is index for values of ith par

   // Find all categorical combinations
   vector<unsigned int> combi( numCatFactors ); //container for a categorical combination
   bool allCombinations = false;
   while ( !allCombinations )
   {
      // Store combination as determined by current index
      for ( unsigned int i = 0; i < numCatFactors; ++i )
      {
         combi[i] = catValues[i][index[i]];
      }
      catCombiSet.push_back( combi );

      // Update index for the next combination
      for ( unsigned int i = 0; i < numCatFactors; ++i )
      {
         if ( index[i] < maxIndex[i] )
         {
            index[i]++;
            break;
         }
         else
         {
            index[i] = 0;
         }
      }

      // All combinations have been handled if we are back where we started,
      // i.e. if index[i] = 0 for all i.
      for ( unsigned int i = 0; i < numCatFactors; ++i )
      {
         if ( index[i] > 0 ) break;
         if ( i == numCatFactors - 1 ) allCombinations = true;
      }
   }
}

int ExpDesignBase::convertOrdToDis( ParameterBounds const& bounds, unsigned int idx, double value ) const
{
   if ( idx < bounds.sizeCon() || idx >= bounds.sizeOrd() )
   {
      THROW2( DimensionMismatch, "Index must correspond to a discrete parameter." );
   }
   unsigned int disIdx = idx - bounds.sizeCon();
   int disValue = bounds.low().discretePar( disIdx );
   if ( bounds.low().isEqualTo( idx, bounds.high() ) )
   {
      return disValue; //is equal to center value
   }
   double subRange = ( value - bounds.low().ordinalPar( idx ) ) / bounds.rangeOrd( idx );
   subRange *= ( bounds.rangeOrd( idx ) + 1 );
   disValue += int( subRange );
   if ( disValue > bounds.high().discretePar( disIdx ) ) disValue--;

   // Last check
   if ( disValue < bounds.low().discretePar( disIdx ) || disValue > bounds.high().discretePar( disIdx ) )
   {
      THROW2( InvalidValue, "Parameter value is out of bounds." );
   }

   return disValue;
}

void ExpDesignBase::createCatDesign( ParameterBounds const& bounds,
                                     vector<vector<unsigned int> >& catDes ) const
{
   unsigned int numCatFactors = bounds.sizeCat();
   catDes.assign( m_design.size(), vector<unsigned int>( numCatFactors ) );
   vector<IndexList> catValues = bounds.catValues(); //retrieve categorical values

   // For each categorical parameter, the index referring to its values starts at zero.
   // The maximum index is determined below. It is zero if the corresponding categorical
   // parameter has been frozen (at the first value = base value).
   vector<unsigned int> maxIdx( numCatFactors, 0 );
   for ( unsigned int i = 0; i < numCatFactors; ++i )
   {
      if ( selFactors()[i + numOrdFactors()] )
      {
         maxIdx[i] = catValues[i].size() - 1;
      }
   }

   // Loop over the cases.
   for ( unsigned int icase = 0; icase < m_design.size(); ++icase )
   {
      // Loop over the categorical parameters.
      for ( unsigned int i = 0; i < numCatFactors; ++i )
      {
         // Scale design value to range [ 0 : maxIdx[i] + 1 ].
         unsigned int parIdx = i + numOrdFactors();
         double dblVal = ( maxIdx[i] + 1.0 ) * ( 1.0 + m_design[icase][parIdx] ) / 2.0;

         // Set index idx referring to appropriate categorical value.
         unsigned int idx = static_cast<unsigned int>( dblVal );
         if ( idx > maxIdx[i] ) idx--; //this precaution should not be necessary

         catDes[icase][i] = catValues[i][idx];
      }
   }
}

void ExpDesignBase::checkDesignDimensions( unsigned int n, DesignMatrix const& design ) const
{
   for ( unsigned int i = 0; i < design.size(); ++i )
   {
      if ( design[i].size() != n )
      {
         THROW2( DimensionMismatch, "Design cases have wrong dimension." );
      }
   }
}

void ExpDesignBase::checkBoundsAndCenter( ParameterBounds const& bounds, Case const& center ) const
{
   if ( ! center.isComparableTo( bounds.low() ) )
   {
      THROW2( DimensionMismatch, "bounds.low and center" );
   }
   if ( ! center.isComparableTo( bounds.high() ) )
   {
      THROW2( DimensionMismatch, "bounds.high and center" );
   }
   if ( center.size() != numFactors() )
   {
      THROW2( DimensionMismatch, "Supplied number of parameters does not match." );
   }
   if ( center.sizeOrd() != numOrdFactors() )
   {
      THROW2( DimensionMismatch, "Supplied number of ordinal parameters does not match." );
   }
   for ( unsigned int i = 0; i < center.sizeOrd(); ++i )
   {
      if ( bounds.high().isSmallerThan( i, center ) || center.isSmallerThan( i, bounds.low() ) )
      {
         THROW2( InvalidValue, "Ordinal part of center is out of parameter bounds." );
      }
   }
}

} // SUMlib
