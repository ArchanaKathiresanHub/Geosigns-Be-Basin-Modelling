// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#ifndef SUMLIB_MCMCSTATISTICS_H
#define SUMLIB_MCMCSTATISTICS_H

#include <ostream>
#include <vector>

#include "BaseTypes.h"
#include "SUMlib.h"
#include "ISerializer.h"

class McmcProxy;

namespace SUMlib {

/// @class McmcStatistics collects chi2 distribution characteristics and
/// average and covariances of the sample parameters and responses
class INTERFACE_SUMLIB McmcStatistics : public ISerializable
{
   public:

      /// Constructor
      McmcStatistics();

      /// Destructor
      virtual ~McmcStatistics();

      /// Calculate the chi2 distribution characteristics for the sample.
      ///
      /// If stdDevFactor is zero (default), the sqrt(chi2) value is used as standard deviation factor
      /// for reduced sample statistics.
      void calcChi2Distribution(
            std::vector<TargetSet> const&    ySample,
            std::vector<McmcProxy*> const&   proxies,
            unsigned int                     numDegFreedom,
            double                           stdDevFactor = 0 );

      /// Update all mcmc statistics
      ///
      /// For each parameter element the average, variance and covariances are calculated from the parameter
      /// element values in pSample.
      /// For each target set in ySample (i.e. for each proxy) the average over the responses to the parameters
      /// in the sample is calculated.
      void update(
            ParameterSet const&                 pSample,
            std::vector<TargetSet> const&       ySample,
            unsigned int                        numDegFreedom,
            double                              stdDevFactor,
            std::vector<McmcProxy*> const&      proxies,
            std::vector< unsigned int > const&  catIndexOfSample );

      /// Get the number of degrees of freedom for the Chi2 distribution
      /// @return the number of degrees of freedom for which chi2 and goodness of fit were calculated
      unsigned int numDegFreedom() const { return m_numDegFreedom; }

      /// Get Chi2 without stdDevFactor taken into account.
      /// @returns chi2
      double getChi2() const { return m_chi2; }

      /// Get Chi2 with stdDevFactor taken into account.
      /// @returns chi2
      double getChi2Reduced() const { return m_chi2Red; }

      /// Goodness of fit without std deviation factor taken into account
      /// @returns goodness of fit
      double getGoodnessOfFit() const { return m_gof; }

      /// Goodness of fit with std deviation factor taken into account
      /// @returns goodness of fit
      double getGoodnessOfFitReduced() const { return m_gofRed; }

      /// Getter for the sample parameters average
      ///
      /// The return vector is resized to the size of the parameters in the sample.
      /// @param [out] sampleAvg vector of parameter element averages
      void getSampleParameterAverage( RealVector &sampleAvg ) const;
      
      /// Getter for the sample parameters average of all categorical varmodels
      ///
      /// The return matrix of number of categorical combination by the size of the parameters in
      /// the categorical sample.
      /// @param [out] sampleAvg matrix of parameter element averages of categorical values
      void getCatSampleParameterAverages( RealMatrix &avg ) const;
      
      /// Getter for the sample response average
      ///
      /// The return vector is resized to the number of proxies.
      /// @param [out] sampleAvg vector of target set averages for each proxy
      void getSampleResponseAverage( RealVector &sampleAvg ) const;

      /// Getter for the scaled deviation
      ///
      /// The return vector is resized to the number of proxies.
      /// @param [out] scaledDeviation vector of scaled deviations w.r.t. the reference values
      void getScaledDeviation( RealVector &scaledDeviation ) const;

      /// Getter for the sample parameters covariances
      ///
      /// The return vector is resized to the size of the parameters in the sample.
      /// @param [out] sampleAvg vector of parameter element averages
      void getSampleParameterCovarianceMatrix( RealMatrix &sampleCovMat ) const;

      /// Getter for the sample parameters covariances of categorical values
      ///
      /// The return vector is resized to the size of the categorial combinations.
      /// @param [out] sampleAvg vector of parameter element averages per categorical combination
      /// Dimension: [catCombis][ordVarmodels][ordVarmodels]
      void getCatSampleParameterCovarianceMatrix( std::vector< RealMatrix > &sampleCatCovMat ) const;

      /// Estimate for the truncated distribution maximum likelyhood parameter
      /// factor values.
      ///
      /// When close to a boundary, the mean sample factors are different. It is an
      /// estimate, assuming independence and normal distribution of the factors,
      /// and it imposes that the value lies within factor bounds.
      ///
      /// Dimension: [pSize]
      void getTruncatedDistributionMaximumLikelihoodParameter( Parameter &mlMu ) const;

      /// Estimate of the truncated distribution variance under the same
      /// assumptions as described for m_mlMu.
      ///
      /// Dimension: [pSize]
      void getTruncatedDistributionMaximumLikelihoodVariance( Parameter &mlSd ) const;

      /// Print statistics to an output stream
      /// For testing only(?)
      std::ostream& print( std::ostream& out, double stdDevFactor, std::vector<McmcProxy*> const& proxies ) const;

      // ISerializable interface
      virtual bool load( IDeserializer*, unsigned int );
      virtual bool save( ISerializer*, unsigned int ) const;

   private:

      void calcCatAverages( const RealMatrix& pSample, const std::vector< unsigned int > &catIdx );

      void calcCatCovariances( const RealMatrix& pSample, const std::vector< unsigned int > &catIdx );

      static double calcGoF( double chi2, unsigned int numDegFreedom );

   private:

      /// Chi squared
      double m_chi2;

      /// Goodness of fit: 0.1 - 0.9 considered a good fit.
      double m_gof;

      /// Reduced Chi squared.
      double m_chi2Red;

      /// Reduced Goodness of fit: 0.1 - 0.9 considered a good fit.
      double m_gofRed;

      unsigned int m_numDegFreedom;

      RealVector m_lhMu;
      RealVector m_lhSd;

      RealVector                 m_pSampleAvg;
      RealMatrix                 m_pCatSampleAvgs;
      RealMatrix                 m_pSampleCovMat;
      std::vector< RealMatrix >  m_pCatSampleCovMat;
      RealVector                 m_ySampleAvg;
      RealVector                 m_scaledDev;
};

/// Maps the goodnessOfFit(Red) onto the range 0-100%
/// This mapped value should be at least 50%; 100% is optimal.
INTERFACE_SUMLIB
double MapGOF(double GOF);

/// Calculate the chi2 from the (ED) simulated results.
/// @return The sum of Chi2.
///
/// This does not include the standard deviation factor.
INTERFACE_SUMLIB
double SumChiSquared( std::vector<std::vector< double > > const& y, std::vector<McmcProxy*> const& px );

} // namespace SUMlib


#endif // SUMLIB_MCMCSTATISTICS_H

