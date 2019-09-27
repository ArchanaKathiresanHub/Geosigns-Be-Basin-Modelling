//
// Copyright (C) 2019 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "CmdAddDesignPoint.h"

#include "RunCaseImpl.h"
#include "RunCaseSetImpl.h"
#include "ScenarioAnalysis.h"
#include "SUMlibUtils.h"

#include <iostream>

CmdAddDesignPoint::CmdAddDesignPoint(CasaCommander& parent, const std::vector<std::string>& cmdPrms) :
  CasaCmd(parent, cmdPrms),
  m_parameterValues{}
{
  for (const std::string& prm : m_prms)
  {
    m_parameterValues.push_back( std::atof( prm.c_str() ) );
  }
}

void CmdAddDesignPoint::execute(std::unique_ptr<casa::ScenarioAnalysis>& sa)
{
  casa::VarSpace& varSpace = sa->varSpace();
  casa::RunCaseSet& doeCaseSet = sa->doeCaseSet();

  const size_t numberInfluentialParameters = varSpace.size();
  const size_t numberProvidedParameters = m_parameterValues.size();

  if (numberInfluentialParameters != numberProvidedParameters)
  {
    throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Mismatch in the amount of provided parameters. You provided " << numberProvidedParameters << " parameters, but " << numberInfluentialParameters << " influential parameters were defined";
  }

  std::shared_ptr<casa::RunCase> runCase(new casa::RunCaseImpl());
  SUMlib::Case sumlibCase(m_parameterValues);
  sumext::convertCase(sumlibCase, varSpace, *runCase);
  doeCaseSet.addUserDefinedCase(runCase);
}

void CmdAddDesignPoint::printHelpPage(const char* cmdName)
{
  std::cout << "  " << cmdName << " <first influential parameter value> <second influential parameter value> <...> \n\n"
            << "  Add a single design point to the list of experiment with specified values for the influential parameters\n"
            << "  The number of arguments has to match the number of influential parameters.\n"
            << "  To select these manual design points in the response surface design, use the name \"" << casa::RunCaseSetImpl::userDefinedName << "\"\n" << std::endl;
}


