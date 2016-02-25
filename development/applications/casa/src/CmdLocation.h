//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 
#ifndef CASA_CMD_LOCATION_H
#define CASA_CMD_LOCATION_H

#include "CasaCmd.h"

/// @brief Define base projet for scenario
class CmdLocation : public CasaCmd
{
public:
   /// @brief Constructor
   /// @param cmdPrms list of command parameters as set of strings
   CmdLocation( CasaCommander & parent, const std::vector< std::string > & cmdPrms );

   virtual ~CmdLocation() { ; }

   /// @brief Run command
   virtual void execute( std::unique_ptr<casa::ScenarioAnalysis> & sa );

   /// @brief Get name of the project file which will be the base for the scenario analysis
   std::string casesLocation() const { return m_locPath;  }

protected:
   std::string m_locPath;
};

#endif // CASA_CMD_LOCATION_H
