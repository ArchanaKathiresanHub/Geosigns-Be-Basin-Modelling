// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#ifndef SUMLIB_MC_H
#define SUMLIB_MC_H

// STD
#include <vector>

// SUMlib
#include "McmcBase.h"
#include "SUMlib.h"

namespace SUMlib {

class INTERFACE_SUMLIB MC : public McmcBase {

public:

   // Constructor
   MC( int seed, std::vector<McmcProxy*>const& ascs, unsigned int sampleSize, const ParameterPdf & pdf, const ParameterBounds & constraints,
         McmcStatistics &statistics, unsigned int maxNbOfIterations = 100 ) : McmcBase( seed, ascs, sampleSize, pdf, constraints, statistics, maxNbOfIterations )
         {}

   // Destructor
   virtual ~MC(){}



   // Virtual functions
   virtual double calcLh( const std::vector<double>& ) const { return 0; }

   virtual bool acceptProposal( double logTransRatio, double& logAccRatio, RandomGenerator& rg ) const { return McmcBase::acceptProposalImpl_MCMC_MC( logTransRatio, logAccRatio, rg ); }

   virtual void calcRMSE(std::vector<double>&, std::vector<double>&, double&) const {}

   virtual unsigned int getNumActiveMeasurements( ) const { return 0; }


   // Impl functions
   virtual bool convergenceImpl( std::vector<std::vector<double> >& sampleVar, double& stddev, const double lambda, const unsigned int maxNbOfTrialsPerCycle );

   virtual void iterateOnceImpl();

   virtual void stepImpl( std::vector<double>& yNew, double& logLhNew, const size_t i );

};


} // namespace SUMlib

#endif // SUMLIB_MC_H
