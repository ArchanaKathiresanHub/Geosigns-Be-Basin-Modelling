//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 
#ifndef CASA_CMD_PLOT_RS_PROXY_QC_H
#define CASA_CMD_PLOT_RS_PROXY_QC_H

#include "CasaCmd.h"

/// @brief Define base projet for scenario
class CmdPlotRSProxyQC : public CasaCmd
{
public:
   /// @brief Constructor
   /// @param cmdPrms list of command parameters as set of strings
   CmdPlotRSProxyQC( CasaCommander & parent, const std::vector< std::string > & cmdPrms );

   virtual ~CmdPlotRSProxyQC() { ; }

   /// @brief Run command
   virtual void execute( std::auto_ptr<casa::ScenarioAnalysis> & sa );

protected:
   std::string                  m_mFileName;
   std::string                  m_proxyName;    ///< Name of Response Surface proxy
   std::vector<std::string>     m_caseList;     ///< List of DoEs or data files name with parameters value (usually which was used to build proxy)
   std::vector<std::string>     m_testCaseList; ///< List of DoEs or data files name with parameters value (test set which was not used to build proxy)
};

#endif // CASA_CMD_PLOT_RS_PROXY_QC_H
