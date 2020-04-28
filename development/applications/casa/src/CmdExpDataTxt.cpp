//
// Copyright (C) 2012-2017 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "CasaCommander.h"
#include "CmdExpDataTxt.h"

#include "casaAPI.h"
#include "RSProxyQualityCalculator.h"
#include "RunCase.h"

#include "LogHandler.h"

#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <fstream>

namespace
{

// Auxiliary functions and variables

void extractObservableValues(std::vector<double>& result, const casa::RunCase* cs)
{
  // go over all observables and calculate PrxVal for each observable
  for (size_t j = 0; j < cs->observablesNumber(); ++j)
  {
    const casa::ObsValue* obv = cs->obsValue(j);

    if (!obv->isDouble())
    {
      continue;
    }

    const std::vector<double>& vals = obv->asDoubleArray();
    result.insert(result.end(), vals.begin(), vals.end());
  }
}

void extractParametersValues(std::vector<double>& result, const casa::RunCase* cs)
{
  for (size_t j = 0; j < cs->parametersNumber(); ++j)
  {
    const casa::Parameter* prm = cs->parameter(j).get();

    if (!prm || !prm->parent()) continue;

    switch (prm->parent()->variationType())
    {
      case casa::VarParameter::Continuous:
      case casa::VarParameter::Discrete:
        {
          const std::vector<double>& vals = prm->asDoubleArray();
          result.insert(result.end(), vals.begin(), vals.end());
        }
        break;

      case casa::VarParameter::Categorical:
        result.push_back(prm->asInteger());
        break;

      default: assert(false); break;
    }
  }
}

template<class T>
void writeOutputStream(const T myRes, std::ofstream & ofs);

template<> void writeOutputStream<double>(const double myRes, std::ofstream & ofs)
{
  ofs << std::scientific << std::setprecision(8) << std::setfill('0') << myRes << " ";
}

template<> void writeOutputStream<size_t>(const size_t myRes, std::ofstream & ofs)
{
  ofs << myRes << " ";
}

} // namespace

CmdExpDataTxt::CmdExpDataTxt(CasaCommander & parent, const std::vector<std::string>& cmdPrms)
  : CasaCmd(parent, cmdPrms)
{
  if (m_prms.size() < 2)
  {
    throw ErrorHandler::Exception(ErrorHandler::RSProxyError) << "Wrong parameters number: "
                                                              << m_prms.size() << " (expected 2 or more) in data export command";
  }

  m_whatToSave   = m_prms[0];
  m_dataFileName = m_prms[1].empty() ? (m_whatToSave + ".dat") : m_prms[1];

  if (m_whatToSave == "DoEParameters")
  {
    // convert list of DoEs or data files like: "Tornado,BoxBenken" into array of names
    if (m_prms.size() > 2) { m_expList = CfgFileParser::list2array(m_prms[2], ','); }
  }
  else if ( m_whatToSave == "RunCasesObservables" || m_whatToSave == "DoeIndices" )
  {
    if (m_prms.size() > 2)
    {
      throw ErrorHandler::Exception(ErrorHandler::UndefinedValue) << "Wrong number of parameters given. Expected 2 parameters, "
                                                                  << m_prms.size() << " provided!";
    }
  }
  else if (m_whatToSave == "ProxyEvalObservables" ||
           m_whatToSave == "ProxyQC" ||
           m_whatToSave == "ProxyEvaluateQuality"
           )
  {
    if (m_prms.size() > 2) { m_expList = CfgFileParser::list2array(m_prms[2], ','); }
    if (m_prms.size() > 3) { m_proxyName = m_prms[3]; }

    if (m_proxyName.empty()) throw ErrorHandler::Exception(ErrorHandler::UndefinedValue) << "No proxy name was given";
  }
  else if (m_whatToSave == "MCResults")
  {
    // no parameters to be parsed
  }
  else
  {
    throw ErrorHandler::Exception(ErrorHandler::RSProxyError) << "Unknown command parameter: " <<  m_whatToSave;
  }
}

