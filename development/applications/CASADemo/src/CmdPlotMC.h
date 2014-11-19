//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 
#ifndef CASA_CMD_PLOT_MC_H
#define CASA_CMD_PLOT_MC_H

#include "CasaCmd.h"

/// @brief Define base projet for scenario
class CmdPlotMC : public CasaCmd
{
public:
   /// @brief Constructor
   /// @param cmdPrms list of command parameters as set of strings
   CmdPlotMC( CasaCommander & parent, const std::vector< std::string > & cmdPrms );

   virtual ~CmdPlotMC() { ; }

   /// @brief Run command
   virtual void execute( std::auto_ptr<casa::ScenarioAnalysis> & sa );

protected:
   std::string m_mFileName;
};

#endif // CASA_CMD_PLOT_MC_H
