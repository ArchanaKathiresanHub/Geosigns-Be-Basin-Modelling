//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 
#ifndef CASA_CMD_RUN_MC_H
#define CASA_CMD_RUN_MC_H

#include "CasaCmd.h"

/// @brief Define base projet for scenario
class CmdRunMC : public CasaCmd
{
public:
   /// @brief Constructor
   /// @param cmdPrms list of command parameters as set of strings
   CmdRunMC( CasaCommander & parent, const std::vector< std::string > & cmdPrms );

   virtual ~CmdRunMC() { ; }

   /// @brief Run command
   virtual void execute( std::auto_ptr<casa::ScenarioAnalysis> & sa );

protected:
   std::string    m_proxyName;     ///< name of the response proxy which will be used for evaluation
   unsigned int   m_mcAlg;         ///< monte carlo algorithm - MC/MCMC/MCSolver
   unsigned int   m_samplesNum;    ///< number of samples
   unsigned int   m_maxStepsNum;   ///< max number of steps
   unsigned int   m_proxyUsage;    ///< how to evaluate proxy - POLYNOMIAL/SMART_KRIGING/FULL_KRIGING
   unsigned int   m_varPrmPDFEval; ///< how to evaluate variable parameters PDF - NoPrior/MarginalPrior/MultivariatePrior
   double         m_stdDevFactor;  ///< standard deviation factor which is used in MC/MCMC RMSE calculation


 

};

#endif // CASA_CMD_RUN_MC_H
