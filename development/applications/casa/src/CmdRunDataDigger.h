//
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#ifndef CASA_CMD_RUN_DATADIGGER_H
#define CASA_CMD_RUN_DATADIGGER_H

#include "CasaCmd.h"

/// @brief Run generated cases and extract observables value
class CmdRunDataDigger : public CasaCmd
{
public:
   /// @brief Constructor
   /// @param commandParameters list of command parameters as set of strings
   CmdRunDataDigger( CasaCommander & parent, const std::vector<std::string>& commandParameters );

   virtual ~CmdRunDataDigger() {}

   /// @brief Run command
   virtual void execute( std::unique_ptr<casa::ScenarioAnalysis>& scenarioAnalysis );

   /// @brief Print help page about command purpose and it parameters
   /// @param commandName - command name - how it is defined in CasaCommander
   static void printHelpPage( const char* commandName );

};

#endif // CASA_CMD_RUN_DATADIGGER_H
