// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#ifndef SUMLIB_MCMC_H
#define SUMLIB_MCMC_H

// STD
#include <vector>

// SUMlib
#include "mcmcBase.h"
#include "SUMlib.h"

namespace SUMlib {

class INTERFACE_SUMLIB MCMC : public mcmcBase {

public:

   /// Constructor
   MCMC( RandomGenerator &rg, std::vector<McmcProxy*>const& ascs, unsigned int sampleSize, const ParameterPdf & pdf, const ParameterBounds & constraints,
         McmcStatistics &statistics, unsigned int maxNbOfIterations = 100 ) : mcmcBase( rg, ascs, sampleSize, pdf, constraints, statistics, maxNbOfIterations )
         { initialise(); };

   /// Destructor
   virtual ~MCMC(){};

   // Virtual functions
   virtual bool acceptProposal( double logTransRatio, double& logAccRatio ) const { return mcmcBase::acceptProposalImpl_MCMC_MC( logTransRatio, logAccRatio ); }

   // Impl functions
   virtual bool convergenceImpl( vector<vector<double> >& sampleVar, double& stddev, const double lambda, const unsigned int maxNbOfTrialsPerCycle );

   virtual void stepImpl( vector<double>& yNew, double& logLhNew, const size_t i );

   virtual double proposeStepImpl1( const vector<double>& pStar, vector<double>& yStar, unsigned int i );

   virtual double proposeStepImpl2( const double logLhStar, const size_t i ){ return (logLhStar - m_logLh[i]); }

private:

   void initialise();
   
   void calcCatLikelihoods();

};


} // namespace SUMlib

#endif // SUMLIB_MCMC_H
