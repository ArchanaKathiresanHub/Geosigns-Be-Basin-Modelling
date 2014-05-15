//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file MCSolverImpl.C
/// @brief This file keeps implementation of API for performing Monte Carlo simulation


#include "MCSolverImpl.h"

#include <stdexcept>

namespace casa
{

MCSolverImpl::MCSolverImpl()
{
   ;
}

MCSolverImpl::~MCSolverImpl()
{
   ;
}

// Add Case to set
ErrorHandler::ReturnCode MCSolverImpl::runSimulation( int numerOfSampl, const RSProxy & proxy, const VarSpace & varPrmsSet )
{
   return ReportError( MCSolverError, "scheduleCase() not implemented yet" );
}

// Execute all scheduled cases
std::vector<RunCase*> & MCSolverImpl::getSimulationResults()
{

   throw std::runtime_error( "getSimulationResults() Not implemented yet" );
   return m_results;
}

}
