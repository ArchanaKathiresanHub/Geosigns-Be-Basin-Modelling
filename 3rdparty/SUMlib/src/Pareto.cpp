// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#include <algorithm>
#include <cassert>
#include <cmath>
#include <functional>
#include <map>
#include <numeric>
#include <vector>

#include "DataStructureUtils.h"
#include "Exception.h"
#include "NumericUtils.h"
#include "ParameterPdf.h"
#include "Pareto.h"
#include "Proxy.h"
#include "SUMlib.h"

using std::vector;

namespace SUMlib {

Pareto::Pareto()
{
}

Pareto::~Pareto()
{
}

void Pareto::normalizeAndSort( vector<double>&values, vector<unsigned int>&indirection, vector<double>& cumulatives )
{
   size_t size = values.size();

   cumulatives.resize( size );

   // If the proxy data is non-trivial do the normalization and sorting, and calculate cumulatives
   if ( normalize( values ) )
   {
      std::vector<double>tmp( values );

      orderIndexes( values, indirection );

      //sorting: descending in size.
      for ( unsigned int i = 0; i < tmp.size(); ++i )
      {
         values[i] = tmp[indirection[i]];
      }
      std::partial_sum( values.begin(), values.end(), cumulatives.begin() );
   }
}

unsigned int Pareto::getNumParetoParameters( double impact, RealVector const& cumSensitivity )
{
   // Assumes cumSensitivity to be sorted
   // Assumes impactLevel to be a fraction

   double percentage = 100* std::min( impact, 1.0 );

   // Find the first element in the cumulative sensitivity array greater than
   // the specified impact level. This is the first element not belonging to the pareto elements
   vector<double>::const_iterator it = find_if( cumSensitivity.begin(), cumSensitivity.end(),
         std::bind2nd( std::greater_equal<double>(), percentage ) );

   return static_cast<unsigned int>( std::distance( cumSensitivity.begin(), it ) + 1 );
}

double Pareto::getParetoParameters(
      double impactLevel,
      const vector<double>& values,
      IndexList& paretoParameters )
{
   RealVector val( values );
   RealVector cumulatives;
   Pareto::normalizeAndSort( val, paretoParameters, cumulatives );

   // Determine the number of parameters to be included
   unsigned int num = getNumParetoParameters( impactLevel, cumulatives );

   // Chop off the insignificant parameters
   paretoParameters.resize( num );

   return cumulatives[num-1];
}

void Pareto::calcSensitivities(
      SensitivityInput const& inputs,
      const ParameterPdf& priorPar,
      vector<double>& sensitivity ) const
{
   // Initialization and declaration.
   const unsigned int nbPar = priorPar.size();

   if ( sensitivity.size() != priorPar.size() )
   {
      THROW2( DimensionMismatch, "sensitivity array not initialised to the same size as parameter set" );
   }

   vector<vector<double> > propSensitivities;
   static const double minScale = MachineEpsilon(); //minimal response scale in SI units
   const unsigned int nbProp = static_cast<unsigned int>( inputs.size() );
   vector<double> rangeOfPropertyResponse( nbProp, minScale );
   getPropertySensitivities( inputs, priorPar, propSensitivities, rangeOfPropertyResponse );

   // Calculate the scaled sensitivities.
   for ( unsigned int i = 0; i < nbPar; ++i ) //loop over the parameters.
   {
      for ( unsigned int k = 0; k < nbProp; ++k ) //loop over the selected properties.
      {
         // propSensitivities[][k] must be scaled by a characteristic property range before they can be summed!
         sensitivity[i] += propSensitivities[i][k]/rangeOfPropertyResponse[k];
      }
   }
}

void Pareto::getPropertySensitivities(
         SensitivityInput const& inputs,
         const ParameterPdf& priorPar,
         vector<vector<double> >& propSensitivities,
         vector<double>& rangeOfPropertyResponse ) const
{
   if ( rangeOfPropertyResponse.size() != inputs.size() )
   {
      THROW2( DimensionMismatch, "scaleOfResponse array not initialised to the same size as inputs array" );
   }
   const unsigned int size_k = static_cast<unsigned int>(inputs.size()); //number of properties

   const unsigned int nbPar = priorPar.size();
   const unsigned int nbOrdPar = priorPar.sizeOrd();
   vector<unsigned int> catCase = priorPar.categoricalBase(); //get categorical base case
   vector<double> cp = priorPar.scaledOrdinalBase();
   assert( cp.size() == nbOrdPar );
   
   // Enforce the base values between the scaled parameter bounds of the proxy
   for ( unsigned int i = 0; i < cp.size(); ++i )
   {
      if ( cp[i] < -1.0 )
      {
         cp[i] = -1.0;
      }
      else if ( cp[i] > 1.0 )
      {
         cp[i] = 1.0;
      }
   }
   
   vector<vector<double> > proxyCases( 5, cp );
   for ( unsigned int i = 0; i < proxyCases.size(); ++i )
   {
      priorPar.extendToProxyCase( catCase, proxyCases[i] );
   }

   // Container for the sensitivities of all parameters per property:
   propSensitivities.resize( nbPar );

   /// Define the parameter values at the extreme points (-1 and +1) and
   /// at 2 intermediate points in each ordinal parameter direction.
   /// Per direction, the 4 points (and the centerpoint) are substituted giving the corresponding proxy values.
   /// The maximum minus the minimum proxy level is a measure for the sensitivity.
   for ( unsigned int i = 0; i < nbOrdPar; ++i ) //loop over the ordinal parameters.
   {
      proxyCases[0][i] = -1.0; //left bound
      proxyCases[1][i] = 0.5 * ( -1.0 + cp[i] ); //intermediate point left from center
      proxyCases[3][i] = 0.5 * ( 1.0 + cp[i] ); //intermediate point right from center
      proxyCases[4][i] = 1.0; //right bound

      vector<double> proxyValue( 5 );
      propSensitivities[i].assign( size_k, 0.0 );
      for ( unsigned int k = 0; k < size_k; ++k ) //loop over the selected properties.
      {
         const unsigned int size_j = static_cast<unsigned int>(inputs[k].size());
         for ( unsigned int j = 0; j < size_j; ++j ) //loop over the wells.
         {
            Proxy const& px = *( inputs[k][j].first );
            for ( unsigned int l = 0; l < 5; ++l )
            {
               proxyValue[l] = px.getProxyValue( proxyCases[l] );
            }

            double minProxy = *(std::min_element( proxyValue.begin(), proxyValue.end() ));
            double maxProxy = *(std::max_element( proxyValue.begin(), proxyValue.end() ));
            double dProxy = maxProxy - minProxy; //raw sensitivity

            if ( dProxy > rangeOfPropertyResponse[k] )
            {
               rangeOfPropertyResponse[k] = dProxy;
            }
            propSensitivities[i][k] += inputs[k][j].second * dProxy; //actual weighting of the raw sensitivities!
         }
      }

      // Parameter i back at centerpoint
      for ( unsigned int l = 0; l < 5; ++l )
      {
         proxyCases[l][i] = cp[i];
      }
   }

   /// Define the parameter values at the available categorical values in each categorical parameter direction.
   /// Per direction, the available values are substituted and converted to proxy cases to evaluate the proxies.
   /// The maximum minus the minimum proxy level is a measure for the sensitivity.
   for ( unsigned int i = nbOrdPar; i < nbPar; ++i ) //loop over the categorical parameters.
   {
      // Define sensitivity cases; one for each categorical value
      const unsigned int catParIdx = i - nbOrdPar;
      const unsigned int nbProxyCases = static_cast<unsigned int>(priorPar.catValues()[catParIdx].size()); //number of categorical values
      assert( nbProxyCases > 1 );
      proxyCases.assign( nbProxyCases, cp ); //step 1: set all proxy cases equal to ordinal base case
      unsigned int baseCatValue = catCase[catParIdx];
      for ( unsigned int l = 0; l < nbProxyCases; ++l ) //loop over the categorical values
      {
         catCase[catParIdx] = priorPar.catValues()[catParIdx][l]; //step 2a: perturb categorical case
         priorPar.extendToProxyCase( catCase, proxyCases[l] ); //step 2b: extend to actual proxy case
      }
      catCase[catParIdx] = baseCatValue; //back to base (default) value

      // Calculate sensitivities
      vector<double> proxyValue( nbProxyCases );
      propSensitivities[i].assign( size_k, 0.0 );
      for ( unsigned int k = 0; k < size_k; ++k ) //loop over the selected properties.
      {
         const unsigned int size_j = static_cast<unsigned int>( inputs[k].size() );
         for ( unsigned int j = 0; j < size_j; ++j ) //loop over the wells.
         {
            Proxy const& px = *( inputs[k][j].first );
            for ( unsigned int l = 0; l < nbProxyCases; ++l )
            {
               proxyValue[l] = px.getProxyValue( proxyCases[l] );
            }
            double minProxy = *(std::min_element( proxyValue.begin(), proxyValue.end() ));
            double maxProxy = *(std::max_element( proxyValue.begin(), proxyValue.end() ));
            double dProxy = maxProxy - minProxy; //raw sensitivity
            if ( dProxy > rangeOfPropertyResponse[k] )
            {
               rangeOfPropertyResponse[k] = dProxy;
            }
            propSensitivities[i][k] += inputs[k][j].second * dProxy; //actual weighting of the raw sensitivities!
         }
      }
   }
}

/**
 *  Get the normalized sensitivities of the parameters, based on the proxy models associated
 *  with the selected properties, wells and time.
 */
void Pareto::getSensitivities(
      SensitivityInput const& inputs,
      const ParameterPdf& priorPar,
      vector<double>& sensitivity ) const
{
   const unsigned int nbPar = priorPar.size();

   // Initialise to exactly nbPar elements of value 0.0
   std::vector<double>(nbPar, 0.0).swap( sensitivity );

   calcSensitivities( inputs, priorPar, sensitivity );
}

/**
 *  Get the normalized sensitivities of the parameters, based on the proxy models associated
 *  with the selected properties, wells and time.
 */
void Pareto::getSensitivities(
      SensitivityInput const& inputs,
      const ParameterPdf& priorPar,
      vector<unsigned int>& parSeqnb,
      vector<double>& sensitivity,
      vector<double>& cumSensitivity ) const
{
   getSensitivities( inputs, priorPar, sensitivity );

   normalizeAndSort( sensitivity, parSeqnb, cumSensitivity );
}

namespace {

// Flatten the 3D array to 2D array
void MapSensitivityInput( vector<Pareto::SensitivityInput> const& input3D, Pareto::SensitivityInput& input2D )
{
   // total number of time steps.
   const size_t nbTimes = input3D.size();

   // total number of properties
   size_t nbProp(0);
   for ( size_t t = 0; t < nbTimes; ++t )
   {
      nbProp = std::max<size_t>( input3D[t].size(), nbProp );
   }

   input2D.clear();
   input2D.resize( nbProp );

   for ( size_t t = 0; t < nbTimes; ++t )
   {
      for ( size_t p = 0; p < input3D[t].size(); ++p )
      {
         for ( size_t w = 0; w < input3D[t][p].size(); ++w )
         {
            input2D[p].push_back( input3D[t][p][w] );
         }
      }
   }
}
} // unnamed namespace

// Note: The only difference between getSensitivities and getAvgSensitivities
// is the way the collection of Proxys is passed into the function.
void Pareto::getAvgSensitivities(
      vector<SensitivityInput> const& inputs,
      const ParameterPdf& priorPar,
      vector<double>& sensitivity ) const
{
   SensitivityInput input2D;
   MapSensitivityInput( inputs, input2D );

   getSensitivities(input2D, priorPar, sensitivity );
}

// Note: The only difference between getSensitivities and getAvgSensitivities
// is the way the collection of Proxys is passed into the function.
void Pareto::getAvgSensitivities(
      vector<SensitivityInput> const& inputs,
      const ParameterPdf& priorPar,
      vector<unsigned int>& parSeqnb,
      vector<double>& sensitivity,
      vector<double>& cumSensitivity) const
{
   getAvgSensitivities(inputs, priorPar, sensitivity);
   normalizeAndSort( sensitivity, parSeqnb, cumSensitivity );
}

bool Pareto::normalize( std::vector<double> &values )
{
   static const double eps = MachineEpsilon();

   const double factor = std::accumulate( values.begin(), values.end(), 0.0 );

   bool ok = (factor > eps );
   if ( ok )
   {
      std::transform( values.begin(), values.end(), values.begin(), std::bind2nd( std::multiplies<double>(), (100.0/factor) ) );
   }
   return ok;
}

void Pareto::orderIndexes( RealVector const& values, std::vector<unsigned int>& indexes )
{
   // Clear the sorted index list
   indexes.clear();

   // Sort the values using a std::map
   typedef std::multimap<double,unsigned int> Sorter;
   Sorter sorter;
   unsigned int idx(0);
   for ( RealVector::const_iterator it = values.begin(); it != values.end(); ++it )
   {
      sorter.insert( std::make_pair(*it, idx++) );
   }
   // Now copy the sorted index list
   indexes.reserve( values.size() );
   for ( Sorter::reverse_iterator s = sorter.rbegin(); s != sorter.rend(); ++s )
   {
      indexes.push_back( (*s).second );
   }
   assert( indexes.size() == values.size() );
}

} // namespace SUMlib
