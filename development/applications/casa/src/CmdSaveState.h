//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 
#ifndef CASA_CMD_SAVE_STATE_H
#define CASA_CMD_SAVE_STATE_H

#include "CasaCmd.h"

/// @brief Define base projet for scenario
class CmdSaveState : public CasaCmd
{
public:
   /// @brief Constructor
   /// @param cmdPrms list of command parameters as set of strings
   CmdSaveState( CasaCommander & parent, const std::vector< std::string > & cmdPrms );

   virtual ~CmdSaveState() { ; }

   /// @brief Run command
   virtual void execute( std::auto_ptr<casa::ScenarioAnalysis> & sa );

protected:
   std::string m_fileName;
   std::string m_fileType;
};

#endif // CASA_CMD_SAVE_STATE_H
