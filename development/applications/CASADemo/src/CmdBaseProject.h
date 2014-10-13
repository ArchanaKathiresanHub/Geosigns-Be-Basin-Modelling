//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 
#ifndef CASA_CMD_BASE_PROJECT_H
#define CASA_CMD_BASE_PROJECT_H

#include "CasaCmd.h"

/// @brief Define base projet for scenario
class CmdBaseProject : public CasaCmd
{
public:
   /// @brief Constructor
   /// @param cmdPrms list of command parameters as set of strings
   CmdBaseProject( CasaCommander & parent, const std::vector< std::string > & cmdPrms );

   virtual ~CmdBaseProject() { ; }

   /// @brief Run command
   virtual void execute( casa::ScenarioAnalysis & sa );

   /// @brief Get name of the project file which will be the base for the scenario analysis
   std::string baseProjectName() const { return m_baseProjectName;  }

protected:
   std::string m_baseProjectName;
};

#endif // CASA_CMD_BASE_PROJECT_H
