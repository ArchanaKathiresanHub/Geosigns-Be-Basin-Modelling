// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.


#include <algorithm>
#include <cassert>
#include <cmath>
#include <functional>
#include <numeric>
#include <vector>

#include "Exception.h"
#include "NumericUtils.h"
#include "ParameterPdf.h"
#include "SerializerUtils.h"

using std::vector;

namespace SUMlib {

ParameterPdf::ParameterPdf()
{
}

ParameterPdf::ParameterPdf(
         ParameterSpace const& space, Case const& minLow, Case const& maxHigh,
         Case const& base       /* Case() */,
         RealMatrix const& covariance /* RealMatrix() */,
         RealMatrix const& disWeights /* RealMatrix() */,
         RealMatrix const& catWeights /* RealMatrix() */ )
{
   initialise( space, minLow, maxHigh, base, covariance, disWeights, catWeights );
}

ParameterPdf::~ParameterPdf()
{
   // empty
}

void ParameterPdf::initialise(
      ParameterSpace const& space, Case const& minLow, Case const& maxHigh,
      Case const& base, RealMatrix const& covariance,
      RealMatrix const& disWeights, RealMatrix const& catWeights )
{
   m_minLow = minLow;
   m_maxHigh = maxHigh;
   ParameterSpace::TransformationSet tr = space.transformationSet();
   vector<double> bounds = m_minLow.continuousPart();
   ParameterSpace::transform( tr, bounds );
   m_minLow.setContinuousPart( bounds );
   bounds = m_maxHigh.continuousPart();
   ParameterSpace::transform( tr, bounds );
   m_maxHigh.setContinuousPart( bounds );

   m_low = space.transformedLow();
   m_high = space.transformedHigh();
   m_catValues = space.catValues();
   m_varParIdx = space.nonFixedParIdx();

   m_categoricalBase = base.categoricalPart();
   m_discreteBase.resize( base.sizeDis() );
   for ( unsigned int i = 0; i < base.sizeDis(); ++i )
   {
      m_discreteBase[i] = base.ordinalPar( i + base.sizeCon() );
   }
   m_mostLikely = base.continuousPart();
   m_covariance = covariance;
   m_disWeights = disWeights;
   m_catWeights = catWeights;

   checkBounds();
   setDefaultsIfUnknownPDF();
   checkDimensions();
   checkOrdinalBase();
   removeRedundantCatWeights();
   removeFixedElements();
   checkWeights();
   checkCategoricalBase();
   checkCovarianceMatrix( m_covariance );
   calcMinStddev();
}

void ParameterPdf::checkBounds() const
{
   if ( !m_minLow.isComparableTo( m_maxHigh ) || !m_minLow.isComparableTo( m_low ) )
   {
      THROW2( DimensionMismatch, "ParameterPdf::bounds" );
   }
   for ( unsigned int i = 0; i < size(); ++i )
   {
      if ( i >= sizeOrd() && m_minLow.categoricalPar( i - sizeOrd() ) != 0 )
      {
         THROW2( InvalidValue, "Lowest possible categorical value must be zero by convention" );
      }
      if ( m_low.isSmallerThan( i, m_minLow ) || m_maxHigh.isSmallerThan( i, m_high ) )
      {
         THROW2( InvalidValue, "Current bounds are not contained in maximum space" );
      }
   }
}

void ParameterPdf::setDefaultsIfUnknownPDF()
{
   // Continuous PDF
   if ( sizeCon() > 0 && ( m_mostLikely.empty() || m_covariance.empty() ) )
   {
      vector<double> mean;
      vector<double> stddev;
      calcDefaultMeanAndStdDev( low().continuousPart(), high().continuousPart(), mean, stddev );
      assert( mean.size() == sizeCon() );
      assert( stddev.size() == sizeCon() );
      if ( m_mostLikely.empty() )
      {
         m_mostLikely = mean;
      }
      if ( m_covariance.empty() )
      {
         m_covariance.assign( sizeCon(), vector<double>( sizeCon(), 0.0 ) );
         for ( unsigned int i = 0; i < stddev.size(); ++i )
         {
            m_covariance[i][i] = stddev[i] * stddev[i];
         }
      }
   }

   // Discrete PDF
   if ( sizeDis() > 0 && m_discreteBase.empty() )
   {
      for ( unsigned int i = 0; i < sizeDis(); ++i )
      {
         int baseRange = high().discretePar( i ) - low().discretePar( i );
         int baseValue = low().discretePar( i ) + baseRange/2;
         m_discreteBase[i] = double( baseValue );
      }
   }
   if ( sizeDis() > 0 && m_disWeights.empty() )
   {
      m_disWeights.resize( sizeDis() );
      for ( unsigned int i = 0; i < sizeDis(); ++i )
      {
         int nbOfWeights = 1 + m_maxHigh.discretePar( i ) - m_minLow.discretePar( i );
         m_disWeights[i].assign( nbOfWeights, 1.0 );
      }
   }

   // Categorical PDF
   if ( sizeCat() > 0 && m_categoricalBase.empty() )
   {
      for ( unsigned int i = 0; i < sizeCat(); ++i )
      {
         m_categoricalBase[i] = catValues()[i][0];
      }
   }
   if ( sizeCat() > 0 && m_catWeights.empty() )
   {
      m_catWeights.resize( sizeCat() );
      for ( unsigned int i = 0; i < sizeCat(); ++i )
      {
         int nbOfWeights = 1 + m_maxHigh.categoricalPar( i ) /* - 0 */;
         m_catWeights[i].assign( nbOfWeights, 1.0 );
      }
   }
}

void ParameterPdf::checkDimensions() const
{
   if ( m_catValues.size() != sizeCat() )
   {
      THROW2( DimensionMismatch, "ParameterPdf::catValues" );
   }
   if ( m_categoricalBase.size() != sizeCat() )
   {
      THROW2( DimensionMismatch, "ParameterPdf::categoricalBase" );
   }
   if ( m_discreteBase.size() != sizeDis() )
   {
      THROW2( DimensionMismatch, "ParameterPdf::discreteBase" );
   }
   if ( m_mostLikely.size() != sizeCon() )
   {
      THROW2( DimensionMismatch, "ParameterPdf::mostLikely" );
   }
   if ( m_covariance.size() != sizeCon() )
   {
      THROW2( DimensionMismatch, "ParameterPdf::covariance" );
   }
   for ( unsigned int i = 0; i < sizeCon(); ++i )
   {
      if ( m_covariance[i].size() != sizeCon() )
      {
         THROW2( DimensionMismatch, "ParameterPdf::covariance" );
      }
   }
   if ( m_disWeights.size() != sizeDis() )
   {
      THROW2( DimensionMismatch, "ParameterPdf::disWeights" );
   }
   for ( unsigned int i = 0; i < sizeDis(); ++i )
   {
      int nbOfDisValues = 1 + m_maxHigh.discretePar( i ) - m_minLow.discretePar( i );
      if ( m_disWeights[i].size() != nbOfDisValues )
      {
         THROW2( DimensionMismatch, "ParameterPdf::disWeights" );
      }
   }
   if ( m_catWeights.size() != sizeCat() )
   {
      THROW2( DimensionMismatch, "ParameterPdf::catWeights" );
   }
   for ( unsigned int i = 0; i < sizeCat(); ++i )
   {
      int nbOfCatValues = 1 + m_maxHigh.categoricalPar( i ) /* - 0 */;
      if ( m_catWeights[i].size() != nbOfCatValues )
      {
         THROW2( DimensionMismatch, "ParameterPdf::catWeights" );
      }
   }
}

void ParameterPdf::removeRedundantCatWeights()
{
   // only store weights for used categorical parameter values
   for ( unsigned int i = 0; i < sizeCat(); ++i )
   {
      vector<double> weights( m_catValues[i].size() );
      for ( unsigned int j = 0; j < weights.size(); ++j )
      {
         weights[j] = m_catWeights[i][m_catValues[i][j]];
      }
      m_catWeights[i] = weights;
   }
}

void ParameterPdf::checkCovarianceMatrix( RealMatrix const& cov )
{
   static const double eps( 1e-6 );

   for ( unsigned int i = 0; i < cov.size(); ++i )
   {
      if ( cov[i][i] <= 0 )
      {
         THROW2( InvalidValue, "Not all variances are positive" );
      }

      for ( unsigned int j = 0; j < i; ++j )
      {
         if ( cov[i][j] < cov[j][i] - eps || cov[i][j] > cov[j][i] + eps )
         {
            THROW2( InvalidValue, "Covariance matrix is not symmetric" );
         }

         double rho = cov[i][j] / sqrt( cov[i][i] * cov[j][j] );
         if ( (rho < -1 - eps) || (rho > 1 + eps) )
         {
            THROW2( InvalidValue, "Covariance matrix is out of standard deviation bounds" );
         }
      }
   }
}


void ParameterPdf::calcMinStddev()
{
   CalcMinStdDev( low().continuousPart(), high().continuousPart(), m_minStddev );
   checkStddev();
}


void ParameterPdf::checkStddev()
{
   for ( unsigned int i = 0; i < sizeCon(); ++i )
   {
      double stddev = sqrt( m_covariance[i][i] );
      if ( stddev < m_minStddev[i] )
      {
         m_covariance[i][i] = m_minStddev[i] * m_minStddev[i];
      }
   }
}

void ParameterPdf::calcDefaultMeanAndStdDev(
      vector<double> const&    min,
      vector<double> const&    max,
      vector<double>           &mean,
      vector<double>           &stdev )
{
   // Calculate the range for each continuous parameter
   vector<double> range;
   CalcRange( min, max, range );

   // stdev := range / sqrt(12) corresponding to a Uniform distribution
   stdev.resize( range.size() );
   std::transform( range.begin(), range.end(), stdev.begin(), std::bind2nd( std::divides<double>(), sqrt(12.0) ) );

   // mean := low + range / 2
   mean.resize( range.size() );
   std::transform( min.begin(), min.end(), stdev.begin(), mean.begin(), std::plus<double>() );
}

void ParameterPdf::removeFixedElements()
{
   // Subdivide indices of non-fixed parameters according to type
   m_conVars.reserve( sizeCon() );
   m_disVars.reserve( sizeDis() );
   m_catVars.reserve( sizeCat() );
   for ( unsigned int i = 0; i < m_varParIdx.size(); ++i )
   {
      if ( m_varParIdx[i] < sizeCon() )
      {
         m_conVars.push_back( m_varParIdx[i] );
      }
      else if ( m_varParIdx[i] < sizeOrd() )
      {
         m_disVars.push_back( m_varParIdx[i] - sizeCon() );
      }
      else
      {
         m_catVars.push_back( m_varParIdx[i] - sizeOrd() );
      }
   }
   unsigned int nbOfConVars = m_conVars.size();
   unsigned int nbOfDisVars = m_disVars.size();
   unsigned int nbOfCatVars = m_catVars.size();
   vector<double> conLow( nbOfConVars ), conHigh( nbOfConVars );
   vector<int> disLow( nbOfDisVars ), disHigh( nbOfDisVars );
   vector<unsigned int> catLow( nbOfCatVars ), catHigh( nbOfCatVars );

   // Nothing to be done if all continuous parameters are non-fixed
   if ( nbOfConVars < sizeCon() )
   {
      vector<double> mean( nbOfConVars );
      RealMatrix cov( nbOfConVars, vector<double>( nbOfConVars ) );
      for ( unsigned int i = 0; i < nbOfConVars; ++i )
      {
         conLow[i] = low().continuousPar( m_conVars[i] );
         conHigh[i] = high().continuousPar( m_conVars[i] );
         mean[i] = mostLikely( m_conVars[i] );
         for ( unsigned int j = 0; j < nbOfConVars; ++j )
         {
            cov[i][j] = covariance()[m_conVars[i]][m_conVars[j]];
         }
      }
      m_mostLikely = mean;
      m_covariance = cov;
   }

   // Nothing to be done if all discrete parameters are non-fixed
   if ( nbOfDisVars < sizeDis() )
   {
      vector<double> disBase( nbOfDisVars );
      RealMatrix weights( nbOfDisVars );
      for ( unsigned int i = 0; i < nbOfDisVars; ++i )
      {
         disBase[i] = discreteBase( m_disVars[i] );
         disLow[i] = low().discretePar( m_disVars[i] );
         disHigh[i] = high().discretePar( m_disVars[i] );
         weights[i] = disWeights()[m_disVars[i]];
      }
      m_disWeights = weights;
      m_discreteBase = disBase;
   }

   // Nothing to be done if all categorical parameters are non-fixed
   if ( nbOfCatVars < sizeCat() )
   {
      vector<IndexList> values( nbOfCatVars );
      vector<unsigned int> catBase( nbOfCatVars );
      RealMatrix weights( nbOfCatVars );
      for ( unsigned int i = 0; i < nbOfCatVars; ++i )
      {
         catBase[i] = categoricalBase( m_catVars[i] );
         catLow[i] = low().categoricalPar( m_catVars[i] );
         catHigh[i] = high().categoricalPar( m_catVars[i] );
         values[i] = catValues()[m_catVars[i]];
         weights[i] = catWeights()[m_catVars[i]];
      }
      m_catValues = values;
      m_categoricalBase = catBase;
      m_catWeights = weights;
   }

   // Set new bounds
   if ( nbOfConVars < sizeCon() )
   {
      m_low.setContinuousPart( conLow );
      m_high.setContinuousPart( conHigh );
   }
   if ( nbOfDisVars < sizeDis() )
   {
      m_low.setDiscretePart( disLow );
      m_high.setDiscretePart( disHigh );
   }
   if ( nbOfCatVars < sizeCat() )
   {
      m_low.setCategoricalPart( catLow );
      m_high.setCategoricalPart( catHigh );
   }
}

void ParameterPdf::checkWeights() const
{
   for ( unsigned int i = 0; i < disWeights().size(); ++i )
   {
      checkDisWeights( i );
   }
   for ( unsigned int i = 0; i < catWeights().size(); ++i )
   {
      checkCatWeights( i );
   }
}

void ParameterPdf::checkDisWeights( unsigned int i_p ) const
{
   for ( unsigned int j = 0; j < disWeights()[i_p].size(); ++j )
   {
      if ( disWeights()[i_p][j] < 0.0 )
      {
         THROW2( InvalidValue, "Weights for discrete parameters are negative" );
      }
   }
}

void ParameterPdf::checkCatWeights( unsigned int i_p ) const
{
   for ( unsigned int j = 0; j < catWeights()[i_p].size(); ++j )
   {
      if ( catWeights()[i_p][j] < 0.0 )
      {
         THROW2( InvalidValue, "Weights for categorical parameters are negative" );
      }
   }
}

void ParameterPdf::checkOrdinalBase()
{
   for ( unsigned int i = 0; i < sizeCon(); ++i )
   {
      if ( mostLikely( i ) < m_minLow.continuousPar( i ) )
      {
         m_mostLikely[i] = m_minLow.continuousPar( i );
      }
      else if ( mostLikely( i ) > m_maxHigh.continuousPar( i ) )
      {
         m_mostLikely[i] = m_maxHigh.continuousPar( i );
      }
   }

   for ( unsigned int i = 0; i < sizeDis(); ++i )
   {
      if ( discreteBase( i ) < m_minLow.discretePar( i ) )
      {
         m_discreteBase[i] = m_minLow.discretePar( i );
      }
      else if ( discreteBase( i ) > m_maxHigh.discretePar( i ) )
      {
         m_discreteBase[i] = m_maxHigh.discretePar( i );
      }
   }
}

void ParameterPdf::checkCategoricalBase()
{
   for ( unsigned int i = 0; i < sizeCat(); ++i )
   {
      unsigned int base = categoricalBase( i );
      unsigned int j;
      for ( j = 0; j < catValues()[i].size(); ++j )
      {
         if ( base == catValues()[i][j] ) break;
      }
      if ( j == catValues()[i].size() ) //if base value has not been found in catValues
      {
         m_categoricalBase[i] = catValues()[i][0]; //take the first value
      }
   }
}

void ParameterPdf::scale()
{
   vector<double> minCon( sizeCon(), -1.0 );
   vector<double> maxCon( sizeCon(), 1.0 );
   scaleCon( minCon, maxCon );

   vector<int> minDis( sizeDis(), -1 );
   vector<int> maxDis( sizeDis(), 1 );
   scaleDis( minDis, maxDis );

   m_scaledOrdinalBase = mostLikely();
   m_scaledOrdinalBase.resize( sizeOrd() );
   for ( unsigned int i = 0; i < sizeDis(); ++i )
   {
      m_scaledOrdinalBase[i + sizeCon()] = discreteBase( i );
   }
}

void ParameterPdf::scaleCon( vector<double> const& p_min, vector<double> const& p_max )
{
   assert( p_min.size() == p_max.size() );
   assert( p_min.size() == sizeCon() );
   assert( m_conVars.size() == sizeCon() );

   // Calculate the scale factor for each continuous parameter,
   // and scale the non-fixed outer bounds accordingly.
   vector<double> scaleFactor( sizeCon() );
   for ( unsigned int i = 0; i < sizeCon(); ++i )
   {
      double unscaledRange = high().continuousPar( i ) - low().continuousPar( i );
      assert( unscaledRange > 0.0 );
      scaleFactor[i] = ( p_max[i] - p_min[i] )/unscaledRange;
      double minLow = m_minLow.continuousPar( m_conVars[i] );
      double scaledMinLow = p_min[i] + scaleFactor[i] * ( minLow - low().continuousPar( i ) ); //<= -1
      m_minLow.setContinuousPar( m_conVars[i], scaledMinLow );
      double maxHigh = m_maxHigh.continuousPar( m_conVars[i] );
      double scaledMaxHigh = p_min[i] + scaleFactor[i] * ( maxHigh - low().continuousPar( i ) ); //>= 1
      m_maxHigh.setContinuousPar( m_conVars[i], scaledMaxHigh );
   }

   // Apply scaling to statistical attributes
   for ( unsigned int i = 0; i < sizeCon(); ++i )
   {
      m_mostLikely[i] = ( mostLikely( i ) - low().continuousPar( i ) )*scaleFactor[i] + p_min[i];
      for ( unsigned int j = 0; j < sizeCon(); ++j )
      {
         m_covariance[i][j] *= ( scaleFactor[i] * scaleFactor[j] );
      }
      m_minStddev[i] *= scaleFactor[i];
   }
   m_low.setContinuousPart( p_min );
   m_high.setContinuousPart( p_max );
}

void ParameterPdf::scaleDis( std::vector<int> const& p_min, std::vector<int> const& p_max )
{
   assert( p_min.size() == p_max.size() );
   assert( p_min.size() == sizeDis() );
   assert( m_disVars.size() == sizeDis() );

   // Apply scaling
   vector<double> minLow( m_minLow.ordinalPart() ), maxHigh( m_maxHigh.ordinalPart() );
   unsigned int nbCon = m_minLow.sizeCon();
   for ( unsigned int i = 0; i < sizeDis(); ++i )
   {
      double unscaledRange = double( high().discretePar( i ) - low().discretePar( i ) );
      assert( unscaledRange > 0.0 );
      double scaleFactor = ( p_max[i] - p_min[i] )/unscaledRange;
      m_discreteBase[i] = ( discreteBase( i ) - low().discretePar( i ) )*scaleFactor + p_min[i];
      unsigned int idx = nbCon + m_disVars[i];
      minLow[idx] = -1.0 + scaleFactor * ( m_minLow.ordinalPar( idx ) - low().discretePar( i ) ); //<= -1
      maxHigh[idx] = -1.0 + scaleFactor * ( m_maxHigh.ordinalPar( idx ) - low().discretePar( i ) ); //>= 1
   }
   m_low.setDiscretePart( p_min );
   m_high.setDiscretePart( p_max );
   m_minLow.setContinuousPart( minLow );
   m_maxHigh.setContinuousPart( maxHigh );
   m_minLow.setDiscretePart( vector<int>() );
   m_maxHigh.setDiscretePart( vector<int>() );
}

void ParameterPdf::extendToProxyCase( IndexList const& c, vector<double> &v ) const
{
   unsigned int vIdx = v.size();
   assert( c.size() == sizeCat() );
   assert( vIdx == sizeOrd() );
   unsigned int nbOfDummyPars = 0;
   for ( unsigned int i = 0; i < sizeCat(); ++i )
   {
      nbOfDummyPars += ( catValues()[i].size() - 1 );
   }
   unsigned int newSize = sizeOrd() + nbOfDummyPars;
   v.resize( newSize, 0.0 );

   for ( unsigned int i = 0; i < sizeCat(); ++i )
   {
      unsigned int val = c[i];
      unsigned int valIdx;
      for ( valIdx = 0; valIdx < catValues()[i].size(); ++valIdx )
      {
         if ( catValues()[i][valIdx] == val ) break;
      }
      assert( valIdx < catValues()[i].size() );
      if ( valIdx > 0 ) //otherwise all binary values must remain zero
      {
         v[vIdx + valIdx - 1] = 1.0;
      }
      vIdx += ( catValues()[i].size() - 1 );
   }
   assert( vIdx == newSize );
}

void ParameterPdf::setStdDev( unsigned int idx, double stddev )
{
   for ( unsigned int i = 0; i < m_conVars.size(); ++i )
   {
      if ( m_conVars[i] == idx )
      {
         if ( stddev < minStddev( i ) )
         {
            stddev = minStddev( i );
         }
         m_covariance[i][i] = stddev * stddev;
         break;
      }
   }
}

void ParameterPdf::setDisWeights( unsigned int idx, vector<double> const& w )
{
   for ( unsigned int i = 0; i < m_disVars.size(); ++i )
   {
      if ( m_disVars[i] == idx )
      {
         int l = m_minLow.discretePar( idx );
         int h = m_maxHigh.discretePar( idx );
         assert( disWeights()[i].size() == 1 + h - l );
         if ( w.size() != 1 + h - l )
         {
            THROW2( DimensionMismatch, "Supplied weights" );
         }
         m_disWeights[i] = w;
         checkDisWeights( i );
      }
   }
}

void ParameterPdf::setCatWeights( unsigned int idx, vector<double> const& w )
{
   for ( unsigned int i = 0; i < m_catVars.size(); ++i )
   {
      if ( m_catVars[i] == idx )
      {
         assert( catWeights()[i].size() == catValues()[i].size() );
         unsigned int nbW = 1 + m_maxHigh.categoricalPar( idx );
         if ( w.size() != nbW )
         {
            THROW2( DimensionMismatch, "Supplied weights" );
         }
         for ( unsigned int j = 0; j < catWeights()[i].size(); ++j )
         {
            m_catWeights[i][j] = w[catValues()[i][j]];
         }
         checkCatWeights( i );
      }
   }
}

vector<double> ParameterPdf::lowestNonFrozenOrdParams() const
{
   vector<double> lowestVarOrdPars( sizeOrd() );
   for ( unsigned int i = 0; i < sizeOrd(); ++i )
   {
      lowestVarOrdPars[i] = m_minLow.ordinalPar( varParIdx()[i] );
   }
   return lowestVarOrdPars;
}

vector<double> ParameterPdf::highestNonFrozenOrdParams() const
{
   vector<double> highestVarOrdPars( sizeOrd() );
   for ( unsigned int i = 0; i < sizeOrd(); ++i )
   {
      highestVarOrdPars[i] = m_maxHigh.ordinalPar( varParIdx()[i] );
   }
   return highestVarOrdPars;
}

bool ParameterPdf::load( IDeserializer* deserialiser, unsigned int /*version*/ )
{
   bool ok = true;
   ok = ok && deserialize( deserialiser, m_low );
   ok = ok && deserialize( deserialiser, m_high );
   ok = ok && deserialize( deserialiser, m_minLow );
   ok = ok && deserialize( deserialiser, m_maxHigh );
   ok = ok && deserialize( deserialiser, m_catValues );
   ok = ok && deserialize( deserialiser, m_varParIdx );
   ok = ok && deserialize( deserialiser, m_conVars );
   ok = ok && deserialize( deserialiser, m_disVars );
   ok = ok && deserialize( deserialiser, m_catVars );
   ok = ok && deserialize( deserialiser, m_discreteBase );
   ok = ok && deserialize( deserialiser, m_mostLikely );
   ok = ok && deserialize( deserialiser, m_scaledOrdinalBase );
   ok = ok && deserialize( deserialiser, m_categoricalBase );
   ok = ok && deserialize( deserialiser, m_covariance );
   ok = ok && deserialize( deserialiser, m_minStddev );
   ok = ok && deserialize( deserialiser, m_disWeights );
   ok = ok && deserialize( deserialiser, m_catWeights );

   return ok;
}

bool ParameterPdf::save( ISerializer* serialiser, unsigned int /*version*/ ) const
{
   bool ok = true;
   ok = ok && serialize( serialiser, m_low );
   ok = ok && serialize( serialiser, m_high );
   ok = ok && serialize( serialiser, m_minLow );
   ok = ok && serialize( serialiser, m_maxHigh );
   ok = ok && serialize( serialiser, m_catValues );
   ok = ok && serialize( serialiser, m_varParIdx );
   ok = ok && serialize( serialiser, m_conVars );
   ok = ok && serialize( serialiser, m_disVars );
   ok = ok && serialize( serialiser, m_catVars );
   ok = ok && serialize( serialiser, m_discreteBase );
   ok = ok && serialize( serialiser, m_mostLikely );
   ok = ok && serialize( serialiser, m_scaledOrdinalBase );
   ok = ok && serialize( serialiser, m_categoricalBase );
   ok = ok && serialize( serialiser, m_covariance );
   ok = ok && serialize( serialiser, m_minStddev );
   ok = ok && serialize( serialiser, m_disWeights );
   ok = ok && serialize( serialiser, m_catWeights );

   return ok;
}

} // namespace SUMlib
