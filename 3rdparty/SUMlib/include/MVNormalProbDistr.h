// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#ifndef SUMLIB_MVNORMALPROBDISTR_H
#define SUMLIB_MVNORMALPROBDISTR_H

#include <vector>

#include "ProbDistr.h"
#include "SUMlib.h"

namespace SUMlib {

class ParameterPdf;
class RandomGenerator;

///  Multi-Variable Normal (Gauss) probability distribution
///  with mean and covariances.
class INTERFACE_SUMLIB MVNormalProbDistr : public ProbDistr
{
public:

   using ProbDistr::calcLogPriorProb;

   /// Default constructor
   MVNormalProbDistr( );

   /// This constructor can only be used for continuous parameters!
   /// Initialise the distribution with a covariance matrix. The
   /// mean is assumed to be the mean of an untruncated distribution.
   /// @param [in] mean       Mean / maximum likelihood factor values.
   /// @param [in] covmat     Covariance matrix of the pdf.
   MVNormalProbDistr( const std::vector<double>& mean,
                      const std::vector<std::vector<double> >& covmat );

   /// This constructor supports all ordinal parameters!
   /// Initialise the distribution from the parameter pdf data.
   /// @param [in] pdf        ParameterPdf from wich to use base, mean, covariances, min and max.
   MVNormalProbDistr( ParameterPdf const& pdf );

   /// Destructor
   virtual ~MVNormalProbDistr( );

   /// Initialise the distribution
   /// @param [in] base       Ordinal base values.
   /// @param [in] covmat     Covariance matrix for the continuous parameters of the pdf.
   void initialise( const std::vector<double>& base,
                    const std::vector<std::vector<double> >& covmat );

   unsigned int size() const { return m_base.size(); }
   unsigned int sizeCon() const { return m_mean.size(); }

   /// Set ordinal parameter bounds between which samples are accepted
   /// @param [in] min  lower ordinal sampling bounds
   /// @param [in] max  upper ordinal sampling bounds
   void setSamplingBounds( const std::vector<double>& min, const std::vector<double>& max );

   /// Obtain samples in p from the distribution using random generator rg,
   /// rejecting samples outside the [min,max] range.
   /// The number of samples to take is determined by the size of p.
   /// @param [in] rg Random generator to use
   /// @param [out] p the collection of samples
   /// @param [in] usePrior Indicates which PDF to use
   int sample( RandomGenerator& rg,
               std::vector<std::vector<double> >& p, bool usePrior = true ) const;

   /// Obtain a sample from the continuous part of the distribution,
   /// rejecting samples outside the given sample range.
   ///
   /// Note: the mean and the
   /// covariances do not account for the sample bounds.
   int priorSample( RandomGenerator& rg, std::vector<double>& p ) const;

   /// Obtain a "No Prior" sample, i.e. from a Uniform PDF.
   int noPriorSample( RandomGenerator& rg, std::vector<double>& p ) const;

   /// Get the (logarithm of the) probability density for parameter p.
   /// This method is used to calculate prior likelihoods in the
   /// statistical analysis of SUM.
   /// @param [in] p The parameter values
   /// @returns The (log of) the likelihood.
   double calcLogPriorProb( Parameter const& par ) const;

private:
   /// Get a uniform sample for the k'th parameter.
   double uniformSample( RandomGenerator& rg, size_t k ) const;

   /// Check the singular value decomposition of the covariance matrix.
   void check( const std::vector<std::vector<double> >& v,
               const std::vector<std::vector<double> >& covmat ) const;

   /// U matrix from singular value decomposition of the covariance matrix.
   std::vector<std::vector<double> > m_a;

   /// Diagonal matrix from singular value decomposition of the covariance matrix.
   std::vector<double> m_w;

   /// Mean values of the continuous parameters of the pdf.
   std::vector<double> m_mean;

   /// Base values of the ordinal parameters of the pdf.
   std::vector<double> m_base;

   /// Lower and upper sampling bounds of the ordinal parameters
   std::vector<double> m_sampleMin, m_sampleMax;

}; // class MVNormalProbDistr

} // namespace SUMlib

#endif // SUMLIB_MVNORMALPROBDISTR_H
