//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 
#ifndef CASA_CMD_RUN_H
#define CASA_CMD_RUN_H

#include "CasaCmd.h"

/// @brief Run generated cases and extract observables value
class CmdRun : public CasaCmd
{
public:
   /// @brief Constructor
   /// @param cmdPrms list of command parameters as set of strings
   CmdRun( CasaCommander & parent, const std::vector< std::string > & cmdPrms );

   virtual ~CmdRun() { ; }

   /// @brief Run command
   virtual void execute( std::auto_ptr<casa::ScenarioAnalysis> & sa );

   /// @brief Print help page about command purpose and it parameters
   /// @param cmdName - command name - how it is defined in CasaCommander
   static void printHelpPage( const char * cmdName );

protected:
   std::string  m_cldVer;         // cauldron version
   std::string  m_cluster;        // cluster name
   size_t       m_maxPendingJobs; // max number of pending jobs for submitting to the cluster, to stick with fair share policy
};

#endif // CASA_CMD_RUN_H
