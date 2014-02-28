// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#include <algorithm>
#include <cassert>
#include <map>
#include <string>
#include <sstream>
#include <vector>

#include "DataStructureUtils.h"
#include "Exception.h"
#include "NumericUtils.h"
#include "PolynomialProxyModel.h"

using std::vector;
using std::string;
using std::stringstream;
using std::map;

namespace SUMlib {


// A normalization constant for the equations
static const double sigma = 1.0;

static map<PolynomialProxyModel::Type, string> s_typeText;
static map<string, PolynomialProxyModel::Type> s_types;

static bool s_initialised   = false;

static void initialise()
{
   s_typeText[PolynomialProxyModel::FULL_QUADRATIC]    = "Linear + Quadratic + Interaction";
   s_typeText[PolynomialProxyModel::LINEAR_CROSSTERMS] = "Linear + Interaction";
   s_typeText[PolynomialProxyModel::LINEAR_QUADRATIC]  = "Linear + Quadratic";
   s_typeText[PolynomialProxyModel::LINEAR]            = "Linear";

   for (map<PolynomialProxyModel::Type, string>::const_iterator it = s_typeText.begin(); it != s_typeText.end(); it++)
      s_types[it->second] = it->first;

   s_initialised = true;
}

PolynomialProxyModel::PolynomialProxyModel( bool quadratic /* false */, bool interaction /* false */ ) :
   m_quadratic( quadratic ),
   m_interaction( interaction ),
   m_constCoefficient(),
   m_firstOrderCoefficients(),
   m_firstOrderIndexes(),
   m_secondOrderCoefficients(),
   m_secondOrderIndexes1(),
   m_secondOrderIndexes2()
{
}

PolynomialProxyModel::PolynomialProxyModel( const Type type ) :
   m_quadratic( type == FULL_QUADRATIC || type == LINEAR_QUADRATIC ),
   m_interaction( type == FULL_QUADRATIC || type == LINEAR_CROSSTERMS ),
   m_constCoefficient(),
   m_firstOrderCoefficients(),
   m_firstOrderIndexes(),
   m_secondOrderCoefficients(),
   m_secondOrderIndexes1(),
   m_secondOrderIndexes2()
{
}

PolynomialProxyModel::~PolynomialProxyModel()
{
}

PolynomialProxyModel::Type PolynomialProxyModel::determineType( int nrOfParameters, int nrOfRuns )
{
   Type      type(FULL_QUADRATIC);
   bool      interaction = nrOfParameters > 1 ? true : false;
   bool      quadratic = true;

   if (nrOfParameters == 1 && nrOfRuns < 3)
   {
      quadratic = false;
   }
   else if (nrOfParameters == 2)
   {
      if (nrOfRuns == 5)
      {
         interaction = false;
      }
      else if (nrOfRuns == 4)
      {
         quadratic = false;
      }
      else if (nrOfRuns < 4)
      {
         interaction = false;
         quadratic = false;
      }
   }
   else if (nrOfParameters == 3)
   {
      if (nrOfRuns < 10)
         interaction = false;

      if (nrOfRuns < 7)
         quadratic = false;
   }
   else if (nrOfParameters > 3)
   {
      if (nrOfRuns < (1 + 2 * nrOfParameters))
      {
         interaction = false;
         quadratic = false;
      }
      else if (nrOfRuns < 1 + nrOfParameters * (nrOfParameters + 1) / 2)
         interaction = false;
      else if (nrOfRuns < 1 + nrOfParameters * (nrOfParameters + 3) / 2)
         quadratic = false;
   }

   if (interaction && !quadratic)
      type = LINEAR_CROSSTERMS;
   else if (!interaction && quadratic)
      type = LINEAR_QUADRATIC;
   else if (!interaction && !quadratic)
      type = LINEAR;

   return type;
}

const string& PolynomialProxyModel::getTypeText( Type type )
{
   if (!s_initialised)
   {
      SUMlib::initialise();
   }

   return s_typeText[type];
}

PolynomialProxyModel::Type PolynomialProxyModel::getType( const string& typeText )
{
   if (!s_initialised)
   {
      SUMlib::initialise();
   }

   map<string, PolynomialProxyModel::Type>::const_iterator   it = s_types.find(typeText);

   if (it == s_types.end())
      throw "PolynomialProxyModel::getType(): typeText (" + typeText + ") not found";

   return it->second;
}

const map<string, PolynomialProxyModel::Type>& PolynomialProxyModel::getTypes()
{
   if (!s_initialised)
   {
      SUMlib::initialise();
   }

   return s_types;
}

void PolynomialProxyModel::initialise( bool quadratic, bool interaction )
{
   m_quadratic = quadratic;
   m_interaction = interaction;
}

void PolynomialProxyModel::initialise( const Type type )
{
   m_quadratic = ( type == FULL_QUADRATIC || type == LINEAR_QUADRATIC );
   m_interaction = ( type == FULL_QUADRATIC || type == LINEAR_CROSSTERMS );
}

bool PolynomialProxyModel::hasQuadraticTerms( ) const
{
   return m_quadratic;
}

bool PolynomialProxyModel::hasInteractionTerms( ) const
{
   return m_interaction;
}

void PolynomialProxyModel::calculateProxyCoefficients(
      ParameterData const& pMin,
      ParameterData const& pMax,
      ParameterSet const& parameterSet,
      TargetSet const& target )
{
   const unsigned int numRuns = target.size();

   // Check preconditions
   if ( numRuns != parameterSet.size() )
   {
      THROW2( DimensionMismatch, "parameter set and target set sizes do not match" );
   }

   if ( numRuns == 0 )
   {
      THROW2( DimensionOutOfBounds, "target set cannot be empty" );
   }

   const unsigned int tupleSize = parameterSet[0].size();

   // [ABU] I think at least tupleSize * (tupleSize+1) / 2 runs are required.
   if ( tupleSize >= numRuns )
   {
      THROW2( DimensionOutOfBounds, "More runs than parameters required to calculate proxy coefficients" );
   }

   unsigned int iset;
   for ( iset = 0; iset < numRuns; ++iset )
   {
      if( parameterSet[iset].size() != tupleSize )
      {
         THROW2( DimensionMismatch, "All parameter tuples must have the same size" );
      }
   }

   // Preconditions met

   // Matrix and right hand side
   std::vector<std::vector<double> > a( numRuns );
   std::vector<double> b( numRuns );

   // Parameter ranges pRange = pMax - pMin
   std::vector<double> pRange( pMax );
   for ( unsigned int i = 0; i < pRange.size(); ++i )
      pRange[i] -= pMin[i];

   for ( iset = 0; iset < numRuns; ++iset )
   {
      // The right hand side
      b[iset] = target[iset] / sigma;

      // The design matrix row
      calculateRow( pMin, pRange, parameterSet[iset], a[iset] );
   }

   // Note: The calculations below assume that all runs have equal
   // amounts of data, but that assumption is not checked.

   // Check if there is sufficient data to perform the match
   if ( a[0].size() > a.size() )
      THROW2( UnequalSize, "Proxy creation requires more runs than parameters" );

   // Perform SVD
   std::vector<double> singVal;
   std::vector<std::vector<double> > sqOrth;
   int stat = Proxy::calculateSVD( a, singVal, sqOrth );

   std::vector<double> coef;
   Proxy::calculateCoefficients( stat, a, singVal, sqOrth, b, coef );

   m_constCoefficient = coef[0];

   m_firstOrderCoefficients.reserve( tupleSize );
   m_firstOrderIndexes.reserve( tupleSize );
   for ( size_t i = 0; i < tupleSize; ++i )
   {
      m_firstOrderIndexes.push_back( i );
      double c = coef[i+1]/pRange[i];
      m_firstOrderCoefficients.push_back( c );
      m_constCoefficient -= c*pMin[i];
   }

   size_t k = tupleSize+1;

   m_secondOrderIndexes1.reserve( coef.size() - k );
   m_secondOrderIndexes2.reserve( coef.size() - k );
   m_secondOrderCoefficients.reserve( coef.size() - k );

   if ( m_interaction )
   {
      for ( size_t i = 0; i < tupleSize; ++i )
      {
         for ( size_t j = i+(m_quadratic?0:1); j < tupleSize; ++j )
         {
            m_secondOrderIndexes1.push_back( i );
            m_secondOrderIndexes2.push_back( j );
            double c = coef[k++]/(pRange[i]*pRange[j]);
            m_secondOrderCoefficients.push_back( c );
            m_firstOrderCoefficients[i] -= c*pMin[j];
            m_firstOrderCoefficients[j] -= c*pMin[i];
            m_constCoefficient += c*pMin[i]*pMin[j];
         }
      }
   }
   else if ( m_quadratic )
   {
      for ( size_t i = 0; i < tupleSize; ++i )
      {
         m_secondOrderIndexes1.push_back( i );
         m_secondOrderIndexes2.push_back( i );
         double c = coef[k++]/(pRange[i]*pRange[i]);
         m_secondOrderCoefficients.push_back( c );
         m_firstOrderCoefficients[i] -= 2*c*pMin[i];
         m_constCoefficient += c*pMin[i]*pMin[i];
      }
   }
}

void PolynomialProxyModel::calculateRow( std::vector<double> const& pMin, std::vector<double> const& pRange,
      std::vector<double> const& values, std::vector<double>& row ) const
{
   assert( row.empty() );

   unsigned int size = values.size();

   // Constant term
   row.push_back( 1/sigma );

   // The first order terms
   std::vector<double> scaledValues( values.size() );
   for ( unsigned int i = 0; i < size; ++i )
   {
      scaledValues[i] = (values[i] - pMin[i])/pRange[i]; //scale parameters between 0 and 1
      row.push_back( scaledValues[i] / sigma );
   }

   // The second order terms
   if ( m_interaction )
   {
      for ( unsigned int i = 0; i < size; ++i )
      {
         for ( unsigned int j = i+(m_quadratic?0:1); j < size; ++j )
         {
            row.push_back( scaledValues[i] * scaledValues[j] / sigma );
         }
      }
   }
   else if ( m_quadratic )
   {
      for ( unsigned int i = 0; i < size; ++i )
      {
         row.push_back( scaledValues[i] * scaledValues[i] / sigma );
      }
   }
}

unsigned int PolynomialProxyModel::size( ) const
{
   return m_firstOrderCoefficients.size();
}

double PolynomialProxyModel::getProxyValue( Parameter const& parameter, KrigingType ) const
{
   if ( parameter.size() != size() )
   {
      THROW2( DimensionMismatch, "parameter size != coefficients array size" );
   }

   // The constant term
   double result = m_constCoefficient;

   // The first order terms
   for ( size_t i = 0; i < m_firstOrderCoefficients.size(); ++i )
      result += parameter[ m_firstOrderIndexes[i] ] * m_firstOrderCoefficients[i];

   // The second order terms
   for ( size_t i = 0; i < m_secondOrderCoefficients.size(); ++i )
      result += parameter[ m_secondOrderIndexes1[i] ]
                  * parameter[ m_secondOrderIndexes2[i] ]
                  * m_secondOrderCoefficients[i];

   return result;
}


void PolynomialProxyModel::getResponseMatrixRow( std::vector<double> const& cp, double stdDev, std::vector<double>& row ) const
{
   // A single row of the linearised equation matrix

   // Linear coefficients of the ED response model (RM)
   size_t i;
   for ( i = 0; i < m_firstOrderCoefficients.size(); ++i )
   {
      row[m_firstOrderIndexes[i]] = m_firstOrderCoefficients[i]/stdDev;
   }

   const double dblStdDev= 2*stdDev;

   // Product and quadratic coefficients, linearized around cp:
   // RM=...+coef2[0]*cp[0]*p[0]+coef2[1]*(cp[1]*p[0]+cp[0]*p[1])/2+...
   for ( i = 0; i < m_secondOrderCoefficients.size(); ++i )
   {
      row[m_secondOrderIndexes1[i]] += m_secondOrderCoefficients[i] * cp[m_secondOrderIndexes2[i]]/dblStdDev;
      row[m_secondOrderIndexes2[i]] += m_secondOrderCoefficients[i] * cp[m_secondOrderIndexes1[i]]/dblStdDev;
   }
}

double PolynomialProxyModel::getConstCoefficient() const
{
   return m_constCoefficient;
}

const vector<double>& PolynomialProxyModel::getFirstOrderCoefficients() const
{
   return m_firstOrderCoefficients;
}

const vector<double>& PolynomialProxyModel::getSecondOrderCoefficients() const
{
   return m_secondOrderCoefficients;
}


std::string PolynomialProxyModel::toString() const
{
   std::stringstream s;
   s << "{";
   s << "[" << m_constCoefficient << "]";
   s << m_firstOrderCoefficients;
   s << m_secondOrderCoefficients;
   s << "}";

   return s.str();
}

// Required for SUM
void PolynomialProxyModel::setConstCoefficient( double coefficient )
{
   m_constCoefficient = coefficient;
}

// Required for SUM
void PolynomialProxyModel::addFirstOrderTerm( double coefficient, unsigned int index )
{
   m_firstOrderCoefficients.push_back( coefficient );
   m_firstOrderIndexes.push_back( index );
}

// Required for SUM
void PolynomialProxyModel::addSecondOrderTerm( double coefficient, unsigned int index1,
      unsigned int index2 )
{
   m_secondOrderCoefficients.push_back( coefficient );
   m_secondOrderIndexes1.push_back( index1 );
   m_secondOrderIndexes2.push_back( index2 );
}

} // namespace SUMlib
