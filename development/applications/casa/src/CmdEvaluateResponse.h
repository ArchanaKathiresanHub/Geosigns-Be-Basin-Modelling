//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 
#ifndef CASA_CMD_EVALUATE_RESPONSE_H
#define CASA_CMD_EVALUATE_RESPONSE_H

#include "CasaCmd.h"

namespace casa
{
   class RunCase;
}

/// @brief Define base projet for scenario
class CmdEvaluateResponse : public CasaCmd
{
public:
   /// @brief Constructor
   /// @param cmdPrms list of command parameters as set of strings
   CmdEvaluateResponse( CasaCommander & parent, const std::vector< std::string > & cmdPrms );

   virtual ~CmdEvaluateResponse() { ; }

   /// @brief Run command
   virtual void execute( std::auto_ptr<casa::ScenarioAnalysis> & sa );

   /// @brief Add to RunCase set new cases for the given list of experiments
   /// @param sa scenario analysis object which keeps all data
   /// @param[out] rcs vector of RunCase objects which will be extended with new objects
   /// @param expList list of DoE names or external data files name with parameters value
   /// @param sizePerExp vector of each experiment size
   static void createRunCasesSet( std::auto_ptr<casa::ScenarioAnalysis> & sa
                                , std::vector<casa::RunCase *>          & rcs
                                , const std::vector<std::string>        & expList
                                , std::vector<size_t>                   & sizePerExp
                                );

protected:
   std::string                  m_proxyName;    ///< Name of Response Surface proxy
   std::vector<std::string>     m_expList;      ///< List of DoEs or data files name with parameters value
   std::string                  m_dataFileName; ///< Output file name to save results of proxy evaluation

};

#endif // CASA_CMD_EVALUATE_RESPONSE_H
