//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 
#ifndef CASA_CMD_RUN_RELOAD_H
#define CASA_CMD_RUN_RELOAD_H

#include "CasaCmd.h"

/// @brief Reload results after the run of generated cases and extract observables value
class CmdRunReload : public CasaCmd
{
public:
   /// @brief Constructor
   /// @param cmdPrms list of command parameters as set of strings
   CmdRunReload( CasaCommander & parent, const std::vector< std::string > & cmdPrms );

   virtual ~CmdRunReload() { ; }

   /// @brief Run command
   virtual void execute( std::unique_ptr<casa::ScenarioAnalysis> & sa );

   /// @brief Print help page about command purpose and it parameters
   /// @param cmdName - command name - how it is defined in CasaCommander
   static void printHelpPage( const char * cmdName );

protected:
   std::string m_locPath;
};

#endif // CASA_CMD_RUN_RELOAD_H
