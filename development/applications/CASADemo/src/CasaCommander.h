//                     7                                                 
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

#ifndef CASA_COMMANDER_H
#define CASA_COMMANDER_H

#include <vector>
#include <string>

#include "CfgFileParser.h"
#include "CasaCmd.h"

namespace casa
{
   class ScenarioAnalysis;
}

/// @brief Implements a factory and keeps a queur for CasaCmd. 
///        Also keeps some global data variable to interchange data between commands
class CasaCommander
{
public:
   enum VerboseLevel
   {
      Quiet,
      Minimal,
      Detailed
   };

   CasaCommander();
   ~CasaCommander() { ; }

   /// @brief Create and add the new command to the queue
   /// @param cmdID command ID defined in CfgFileParser
   /// @param prms list of command parameters
   void addCommand( const std::string & cmdID, const std::vector< std::string > & prms );

   /// @brief Go over all command in a queue and execute them for the
   void executeCommands( std::auto_ptr<casa::ScenarioAnalysis> & sa );

   /// @brief Get verbose level for command processing
   VerboseLevel verboseLevel() const { return m_msgLvl; }

   const std::vector< SharedCmdPtr> & cmdQueue() { return m_cmds; }

private:
   std::vector< SharedCmdPtr>  m_cmds;         ///< Queue of commands
   VerboseLevel                m_msgLvl;       ///< How talkative should be CASADemo app
};

#endif // CASA_COMMANDER_H