CmdExpDataTxt::~CmdExpDataTxt()
{
}

void CmdExpDataTxt::execute(std::unique_ptr<casa::ScenarioAnalysis>& scenario)
{
  LogHandler(LogHandler::INFO_SEVERITY) << "Exporting CASA results for " << m_whatToSave << " to : " << m_dataFileName << "...";

  if (     m_whatToSave == "DoEParameters"       ) { exportParameters(scenario); }
  else if (m_whatToSave == "DoeIndices"          ) { exportDoeIndices(scenario); }
  else if (m_whatToSave == "RunCasesObservables" ) { exportRunCaseObs(scenario); }
  else if (m_whatToSave == "ProxyEvalObservables") { exportEvalObserv(scenario); }
  else if (m_whatToSave == "ProxyQC"             ) { exportProxyQC(   scenario); }
  else if (m_whatToSave == "ProxyEvaluateQuality") { exportRSPQuality(scenario); }
  else if (m_whatToSave == "MCResults"           ) { exportMCResults( scenario); }

  LogHandler(LogHandler::INFO_SEVERITY) << "Exporting CASA results succeeded.";
}

void CmdExpDataTxt::printHelpPage(const char* cmdName)
{
  std::cout << "  " << cmdName << R"(<what> <fileName> [<DoEList>] [<proxyName>]

               Export in plane txt file scenario data. This command goes over all run cases from the list of specified DoEs
               (if list wasn't specified - over all DoEs in scenario) and export numerical data for:
                DoEParameters       : generated by DoE parameters value
                RunCasesObservables : extracted from simulations observables value
                ProxyEvalObservables: evaluated from the given response surface observables value
                ProxyEvaluateQuality: evaluate proxy response surface quality measures R2, R2adj, and Q2
                ProxyQC:              relation of simulated to evaluated observables value
                MCResults:            variable parameters and observables value from MC/MCMC simulation

                Where:
                what     - One of the above options
                fileName - file name to export data
                DoEList  - (Optional) comma separated list of DoEs name
                proxyName - must be defined for ProxyEvalObservables/ProxyQC/ProxyEvaluateQuality. Proxy name which will be used to calculate observables value

                Example:
                )" << cmdName << R"("ProxyQC" "datafile.txt"  "FullFactorial,Tornado" "FirstOrderProxy"\n)";
}

void CmdExpDataTxt::exportParameters(std::unique_ptr<casa::ScenarioAnalysis>& scenario)
{
  LogHandler(LogHandler::INFO_SEVERITY) << "Exporting parameters value calculated by DoEs to " << m_dataFileName << "...";

  casa::RunCaseSet & doeCaseSet = scenario->doeCaseSet();
  std::vector<std::string> doeList = m_expList.empty() ? doeCaseSet.experimentNames() : m_expList;

  std::vector< std::vector<double>> results;
  doeCaseSet.filterByDoeList(doeList);

  for (size_t c = 0; c < doeCaseSet.size(); ++c)
  {
    results.push_back(std::vector<double>());

    // go over all parameters
    extractParametersValues(results.back(), doeCaseSet.runCase(c));
  }
  saveResults(results);
}

template<class T>
void CmdExpDataTxt::saveResults(const std::vector< std::vector<T>>& res)
{
  std::ofstream ofs;

  ofs.open(m_dataFileName.c_str(), std::ios_base::out | std::ios_base::trunc);

  if (ofs.fail()) throw ErrorHandler::Exception(ErrorHandler::IoError) << "Can not open file for writing: " << m_dataFileName;

  for (size_t i = 0; i < res.size(); ++i)
  {
    for (size_t j = 0; j < res[i].size(); ++j)
    {
      writeOutputStream(res[i][j], ofs);
    }
    ofs << std::endl;
  }
  ofs.close();
}

