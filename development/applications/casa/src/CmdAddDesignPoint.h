//
// Copyright (C) 2019 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#ifndef CASA_CMD_ADD_DESIGN_POINT_H
#define CASA_CMD_ADD_DESIGN_POINT_H

#include "CasaCmd.h"

/// @brief Add a single design point to the design of experiments
class CmdAddDesignPoint : public CasaCmd
{
public:
   /// @brief Constructor
   /// @param cmdPrms list of command parameters as set of strings
   CmdAddDesignPoint( CasaCommander & parent, const std::vector< std::string > & cmdPrms );

   virtual ~CmdAddDesignPoint() = default;

   /// @brief Run command
   virtual void execute( std::unique_ptr<casa::ScenarioAnalysis> & sa );

   /// @brief Print help page about command purpose and it parameters
   /// @param cmdName - command name - how it is defined in CasaCommander
   static void printHelpPage( const char * cmdName );

private:
   std::vector<double> m_parameterValues;
};

#endif // CASA_CMD_ADD_DESIGN_POINT_H
