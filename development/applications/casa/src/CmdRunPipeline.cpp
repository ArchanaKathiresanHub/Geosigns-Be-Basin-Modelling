//
// Copyright (C) 2019 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "CmdRunPipeline.h"

#include "LogHandler.h"
#include "RunCaseImpl.h"
#include "ScenarioAnalysis.h"

#include "utilities.h"

#include <iostream>

CmdRunPipeline::CmdRunPipeline(CasaCommander& parent, const std::vector<std::string>& cmdPrms) :
  CasaCmd(parent, cmdPrms)
{
  m_cluster = m_prms.size() > 0 ? m_prms[0] : "";
  m_cldVer  = m_prms.size() > 1 ? m_prms[1] : "";

  casaAppUtils::checkCauldronVersion(m_cldVer);
}

void CmdRunPipeline::execute(std::unique_ptr<casa::ScenarioAnalysis>& sa)
{
   casa::RunManager & rm = sa->runManager();

   if ( ErrorHandler::NoError != rm.setCauldronVersion( m_cldVer.c_str() ) ||
        ErrorHandler::NoError != rm.setClusterName( m_cluster.c_str() ) )
   {
     throw ErrorHandler::Exception( rm.errorCode() ) << rm.errorMessage();
   }

   casa::RunCaseImpl runCase;
   runCase.mutateCaseTo(sa->baseCase(), sa->baseCase().projectFileName().c_str());

   LogHandler( LogHandler::INFO_SEVERITY ) << "Adding base case project to the queue and generating scripts...";

   if ( ErrorHandler::NoError != rm.scheduleCase( runCase, sa->scenarioID()) )
   {
      throw ErrorHandler::Exception( rm.errorCode() ) << rm.errorMessage();
   }

   if ( ErrorHandler::NoError != rm.runScheduledCases() )
   {
      throw ErrorHandler::Exception( rm.errorCode() ) << rm.errorMessage();
   }
}

void CmdRunPipeline::printHelpPage(const char* cmdName)
{
   std::cout << "  " << cmdName << R"( <HPC cluster name>) [<Cauldron version>]

    - runs the pipeline of applications in the present folder
     Here:
     <HPC cluster name> - cluster name, LOCAL | AMSGDC | HOUGDC.\n
     <Cauldron version> - (Optional) simulator version. Must be installed in IBS folder. Could be specified as "Default"
                          In this case the same simulator version as casa application will be used.)";
}
