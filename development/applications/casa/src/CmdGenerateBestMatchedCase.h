//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

#ifndef CASA_CMD_GENERATE_BEST_MATCHED_CASE_H
#define CASA_CMD_GENERATE_BEST_MATCHED_CASE_H

#include "CasaCmd.h"

/// @brief Create RunCase with best matched parameters set from Monte Carlo simulation
class CmdGenerateBestMatchedCase : public CasaCmd
{
public:
   /// @brief Constructor
   /// @param cmdPrms list of command parameters as set of strings
   CmdGenerateBestMatchedCase( CasaCommander & parent, const std::vector< std::string > & cmdPrms );

   virtual ~CmdGenerateBestMatchedCase() { ; }

   /// @brief Run command
   virtual void execute( std::unique_ptr<casa::ScenarioAnalysis> & sa );

   /// @brief Print help page about command purpose and it parameters
   /// @param cmdName - command name - how it is distinguished in CasaCommander
   static void printHelpPage( const char * cmdName );

protected:
   std::string                  m_bmcName;   /// project name for case
   size_t                       m_sampleNum; /// MC sample number for generating
};

#endif // CASA_CMD_CREATE_RESPONSE_H
