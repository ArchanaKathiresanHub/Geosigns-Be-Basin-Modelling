// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#ifndef SUMLIB_PARETO_H
#define SUMLIB_PARETO_H

#include <vector>

#include "BaseTypes.h"
#include "SUMlib.h"


namespace SUMlib {

class ParameterPdf;
class Proxy;


/// Sensitivities of the screening parameters on one or more properties at one point in time are calculated.
/// All selected properties are represented by the associated proxy models at the selected point in time.
/// If the user has selected more than one well,
/// the proxy models for each individual property are summed over the wells.

/// IMPORTANT: This class assumes only non-frozen parameters (in a scaled space) that cannot be related to
/// the parameters from the total parameter set. As a result, the caller of SUMlib::Pareto functions is
/// responsible for the mapping of the sensitivities back to the correct parameters!
class INTERFACE_SUMLIB Pareto
{
public:

   /// @typedef SensitivityInput 2D array of proxy and weight pairs.
   /// SensitivityInput[i][j] corresponds to a proxy and weight pair for
   /// property i and well j.
   typedef std::vector< std::vector< std::pair<const Proxy*,double> > > SensitivityInput;

   /// Constructor
   Pareto();

   /// Destructor
   virtual ~Pareto();

   /// Calculate sensitivities for Proxies to Parameters
   /// @param in input[i][j] corresponds to a proxy and weight pair for property i and well j.
   /// @param in priorPar corresponds to task-specific parameter settings.
   /// @param out normalized sensitivities are ordered (descending in size) in array sensitivity.
   void getSensitivities(
         SensitivityInput const& input,
         const ParameterPdf& priorPar,
         std::vector<double>& sensitivity ) const;

   /// Calculate sensitivities for Proxies to Parameters
   /// @param in input[i][j] corresponds to a proxy and weight pair for property i and well j.
   /// @param in priorPar corresponds to task-specific parameter settings.
   /// @param out normalized sensitivities are ordered (descending in size) in array sensitivity.
   /// @param out parSeqnb[i] gives the parameter index that corresponds to sensitivity[i].
   /// @param out cumSensitivity[i] equals the sum of sensitivity[0] to sensitivity[i].
   void getSensitivities(
         SensitivityInput const& input,
         const ParameterPdf& priorPar,
         std::vector<unsigned int>& parSeqnb,
         std::vector<double>& sensitivity,
         std::vector<double>& cumSensitivity ) const;

   /// Calculate time-averaged sensitivities for Proxies to Parameters.
   /// @param [in]   input[t][i][j] corresponds to a proxy and weight pair for time t, property i and well j.
   /// @param [in]   priorPar corresponds to task-specific parameter settings.
   /// @param [out]  sensitivities
   void getAvgSensitivities(
         std::vector<SensitivityInput> const& input,
         const ParameterPdf& priorPar,
         std::vector<double>& sensitivity ) const;

   /// Calculate time-averaged sensitivities for Proxies to Parameters
   /// @param [in]  input[t][i][j] corresponds to a proxy and weight pair for time t, property i and well j.
   /// @param [in]  priorPar corresponds to task-specific parameter settings.
   /// @param [out] normalized sensitivities are ordered (descending in size) in array sensitivity.
   /// @param [out] parSeqnb[i] gives the parameter index that corresponds to sensitivity[i].
   /// @param [out] cumSensitivity[i] equals the sum of sensitivity[0] to sensitivity[i].
   void getAvgSensitivities(
         std::vector<SensitivityInput> const& input,
         const ParameterPdf& priorPar,
         std::vector<unsigned int>& parSeqnb,
         std::vector<double>& sensitivity,
         std::vector<double>& cumSensitivity) const;

   /// Sort the values in ascending order, provide an index list to the original
   /// positions and return a list of cumulative relative contributions
   /// @param [in,out] values      on input unsorted values, on output sorted in ascending order
   /// @param [out]    indirection index list of original positions of values
   /// @param [out]    cumulatives cumulative relative contributions
   static void normalizeAndSort(
         std::vector<double>&values,
         std::vector<unsigned int>&indirection,
         std::vector<double>& cumulatives );

   /// Normalize the values to a cumulative value of 100%
   /// @param [in,out] values vector of values to normalize. On output the sum of all values is 100.0
   /// @returns whether the normalization factor is larger than the threshold value
   static bool normalize( std::vector<double> & values );

   /// Order the vector of indexes such that they point to non-decreasing values
   /// @param [in]        values vector of values to use as sorting criterion
   /// @param [out] index vector of indexes. On output the index[p] indicates the value that has p-1
   ///                    elements with smaller values.
   static void orderIndexes( std::vector<double> const& values, std::vector<unsigned int> & index );


   /// Determine the indexes to the most significant parameters. The cumulative relative
   /// contribution of these parameters is at least equal to the supplied impact level (0:1].
   /// The indexes are ordered to decreasing significance/contribution.
   /// @param [in] impactLevel
   /// @param [in] unsorted values
   /// @param [out] index list of pareto parameters up to specified impact level
   /// @returns the cumulative contribution of the parameters as a fraction
   static double getParetoParameters( double impactLevel, RealVector const& values, IndexList & indexes );

private:

   /// @brief Perform the actual calculations
   /// @param [in]  input        2D array of proxy and weight pairs.
   /// @param [in]  priorPar     the parameter set to calculate sensitivities for
   /// @param [out] sensitivity  the parameter sensitivities
   void calcSensitivities(
         SensitivityInput const& input,
         const ParameterPdf& priorPar,
         std::vector<double>& sensitivity ) const;

   /// Calculate property sensitivities
   /// @param [in]  input                   2D array of proxy and weight pairs.
   /// @param [in]  priorPar                the parameter set to calculate sensitivities for
   /// @param [out] propSensitivities       stores the weighted sensitivities for parameter i and property k
   /// @param [out] rangeOfPropertyResponse gives the range/spread of each property response
   void getPropertySensitivities(
         SensitivityInput const& input,
         const ParameterPdf& priorPar,
         std::vector<std::vector<double> >& propSensitivities,
         std::vector<double>& rangeOfPropertyResponse ) const;


   /// Return the number of significant parameters for the given impact level, based on
   /// the supplied cumulative sensitivity
   /// @param [in] impactLevel
   /// @param [in] cumulative sensitivity array, assumed sorted
   /// @returns the number of parameters that represent an impactLevel that is not less than the specified level
   static unsigned int getNumParetoParameters( double impactLevel, RealVector const& cumSensitivity );

};


} // namespace SUMlib

#endif // SUMLIB_PARETO_H
