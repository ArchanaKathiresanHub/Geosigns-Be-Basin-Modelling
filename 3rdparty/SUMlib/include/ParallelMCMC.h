// Copyright 2014, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#ifndef SUMLIB_PARALLELMCMC_H
#define SUMLIB_PARALLELMCMC_H

// STD
#include <vector>

// SUMlib
#include "SUMlib.h"
#include "McmcParProtocol.h"

namespace SUMlib {

class INTERFACE_SUMLIB ParallelMCMC : public McmcParProtocolMaster
{

public:

   /// Constructor
   ///   @param seed                Seed for random number generator
   ///   @param ascs                Proxies
   ///   @param sampleSize          Number of required samples
   ///   @param pdf                 Pdf
   ///   @param constraints         Constraints for proxies
   ///   @param statistics          Storage for statistical results
   ///   @param krigingData         Kriging data for proxies
   ///   @param startcmd            Command-line for starting the slave
   ///   @param mcmcType            Type of algorithm: MC, MCMC or MCSolver
   ///   @param maxNbOfIterations   Maximum number of iterations
   ParallelMCMC( int seed, std::vector<McmcProxy*>const& ascs, unsigned int sampleSize,
                 const ParameterPdf& pdf, const ParameterBounds& constraints,
                 McmcStatistics& statistics, const KrigingData& krigingData,
                 const std::string& startcmd, IMcmc::McmcType mcmcType,
                 unsigned int maxNbOfIterations = 100 );

   /// Destructor
   virtual ~ParallelMCMC();

   // attributes
   static unsigned int  mcmcCounter;   ///< Class counter for generating unique ids
};

} // namespace SUMlib

#endif // SUMLIB_PARALLELMCMC_H
