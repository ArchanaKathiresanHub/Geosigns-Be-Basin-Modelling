//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 
#ifndef CASA_CMD_ADD_CAULDRON_APPLICATION_H
#define CASA_CMD_ADD_CAULDRON_APPLICATION_H

#include "CasaCmd.h"

/// @brief Define interface to any CASADemo command and keeps the common part
class CmdAddCldApp : public CasaCmd
{
public:
   /// @brief Constructor
   /// @param cmdPrms list of command parameters as set of strings
   CmdAddCldApp( CasaCommander & parent, const std::vector< std::string > & cmdPrms );

   virtual ~CmdAddCldApp() { ; }

   /// @brief Run command
   virtual void execute( casa::ScenarioAnalysis & sa );

private:
   unsigned int m_app;
};

#endif // CASA_CMD_ADD_CAULDRON_APPLICATION_H
