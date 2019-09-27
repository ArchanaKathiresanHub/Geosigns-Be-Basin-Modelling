//                                                                      
// Copyright (C) 2012-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 
#ifndef CASA_CMD_ADD_VARIABLE_PARAMETER_H
#define CASA_CMD_ADD_VARIABLE_PARAMETER_H

#include "CasaCmd.h"

/// @brief Add new variable parameter to scenario
class CmdAddVarPrm : public CasaCmd
{
public:
   /// @brief Constructor
   /// @param cmdPrms list of command parameters as set of strings
   CmdAddVarPrm( CasaCommander & parent, const std::vector< std::string > & cmdPrms );

   virtual ~CmdAddVarPrm() = default;

   /// @brief Run command
   virtual void execute( std::unique_ptr<casa::ScenarioAnalysis> & sa );

   /// @brief Print help page about command purpose and it parameters
   /// @param cmdName - command name - how it is defined in CasaCommander
   static void printHelpPage( const char * cmdName );

   /// @brief Get the name of the parameter
   virtual std::string prmName() const { return m_prmName; }

protected:
   std::string m_prmName; ///< optional varparameter name
};

#endif // CASA_CMD_ADD_VARIABLE_PARAMETER_H
