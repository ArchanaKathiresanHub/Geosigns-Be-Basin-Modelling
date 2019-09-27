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

/// @brief Define base project for scenario
class CmdExpDataTxt : public CasaCmd
{
public:
   /// @brief Constructor
   /// @param cmdPrms list of command parameters as set of strings
   CmdExpDataTxt(CasaCommander& parent, const std::vector<std::string>& cmdPrms);

   virtual ~CmdExpDataTxt();

   /// @brief Run command
   virtual void execute(std::unique_ptr<casa::ScenarioAnalysis>& scenario);

   /// @brief Print help page about command purpose and it parameters
   /// @param cmdName - command name - how it is defined in CasaCommander
   static void printHelpPage(const char * cmdName);

private:
   template<class T>
   void saveResults(const std::vector<std::vector<T>>& res);

   void exportParameters(std::unique_ptr<casa::ScenarioAnalysis>& scenario);
   void exportRunCaseObs(std::unique_ptr<casa::ScenarioAnalysis>& scenario);
   void exportDoeIndices(std::unique_ptr<casa::ScenarioAnalysis>& scenario);
   void exportEvalObserv(std::unique_ptr<casa::ScenarioAnalysis>& scenario);
   void exportProxyQC(std::unique_ptr<casa::ScenarioAnalysis>& scenario);
   void exportRSPQuality(std::unique_ptr<casa::ScenarioAnalysis>& scenario);
   void exportMCResults(std::unique_ptr<casa::ScenarioAnalysis>& scenario);
   const std::vector<std::vector<double>> rsQualityR2AndR2adj(std::unique_ptr<casa::ScenarioAnalysis>& scenario) const;
   const std::vector<double> rsQualityQ2(std::unique_ptr<casa::ScenarioAnalysis>& scenario);

   std::string                  m_whatToSave;   ///< DoEParameters/RunCasesObservables/ProxyEvalObservables
   std::string                  m_proxyName;    ///< Name of Response Surface proxy
   std::vector<std::string>     m_expList;      ///< List of DoEs or data files name with parameters value
   std::string                  m_dataFileName; ///< Output file name to save results of proxy evaluation
};

#endif // CASA_CMD_EXPORT_DATA_TXT_H
