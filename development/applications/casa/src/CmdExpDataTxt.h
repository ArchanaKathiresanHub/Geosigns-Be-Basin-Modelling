//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 
#ifndef CASA_CMD_EXPORT_DATA_TXT_H
#define CASA_CMD_EXPORT_DATA_TXT_H

#include "CasaCmd.h"

/// @brief Define base projet for scenario
class CmdExpDataTxt : public CasaCmd
{
public:
   /// @brief Constructor
   /// @param cmdPrms list of command parameters as set of strings
   CmdExpDataTxt( CasaCommander & parent, const std::vector< std::string > & cmdPrms );

   virtual ~CmdExpDataTxt() { ; }

   /// @brief Run command
   virtual void execute( std::auto_ptr<casa::ScenarioAnalysis> & sa );

   /// @brief Print help page about command purpose and it parameters
   /// @param cmdName - command name - how it is defined in CasaCommander
   static void printHelpPage( const char * cmdName );

protected:
   void saveResults( const std::vector< std::vector<double> > & res );

   void exportParameters( std::auto_ptr<casa::ScenarioAnalysis> & sa );
   void exportRunCaseObs( std::auto_ptr<casa::ScenarioAnalysis> & sa );
   void exportEvalObserv( std::auto_ptr<casa::ScenarioAnalysis> & sa );
   void exportProxyQC(    std::auto_ptr<casa::ScenarioAnalysis> & sa );
 
   std::string                  m_whatToSave;   ///< DoEParameters/RunCasesObservables/ProxyEvalObservables
   std::string                  m_proxyName;    ///< Name of Response Surface proxy
   std::vector<std::string>     m_expList;      ///< List of DoEs or data files name with parameters value
   std::string                  m_dataFileName; ///< Output file name to save results of proxy evaluation
};

#endif // CASA_CMD_EXPORT_DATA_TXT_H