void CmdExpDataTxt::exportRunCaseObs(std::unique_ptr<casa::ScenarioAnalysis>& scenario)
{
  LogHandler(LogHandler::INFO_SEVERITY) << "Export observables value calculated in DoEs runs to " << m_dataFileName << "...";

  casa::RunCaseSet & doeCaseSet = scenario->doeCaseSet();
  std::vector<std::string> doeList = m_expList.empty() ? doeCaseSet.experimentNames() : m_expList;

  std::vector<std::vector<double>> runCasesObservables;
  doeCaseSet.filterByDoeList(doeList);

  for ( size_t c = 0; c < doeCaseSet.size(); ++c )
  {
    runCasesObservables.push_back(std::vector<double>());
    extractObservableValues(runCasesObservables.back(), doeCaseSet.runCase(c));
  }

  saveResults(runCasesObservables);
}

void CmdExpDataTxt::exportDoeIndices(std::unique_ptr<casa::ScenarioAnalysis>& scenario)
{
  LogHandler(LogHandler::INFO_SEVERITY) << "Export indices of all the unique (non-overlapping) Doe points correponding to matrix of run case observables."
                                           "The indices are saved to " << m_dataFileName << "...";

  casa::RunCaseSet & doeCaseSet = scenario->doeCaseSet();
  std::vector<std::string> doeList = m_expList.empty() ? doeCaseSet.experimentNames() : m_expList;

  std::vector<std::vector<size_t>> experimentsIndices;
  for ( const std::string & doeName : doeList )
  {
    experimentsIndices.push_back( doeCaseSet.experimentIndexSet( doeName ) );
  }

  saveResults(experimentsIndices);
}


void CmdExpDataTxt::exportEvalObserv(std::unique_ptr<casa::ScenarioAnalysis>& scenario)
{
  LogHandler(LogHandler::INFO_SEVERITY) << "Exporting observables value for response surface proxy " << m_proxyName << " to " << m_dataFileName << "...";

  std::vector<std::string> doeList = m_expList.empty() ? scenario->doeCaseSet().experimentNames() : m_expList;

  casa::RunCaseSet & doeCaseSet = scenario->doeCaseSet();
  doeCaseSet.filterByDoeList(doeList);

  casa::RSProxy* proxy = scenario->rsProxySet().rsProxy(m_proxyName.c_str());
  if (!proxy)
  {
    throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "Unknown proxy name:" << m_proxyName;
  }

  std::vector<std::vector<double>> proxyEvaluationObservables(doeCaseSet.size());

  size_t i = 0;
  for (size_t iRunCase = 0; iRunCase < doeCaseSet.size(); ++iRunCase)
  {
    std::shared_ptr<casa::RunCase> runCase = doeCaseSet.runCase(iRunCase)->shallowCopy();

    if (ErrorHandler::NoError != proxy->evaluateRSProxy(*runCase))
    {
      throw ErrorHandler::Exception(proxy->errorCode()) << proxy->errorMessage();
    }

    extractObservableValues(proxyEvaluationObservables[i], runCase.get());
    ++i;
  }

  saveResults(proxyEvaluationObservables);
}

