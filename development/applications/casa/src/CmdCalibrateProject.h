//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

#ifndef CASA_CMD_CALIBRATE_PROJECT_H
#define CASA_CMD_CALIBRATE_PROJECT_H

#include "CasaCmd.h"

/// @brief Create RunCase with best matched parameters set from Monte Carlo simulation
class CmdCalibrateProject : public CasaCmd
{
public:
   /// @brief Constructor
   /// @param cmdPrms list of command parameters as set of strings
   CmdCalibrateProject( CasaCommander & parent, const std::vector< std::string > & cmdPrms );

   virtual ~CmdCalibrateProject() { ; }

   /// @brief Run command
   virtual void execute( std::unique_ptr<casa::ScenarioAnalysis> & sa );

   /// @brief Print help page about command purpose and it parameters
   /// @param cmdName - command name - how it is distinguished in CasaCommander
   static void printHelpPage( const char * cmdName );

protected:
   std::string                  m_bmcName;   /// project name for calibrated case 
   std::string                  m_optimAlg;  /// name of optimization algorithm
   std::string                  m_cldVer;    /// version of cauldron simulator
};

#endif // CASA_CMD_CALIBRATE_PROJECT_H

