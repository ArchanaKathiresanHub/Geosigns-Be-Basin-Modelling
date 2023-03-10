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
   /// @param inpFileLineNum command position in input file
   void addCommand( const std::string & cmdID, const std::vector< std::string > & prms, size_t inpFileLineNum );

   /// @brief Create string representation of the given command
   /// @param pCmd casa command
   std::string toString( const CasaCmd * pCmd );

   /// @brief Go over all command in a queue and execute them for the
   void executeCommands( std::unique_ptr<casa::ScenarioAnalysis> & sa );

   /// @brief Get verbose level for command processing
   VerboseLevel verboseLevel() const { return m_msgLvl; }

   const std::vector< SharedCmdPtr> & cmdQueue() { return m_cmds; }

   /// @brief Get current command name. In case of exception, allow to inform user about wrong command name
   /// @return command name which is being executed now
   std::string curCmdName()                const { return m_cmdNames.size()      > m_curCmd ? m_cmdNames[m_curCmd]      : "";    }

   /// @brief Get current command position (line number) in input file
   /// @return command line number in input file
   size_t      curCmdInputFileLineNumber() const { return m_inpFileCmdPos.size() > m_curCmd ? m_inpFileCmdPos[m_curCmd] : 999999; }

   /// @brief Print short description of all available commands
   static void printHelpPage( const std::string & cmdName );

   // commands name
   static const char * nameAddCldApp;
   static const char * nameAddDesignPoint;
   static const char * nameAddObservable;
   static const char * nameAddVarPrm;
   static const char * nameBaseProject;
   static const char * nameCalibrateProject;
   static const char * nameExpDataTxt;
   static const char * nameGenerateBestMatchedCase;
   static const char * nameGenerateMultiOneD;
   static const char * nameImportOneDResults;
   static const char * nameLocation;
   static const char * nameRun;
   static const char * nameRunBaseCase;
   static const char * nameRunDataDigger;
   static const char * nameRunPipeline;
   static const char * nameScenarioID;
   static const char * nameSetFilterOneDResults;
   static const char * nameSetOneThreeDFromOneD;

private:
   std::vector< SharedCmdPtr>  m_cmds;          ///< Queue of commands
   VerboseLevel                m_msgLvl;        ///< How talkative should be CASADemo app

   size_t                      m_curCmd;        ///< Current command for execution (needed for error processing)
   std::vector<std::string>    m_cmdNames;      ///< command names as they were added
   std::vector<size_t>         m_inpFileCmdPos; ///< command position (line number) in input file
};

#endif // CASA_COMMANDER_H