void CmdExpDataTxt::exportProxyQC(std::unique_ptr<casa::ScenarioAnalysis>& scenario)
{
  LogHandler(LogHandler::INFO_SEVERITY) << "Exporting QC data for response surface proxy " << m_proxyName << " to " << m_dataFileName << "...";

  casa::RunCaseSet & doeCaseSet = scenario->doeCaseSet();
  std::vector<std::string> doeList = m_expList.empty() ? doeCaseSet.experimentNames() : m_expList;

  // Search for given proxy name in the set of calculated proxies
  casa::RSProxy* proxy = scenario->rsProxySet().rsProxy(m_proxyName.c_str());
  // call response evaluation
  if (!proxy)
  {
    throw ErrorHandler::Exception(ErrorHandler::NonexistingID) << "Unknown proxy name:" << m_proxyName;
  }

  doeCaseSet.filterByDoeList(doeList);
  std::vector< std::vector<double>> results(doeCaseSet.size());

  size_t i = 0;
  for (size_t iRunCase = 0; iRunCase < doeCaseSet.size(); ++iRunCase)
  {
    const casa::RunCase* runCase = doeCaseSet.runCase(iRunCase);
    std::shared_ptr<casa::RunCase> proxyRunCase = runCase->shallowCopy();

    if (ErrorHandler::NoError != proxy->evaluateRSProxy(*proxyRunCase) )
    {
      throw ErrorHandler::Exception(proxy->errorCode()) << proxy->errorMessage();
    }

    // go over all observables and calculate PrxVal/SimVal for each observable
    for (size_t j = 0; j < proxyRunCase->observablesNumber(); ++j)
    {
      const casa::ObsValue* obsProxy = proxyRunCase->obsValue(j);
      const casa::ObsValue* obsSim = runCase->obsValue(j);

      if (!obsProxy->isDouble() || !obsSim->isDouble()) continue;

      const std::vector<double>& valsPrx = obsProxy->asDoubleArray();
      const std::vector<double>& valsSim = obsSim->asDoubleArray();

      assert(valsSim.size() == valsPrx.size());

      for (size_t k = 0; k < valsPrx.size(); ++k)
      {
        double rel = std::fabs(valsSim[k]) < 1e-20 ? (std::abs(valsPrx[k]) < 1e-20 ? 1.0 : Utilities::Numerical::IbsNoDataValue) :
                                                     (valsPrx[k] / valsSim[k]);
        results[i].push_back(rel);
      }
    }
    ++i;
  }
  saveResults(results);
}

void CmdExpDataTxt::exportRSPQuality(std::unique_ptr<casa::ScenarioAnalysis>& scenario)
{
  std::vector<std::vector<double>> results = rsQualityR2AndR2adj(scenario);
  results.push_back(rsQualityQ2(scenario));

  saveResults(results);

  LogHandler( LogHandler::INFO_SEVERITY ) << "Evaluation of response surface proxy quality measures R2, R2adj, and Q2 was succeeded";
}

const std::vector<std::vector<double>> CmdExpDataTxt::rsQualityR2AndR2adj(std::unique_ptr<casa::ScenarioAnalysis>& scenario) const
{
  LogHandler(LogHandler::INFO_SEVERITY) << "Calculating quality measures R2 and R2Adj for response surface proxy " << m_proxyName << "...";

  casa::RSProxyQualityCalculator proxyQualityCalculator{*scenario};
  return proxyQualityCalculator.calculateR2AndR2adj(m_proxyName, m_expList);
}

const std::vector<double> CmdExpDataTxt::rsQualityQ2(std::unique_ptr<casa::ScenarioAnalysis>& scenario)
{
  LogHandler(LogHandler::INFO_SEVERITY) << "Calculating quality measure Q2 for response surface proxy " << m_proxyName << "...";

  casa::RSProxyQualityCalculator proxyQualityCalculator{*scenario};

  return proxyQualityCalculator.calculateQ2(m_proxyName, m_expList);
}

void CmdExpDataTxt::exportMCResults(std::unique_ptr<casa::ScenarioAnalysis>& scenario)
{
  LogHandler(LogHandler::INFO_SEVERITY) << "Exporting MC/MCMC to " << m_dataFileName << "...";

  std::vector< std::vector<double>> results;
  // export MC samples
  const casa::MonteCarloSolver::MCResults & mcSamples = scenario->mcSolver().getSimulationResults();

  for (size_t i = 0; i < mcSamples.size(); ++i)
  {
    results.push_back(std::vector<double>());

    std::vector<double>& rsmpl = results.back();
    rsmpl.push_back(static_cast<double>(i));                //sample num
    rsmpl.push_back(mcSamples[i].first); //sample RMSE

    // sample parameters set
    extractParametersValues(rsmpl, mcSamples[i].second);
    // extract observables values set
    extractObservableValues(rsmpl, mcSamples[i].second);
  }
  saveResults(results);
}
