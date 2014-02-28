// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#ifndef SUMLIB_MARGINALPROBDISTR_H
#define SUMLIB_MARGINALPROBDISTR_H

#include <vector>

#include "BaseTypes.h"
#include "ProbDistr.h"
#include "SUMlib.h"

namespace SUMlib
{
   class ParameterPdf;

/// Marginal prior probability distribution for each parameter:
/// Normal, Log-Normal, Uniform or Triangular on the interval [low, high].
/// The Log-Normal pdf is parameterised by m_mu and m_sigma whereas the ParameterPdf
/// is characterised by m_base and m_stdev. In principle, three configurations are possible:
/// logNormalInfo = 0: m_base = m_mu and m_stdev = m_sigma (not used here)
/// logNormalInfo = 1: m_base is the mode (i.e. most likely) of the ParameterPDF (MultiRun/SUM++ convention)
/// logNormalInfo > 1: m_base is the median of the ParameterPDF (not used here)
///
/// Here logNormalInfo = 1 will be assumed as this is in agreement with MultiRun!
class INTERFACE_SUMLIB MarginalProbDistr : public ProbDistr
{

  public:

     using ProbDistr::calcLogPriorProb;

     /// Enumerates the types of parameter marginal distributions.
     enum Type {
        LogNormal,
        Normal,
        Triangular,
        Uniform
     };

      /// This constructor can only be used for continuous parameters!
      /// The marginal probability distributions are handled, with known base and covariances.
      MarginalProbDistr(
            const std::vector<double>& base,
            const std::vector<std::vector<double> >& cov,
            const std::vector<double>& low,
            const std::vector<double>& high,
            const std::vector<Type>& types = std::vector<Type>(),
            unsigned int logNormalInfo = 1
            );

      /// This constructor supports all ordinal parameters (discrete ones are interpolated)!
      /// Base, variances, low and high are taken from ParameterPdf data.
      /// @param [in] pdf parameter pdf to use bounds, base and variances from
      MarginalProbDistr(
            ParameterPdf const& pdf,
            const std::vector<Type>& types = std::vector<Type>(),
            unsigned int logNormalInfo = 1
            );

      /// Destructor
      virtual ~MarginalProbDistr();

      /// Initialise the marginal probability distributon, with known base and covariances
      void initialise(
            const std::vector<double>& base,
            const std::vector<std::vector<double> >& cov,
            const std::vector<double>& low,
            const std::vector<double>& high,
            const std::vector<double>& pdfMin,
            const std::vector<double>& pdfMax,
            const std::vector<Type>& types = std::vector<Type>(),
            unsigned int logNormalInfo = 1
            );

      /// Setter for the parameter distribution types
      void setTypes( std::vector<Type> const& types );

      /// Calculates the logarithm of the prior probability density for parameter p.
      ///
      /// The logarithm of the product of the marginals = the sum of the logarithms of the
      /// individual marginals. Note that normalization factors are neglected as they cancel
      /// out in the MCMC procedure.
      double calcLogPriorProb( Parameter const& p ) const;

   private:
      /**
       *  Get internal parameters mu and sigma as a function of median, stdev and low;
       *    low comes into play if it is not zero as it is subtracted from mode when the
       *  logNormal properties are calculated (support on positive axis only!).
       */
      void initLogNormal2( const std::vector<double>& median, const std::vector<double>& stdev,
            const std::vector<double>& low, std::vector<double>& mu, std::vector<double>& sigma );

      /**
       *  Get internal parameters mu and sigma as a function of mode, stdev and low;
       *    low comes into play if it is not zero as it is subtracted from mode when the
       *  logNormal properties are calculated (support on positive axis only!).
       */
      void initLogNormal1( const std::vector<double>& mode, const std::vector<double>& stdev,
            const std::vector<double>& low, std::vector<double>& mu, std::vector<double>& sigma );

      // Calculation of logarithm of marginal Normal probability density function for parameter p
      double calcLogMarginalNormal( double p, double mean, double stdev ) const;

      // Calculation of logarithm of marginal LogNormal probability density function for parameter p
      double calcLogMarginalLogNormal( double p, double low, double mu, double sigma ) const;

      // Calculation of logarithm of marginal Triangular probability density function for parameter p
      double calcLogMarginalTriangular( double p, double base, double low, double high ) const;

      std::vector<Type> m_types;

      // Mean values of the continuous parameters of the pdf
      std::vector<double> m_mean;

      // Base values of the ordinal parameters of the pdf
      std::vector<double> m_base;

      // Prior standard deviations corresponding to the continuous parameters
      std::vector<double> m_stdev;

      // Lower and upper bounds for the parameters
      std::vector<double> m_low, m_high;
      
      // Lower and upper bounds for the parameter PDF's
      std::vector<double> m_pdfMin, m_pdfMax;

      // Internal parameters mu and sigma needed to describe a logNormal distribution
      std::vector<double> m_mu;
      std::vector<double> m_sigma;

      // Weights for the discrete parameters
      std::vector<std::vector<double> > m_disWeights;

}; // class MarginalProbDistr

} // namespace SUMlib

#endif // SUMLIB_MARGINALPROBDISTR_H
