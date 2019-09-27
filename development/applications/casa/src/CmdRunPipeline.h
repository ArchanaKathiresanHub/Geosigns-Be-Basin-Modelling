//
// Copyright (C) 2012-2019 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef CASA_CMD_RUNPIPELINE_H
#define CASA_CMD_RUNPIPELINE_H

#include "CasaCmd.h"

/// @brief Run the case in this folder using the pipeline

class CmdRunPipeline : public CasaCmd
{
public:
   /// @brief Constructor
   /// @param cmdPrms list of command parameters as set of strings
   CmdRunPipeline( CasaCommander & parent, const std::vector< std::string > & cmdPrms );

   virtual ~CmdRunPipeline() = default;

   /// @brief Run command
   virtual void execute( std::unique_ptr<casa::ScenarioAnalysis> & sa );

   /// @brief Print help page about command purpose and it parameters
   /// @param cmdName - command name - how it is defined in CasaCommander
   static void printHelpPage( const char * cmdName );

private:
   std::string m_cluster;
};

#endif // CASA_CMD_RUNPIPELINE_H
