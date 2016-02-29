//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 
#ifndef CASA_CMD_PLOT_P10_P90_H
#define CASA_CMD_PLOT_P10_P90_H

#include "CasaCmd.h"

/// @brief Define base projet for scenario
class CmdPlotP10P90 : public CasaCmd
{
public:
   /// @brief Constructor
   /// @param cmdPrms list of command parameters as set of strings
   CmdPlotP10P90( CasaCommander & parent, const std::vector< std::string > & cmdPrms );

   virtual ~CmdPlotP10P90() { ; }

   /// @brief Run command
   virtual void execute( std::unique_ptr<casa::ScenarioAnalysis> & sa );

   /// @brief Print help page about command purpose and it parameters
   /// @param cmdName - command name - how it is distinguished in CasaCommander
   static void printHelpPage( const char * cmdName );

protected:
   std::string m_mFileName;
};

#endif // CASA_CMD_PLOT_P10_P90_H
