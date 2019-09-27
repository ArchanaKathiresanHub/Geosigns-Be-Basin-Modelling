//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 
#ifndef CASA_CMD_CREATE_RESPONSE_H
#define CASA_CMD_CREATE_RESPONSE_H

#include "CasaCmd.h"

/// @brief Define base projet for scenario
class CmdCreateResponse : public CasaCmd
{
public:
   /// @brief Constructor
   /// @param cmdPrms list of command parameters as set of strings
   CmdCreateResponse( CasaCommander & parent, const std::vector< std::string > & cmdPrms );

   virtual ~CmdCreateResponse() { ; }

   /// @brief Run command
   virtual void execute( std::unique_ptr<casa::ScenarioAnalysis> & sa );

protected:
   std::string                  m_proxyName;
   std::vector<std::string>     m_doeList;
   long                         m_respSurfOrder;
   unsigned int                 m_krType;
};

#endif // CASA_CMD_CREATE_RESPONSE_H
