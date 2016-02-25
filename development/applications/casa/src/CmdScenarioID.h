//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 
#ifndef CASA_CMD_SCENARIO_ID_H
#define CASA_CMD_SCENARIO_ID_H

#include "CasaCmd.h"

/// @brief Define base projet for scenario
class CmdScenarioID : public CasaCmd
{
public:
   /// @brief Constructor
   /// @param cmdPrms list of command parameters as set of strings
   CmdScenarioID( CasaCommander & parent, const std::vector< std::string > & cmdPrms );

   virtual ~CmdScenarioID() { ; }

   /// @brief Run command
   virtual void execute( std::auto_ptr<casa::ScenarioAnalysis> & sa );

   /// @brief Get name of the project file which will be the base for the scenario analysis
   std::string scenarioID() const { return m_id;  }

   /// @brief Print help page about command purpose and it parameters
   /// @param cmdName - command name - how it is defined in CasaCommander
   static void printHelpPage( const char * cmdName );

protected:
   std::string m_id;
};

#endif // CASA_CMD_SCENARIO_ID_H
