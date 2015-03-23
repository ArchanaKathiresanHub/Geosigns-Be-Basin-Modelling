// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#include "CubicProxy.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <limits>
#include <map>
#include <numeric>

#include "DataStructureUtils.h"
#include "Exception.h"
#include "NumericUtils.h"
#include "ProxyBuilder.h"
#include "SerializerUtils.h"

using std::vector;

namespace SUMlib {

namespace
{

unsigned int g_version( 2 );

struct AbsoluteLessThan
{
   AbsoluteLessThan( double const& t) :
      value( t )
   {
      // empty
   }
   bool operator()( double const& val )
   {
      return fabs( val ) < value;
   }
   double value;
};

}

void CubicProxy::calculateCoefficients( int stat, vector<vector<double> > const& a,
                                        vector<double> const& w,
                                        vector<vector<double> > const& v,
                                        vector<double> const& b,
                                        vector<double> & coef )
{
   const size_t nRows( a.size() );
   const size_t nCols( nRows ? a.front().size() : 0 );
   if ( nRows == 0 || nCols == 0 )
   {
      coef.clear();
      return;
   }
   assert( w.size() == nCols );
   assert( v.size() == nCols );
   assert( v.front().size() == nCols );
   assert( b.size() == nRows );

   // Singular values w smaller than tolerance*wmax are discarded
   const double tolerance = nRows * MachineEpsilon();
   const double wmax(*(std::max_element( w.begin(), w.end() ) ) );
   const double threshold( wmax*tolerance );

   // Set diagonal terms to zero for insensitive elements
   vector<double> SV( w );
   std::replace_if( SV.begin(), SV.end(), AbsoluteLessThan(threshold), 0.0 );

   // Backsubstitution to obtain the coefficients
   // coef does not have to be cleared
   coef.resize( nCols );
   if ( stat == 0 )
   {
      svbksb( a, SV, v, b, coef, threshold );

      // Set tiny coefficients to zero. These will be ignored by CubicProxy::getCoefficientsMap(). Coefficients are also
      // calculated in ProxyBuilder::calcAugmentedCoeff and ProxyBuilder::calcReducedCoeff. It is expected that those
      // functions do not generate tiny coefficients so those coefficients are not set to zero.
      std::replace_if( coef.begin(), coef.end(), AbsoluteLessThan( MachineEpsilon() ), 0.0 );
   }
   else
   {
      coef.assign( nCols, 0.0 );
   }
}

unsigned int CubicProxy::numVars( unsigned int nPar, unsigned int maxOrder /* 3 */ )
{
   unsigned int num = 0;

   switch( maxOrder )
   {
      case 9: //special convention: linear + pure quadratic terms
         num = 2*nPar;
         break;
      case 3:
         num = 3*nPar*(nPar+1)/2 + nPar*(nPar-1)*(nPar-2)/6;
         break;
      case 2:
         num = nPar + nPar*(nPar+1)/2;
         break;
      case 1:
         num = nPar;
         break;
      case 0:
         break;
      default:
         THROW2( InvalidValue, "maximum order is 3" );
   }
   return num;
}

void CubicProxy::monomials( Parameter const& Par, RealVector & r )
{
   const size_t nPars(Par.size());
   const size_t nVars(CubicProxy::numVars(nPars));

   size_t j, k, l;

   // clearing is unncessary, as all elements will be overwritten
   r.resize( nVars);

   // Start filling
   size_t i = 0;

   // intercept skipped

   // linear terms
   for ( j = 0; j < nPars; ++j )
   {
      r[i++] = Par[j];
   }
   assert( i == nPars );

   // quadratic terms
   for ( j = 0; j < nPars; ++j )
   {
      const double p = Par[j];
      for ( k = j; k < nPars; ++k )
      {
         r[i++] = p * Par[k];
      }
   }
   assert( i == nPars + nPars*(nPars+1)/2 );

   // cubic terms
   for ( j = 0; j < nPars; ++j )
   {
      const double p = Par[j];
      for ( k = j; k < nPars; ++k )
      {
         const double pp = p * Par[k];
         for ( l = k; l < nPars; ++l )
         {
            r[i++] = pp * Par[l];
         }
      }
   }
   assert( i == nVars );
}

CubicProxy::CubicProxy() :
   m_size(0),
   m_targetMean(0.0),
   m_designMatrixRank(0)
{
   // empty
}

CubicProxy::~CubicProxy()
{
   // empty
}

CubicProxy::CubicProxy(
      ParameterSet const&  parSet,
      TargetSet const&     targetSet,
      IndexList const&     vars
      ) :
   m_size(0),
   m_targetMean(0.0),
   m_designMatrixRank(0)
{
   initialise( parSet, targetSet, vars );
}

CubicProxy::CubicProxy(
      unsigned int                  size,
      VarList const&                vars,
      MonomialKeyList const&        code,
      RealVector const&             proxyMean,
      double                        targetMean,
      RealVector const&             coefficients
      )
{
   initialise( size, vars, code, proxyMean, targetMean, coefficients );
}

void CubicProxy::initialise(
      ParameterSet const&  parSet,
      TargetSet const&     targetSet,
      IndexList const&     vars
      )
{
   m_size = parSet.size() ? parSet.front().size() : 0;
   if ( ! size() )
   {
      THROW2( DimensionOutOfBounds, "Parameter must have at least one element" );
   }
   if ( parSet.size() != targetSet.size() )
   {
      THROW2( DimensionMismatch, "Parameter set and target set must have same number of cases" );
   }
   if ( vars.size() >= parSet.size() )
   {
      THROW2( DimensionOutOfBounds, "Parameter set must have more cases than the number of cubic proxy variables" );
   }
   if ( !vars.empty() && vars.back() >= CubicProxy::numVars( size() ) )
   {
      THROW2( InvalidValue, "Cubic variable index exceeds the number of variables for the parameter size" );
   }

   // Copy results that are needed for proxy function calls
   ProxyBuilder builder( parSet, targetSet, vars );

   initialise( builder.size(), builder.baseVars(), builder.code(), builder.proxyMean(), builder.targetMean(), builder.coefficients() );
}

void CubicProxy::initialise(
      unsigned int                  size,
      VarList const&                vars,
      MonomialKeyList const&        code,
      RealVector const&             proxyMean,
      double                        targetMean,
      RealVector const&             coefficients
      )
{
   unsigned int maxNumVars = CubicProxy::numVars( size );
   assert( vars.size() <= maxNumVars );
   assert( vars.empty() || vars.back() < maxNumVars );
   assert( vars.size() == code.size() );
   assert( vars.size() == coefficients.size() );
   assert( vars.size() == proxyMean.size() );

   m_size             = size;
   m_vars             = vars;
   m_code             = code;
   m_proxyMean        = proxyMean;
   m_targetMean       = targetMean;
   m_coefficients     = coefficients;
   m_stdErrors.assign( coefficients.size() + 1, -1.0 );
   m_designMatrixRank = 0;
}

unsigned int CubicProxy::size() const
{
   return m_size;
}

void CubicProxy::getVarList( IndexList &vars ) const
{
   vars = m_vars;
}

void CubicProxy::getCoefficientsMap( CoefficientsMap& map ) const
{
   map.clear();

   map[ IndexList() ] = std::make_pair( getIntercept(), m_stdErrors[0] );
   for ( size_t i = 0; i < m_code.size(); ++i )
   {
      if ( m_coefficients[i] != 0.0 )
      {
         map[ m_code[i] ] = std::make_pair( m_coefficients[i], m_stdErrors[i+1] );
      }
   }
}

double CubicProxy::getIntercept( void ) const
{
   double value( m_targetMean );

   // subtract coefficients times column means
   for ( size_t i = 0; i < m_code.size(); ++i )
   {
      value -= m_coefficients[i] * m_proxyMean[i];
   }
   return value;
}

double CubicProxy::getValue( Parameter const& par ) const
{
   double value( m_targetMean );

   // calculate active monomials only
   for ( size_t i = 0; i < m_code.size(); ++i )
   {
      value += m_coefficients[i] * ( monomial( m_code[i], par ) - m_proxyMean[i] );
   }
   return value;
}

void CubicProxy::monomial_code( unsigned int nPars, MonomialKeyList& code )
{
   const size_t nVars = CubicProxy::numVars( nPars );

   size_t j, k, l;

   // Start filling
   size_t i = 0;

   code.resize(nVars);

   // linear terms
   for ( j = 0; j < nPars; ++j )
   {
      code[i].resize(1);
      code[i][0] = j;
      ++i;
   }
   assert( i == nPars );

   // quadratic terms
   for ( j = 0; j < nPars; ++j )
   {
      for ( k = j; k < nPars; ++k )
      {
         code[i].resize(2);
         code[i][0] = j;
         code[i][1] = k;
         ++i;
      }
   }
   assert( i == nPars + nPars*(nPars+1)/2 );

   // cubic terms
   for ( j = 0; j < nPars; ++j )
   {
      for ( k = j; k < nPars; ++k )
      {
         for ( l = k; l < nPars; ++l )
         {
            code[i].resize(3);
            code[i][0] = j;
            code[i][1] = k;
            code[i][2] = l;
            ++i;
         }
      }
   }
   assert( i == nVars );
}

void CubicProxy::monomial_code( unsigned int nPars, IndexList const& active, MonomialKeyList& code )
{
   if ( active.empty() || nPars == 0 )
   {
      code.clear();
      return;
   }

   // NOTE: active is assumed to be sorted!

   // Indexes for parameter elements
   size_t j, k, l;

   // Index of all variables
   size_t p = 0;

   // Index of active variables
   size_t i = 0;

   const size_t nVars(active.size());
   code.resize(nVars);

   // linear terms
   for ( j = 0; j < nPars && i < nVars; ++j )
   {
      if ( p == active[i] )
      {
         code[i].resize(1);
         code[i][0] = j;
         ++i;
      }
      ++p;
   }
   assert( i == nVars || p == nPars );

   // quadratic terms
   for ( j = 0; j < nPars; ++j )
   {
      for ( k = j; k < nPars && i < nVars; ++k )
      {
         if ( p == active[i] )
         {
            code[i].resize(2);
            code[i][0] = j;
            code[i][1] = k;
            ++i;
         }
         ++p;
      }
   }
   assert( i == nVars || p == nPars + nPars*(nPars+1)/2 );

   // cubic terms
   for ( j = 0; j < nPars; ++j )
   {
      for ( k = j; k < nPars; ++k )
      {
         for ( l = k; l < nPars && i < nVars; ++l )
         {
            if ( p == active[i] )
            {
               code[i].resize(3);
               code[i][0] = j;
               code[i][1] = k;
               code[i][2] = l;
               ++i;
            }
            ++p;
         }
      }
   }
   assert( i == nVars || p == CubicProxy::numVars(nPars));
}

IndexList CubicProxy::initialVarList(
      unsigned int         nPars,
      unsigned int         order,
      Partition const&     partition )
{
   assert( partition.size() == nPars );
   assert( order < 3 || order == 9 ); //by convention, order = 9 stands for linear + pure quadratic terms

   // start without any vars, i.e. intercept only!
   IndexList vars( 0 ); //no vars if order == 0 (intercept only)

   size_t i = 0;
   size_t j, k;

   // linear terms
   if ( order > 0 )
   {
      for ( j = 0; j < nPars; ++j )
      {
         if ( partition[j] )
         {
            vars.push_back( i );
         }
         ++i;
      }
   }

   // second order terms
   if ( order == 2 || order == 9 )
   {
      for ( j = 0; j < nPars; ++j )
      {
         for ( k = j; k < nPars; ++k )
         {
            if ( partition[j] && partition[k] )
            {
               if ( order == 2 || k == j ) //no interaction terms if order = 9
               {
                  vars.push_back( i );
               }
            }
            ++i;
         }
      }
   }

   return vars;
}

IndexList CubicProxy::initialVarList( unsigned int nPars, unsigned int nOrdPars,
                                      unsigned int order, Partition const& partition )
{
   assert( partition.size() == nPars );
   assert( order < 3 || order == 9 ); //by convention, order = 9 stands for linear + pure quadratic terms
   assert( nOrdPars < nPars );

   IndexList vars;

   unsigned int i = 0;
   unsigned int j, k, l;

   // linear terms, including terms to be interpreted as constants (linear in a dummy parameter)
   for ( j = 0; j < nPars; ++j )
   {
      if ( partition[j] && ( ( order > 0 ) || ( j >= nOrdPars ) ) )
      {
         vars.push_back( i );
      }
      ++i;
   }

   if ( nOrdPars ) //otherwise too many (false) intercepts are added that cannot be matched by the number of cases anyway
   {

      // second-order terms, including terms to be interpreted as being lower order
      for ( j = 0; j < nPars; ++j )
      {
         for ( k = j; k < nPars; ++k )
         {
            if ( partition[j] && partition[k] && validOrder2Var( order, nOrdPars, k, j ) )
            {
               vars.push_back( i );
            }
            ++i;
         }
      }

      // third-order terms, including terms to be interpreted as being lower order
      for ( j = 0; j < nPars; ++j )
      {
         for ( k = j; k < nPars; ++k )
         {
            for ( l = k; l < nPars; ++l )
            {
               if ( partition[j] && partition[k] && partition[l] && validOrder3Var( order, nOrdPars, l, k, j ) )
               {
                  vars.push_back( i );
               }
               ++i;
            }
         }
      }

   }

   return vars;
}

bool CubicProxy::validOrder2Var( unsigned int order, unsigned int nOrds, unsigned int k, unsigned int j )
{
   assert( k >= j );
   bool validTerm, validOrder0, validOrder1, validOrder2, validOrder9;

   validOrder0 = ( order == 0 ) && ( k > j ) && ( j >= nOrds );
   validOrder1 = ( order == 1 ) && ( k > j ) && ( k >= nOrds );
   validOrder2 = ( order == 2 ) && !( k == j && k >= nOrds );
   validOrder9 = ( order == 9 ) && !( k == j && k >= nOrds ) && !( k > j && k < nOrds );

   validTerm = ( validOrder0 || validOrder1 || validOrder2 || validOrder9 );
   return validTerm;
}

bool CubicProxy::validOrder3Var( unsigned int order, unsigned int nOrds,
                                 unsigned int l, unsigned int k, unsigned int j )
{
   assert( ( l >= k ) && ( k >= j ) );
   bool validTerm, validOrder0, validOrder1, validOrder2, validOrder9;

   validOrder0 = ( order == 0 ) && ( l > k ) && ( k > j ) && ( j >= nOrds );
   validOrder1 = ( order == 1 ) && ( l > k ) && ( k > j ) && ( k >= nOrds );
   validOrder2 = ( order == 2 ) && ( l > k ) && ( l >= nOrds ) && !( k == j && k >= nOrds );
   validOrder9 = ( order == 9 ) && ( l > k ) && ( l >= nOrds ) && !( k == j && k >= nOrds ) && !( k > j && k < nOrds );

   validTerm = ( validOrder0 || validOrder1 || validOrder2 || validOrder9 );
   return validTerm;
}


void CubicProxy::setStdErrors( RealVector const& stdErrors )
{
   assert( stdErrors.size() == m_coefficients.size() + 1 );
   m_stdErrors = stdErrors;
}


void CubicProxy::setDesignMatrixRank( unsigned int rank )
{
   m_designMatrixRank = rank;
}


unsigned int CubicProxy::getDesignMatrixRank() const
{
   return m_designMatrixRank;
}


bool CubicProxy::isRegressionIllPosed() const
{
   // The number of polynomial terms is less than or equal to the number of cases, so for the maximum rank it suffices
   // to look only at the number of terms.
   const unsigned int maxRank = m_coefficients.size() + 1;
   return m_designMatrixRank < maxRank;
}


bool CubicProxy::load( IDeserializer* deserializer, unsigned int version )
{
   bool           ok(true);

   ok = ok && deserialize(deserializer, m_vars);

   unsigned int   nrOfIndexLists(0);
   ok = ok && deserialize(deserializer, nrOfIndexLists);

   if (ok)
   {
      m_code.resize(nrOfIndexLists);

      // for all indexLists
      for (size_t i(0); ok && i < nrOfIndexLists; ++i)
      {
         ok = ok && deserialize( deserializer, m_code[i]);
      } // for all indexLists
   }

   ok = ok && deserialize(deserializer,m_proxyMean);
   ok = ok && deserialize(deserializer,m_targetMean);
   ok = ok && deserialize(deserializer,m_coefficients);
   if ( version >= 1 )
   {
      ok = ok && deserialize(deserializer,m_stdErrors);
   }
   else
   {
      m_stdErrors.assign( m_coefficients.size() + 1, -1.0 );
   }
   ok = ok && deserialize(deserializer,m_size);
   if ( version >= 2 )
   {
      ok = ok && deserialize(deserializer,m_designMatrixRank);
   }
   else
   {
      m_designMatrixRank = 0;
   }

   return ok;
} // CubicProxy::load()


bool CubicProxy::save( ISerializer* serializer, unsigned int version ) const
{
   assert( version == g_version );
   bool  ok(true);

   ok = ok && serialize(serializer, m_vars);
   ok = ok && serialize(serializer, (unsigned int)m_code.size());

   // for all indexLists
   for (size_t i(0); ok && i < m_code.size(); ++i)
   {
      ok = ok && serialize( serializer, m_code[i]);
   } // for all indexLists

   ok = ok && serialize(serializer, m_proxyMean);
   ok = ok && serialize(serializer, m_targetMean);
   ok = ok && serialize(serializer, m_coefficients);
   ok = ok && serialize(serializer, m_stdErrors);
   ok = ok && serialize(serializer, m_size);
   ok = ok && serialize(serializer, m_designMatrixRank);

   return ok;
} // CubicProxy::save()


unsigned int CubicProxy::getSerializationVersion() const
{
   return g_version;
}


} // namespace SUMlib
