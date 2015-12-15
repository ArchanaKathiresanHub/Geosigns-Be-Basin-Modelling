//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 
#ifndef CASA_CMD_DOE_H
#define CASA_CMD_DOE_H

#include "CasaCmd.h"

/// @brief Define DoE algorithm for scenario and run it
class CmdDoE : public CasaCmd
{
public:
   /// @brief Constructor
   /// @param cmdPrms list of command parameters as set of strings
   CmdDoE( CasaCommander & parent, const std::vector< std::string > & cmdPrms );

   virtual ~CmdDoE() { ; }

   /// @brief Run command
   virtual void execute( std::auto_ptr<casa::ScenarioAnalysis> & sa );

protected:
   unsigned int   m_doeAlg;
   size_t         m_numExp;
};

#endif // CASA_CMD_DOE
