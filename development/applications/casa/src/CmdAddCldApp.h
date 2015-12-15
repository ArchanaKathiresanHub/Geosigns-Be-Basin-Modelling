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
   virtual void execute( std::auto_ptr<casa::ScenarioAnalysis> & sa );

   /// @brief Print help page about command purpose and it parameters
   /// @param cmdName - command name - how it is distinguished in CasaCommander
   static void printHelpPage( const char * cmdName );

private:
   unsigned int m_app;           ///< application name
   int          m_cpus;          ///< number of CPUS for the application
   size_t       m_maxRunLimMin;  ///< hard limit for application execution time [Minutes]
};

#endif // CASA_CMD_ADD_CAULDRON_APPLICATION_H
