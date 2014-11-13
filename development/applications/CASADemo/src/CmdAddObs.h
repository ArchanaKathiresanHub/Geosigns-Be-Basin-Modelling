//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 
#ifndef CASA_CMD_ADD_OBSERVABLE_H
#define CASA_CMD_ADD_OBSERVABLE_H

#include "CasaCmd.h"

/// @brief Add new observable definition to scenario
class CmdAddObs : public CasaCmd
{
public:
   /// @brief Constructor
   /// @param cmdPrms list of command parameters as set of strings
   CmdAddObs( CasaCommander & parent, const std::vector< std::string > & cmdPrms );

   virtual ~CmdAddObs() { ; }

   /// @brief Run command
   virtual void execute( std::auto_ptr<casa::ScenarioAnalysis> & sa );

protected:
};

#endif // CASA_CMD_ADD_OBSERVABLE_H
