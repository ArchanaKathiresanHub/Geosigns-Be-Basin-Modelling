//
// Copyright (C) 2012-2017 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "Genex0d.h"

#include "CommonDefinitions.h"
#include "Genex0dFormationManager.h"
#include "Genex0dInputData.h"
#include "Genex0dProjectManager.h"
#include "Genex0dSourceRock.h"

#include "FilePath.h"

#include "SourceRockNode.h"

#include "LogHandler.h"

namespace genex0d
{

Genex0d::Genex0d(const Genex0dInputData & inputData) :
  m_inData{inputData},
  m_formationMgr{nullptr},
  m_projectMgr{nullptr},
  m_sourceRock{nullptr}
{
  if (m_inData.projectFilename.empty())
  {
    throw Genex0dException() << "Fatal error, empty project file name!";
  }

  LogHandler(LogHandler::INFO_SEVERITY) << "Running genex0d on " << m_inData.projectFilename;
}

Genex0d::~Genex0d()
{
}

void Genex0d::initialize()
{
  m_formationMgr.reset(new Genex0dFormationManager(m_inData.projectFilename, m_inData.formationName, m_inData.xCoord, m_inData.yCoord));
  LogHandler(LogHandler::INFO_SEVERITY) <<  "The selected formation " << m_inData.formationName << " is "
                                        << (m_formationMgr->isFormationSourceRock() ? "" : "not ") << "source rock";
  m_projectMgr.reset(new Genex0dProjectManager(m_inData.projectFilename, m_inData.xCoord, m_inData.yCoord, m_formationMgr->getTopSurfaceName()));
  m_sourceRock.reset(new Genex0dSourceRock(m_inData.sourceRockType));
}

void Genex0d::setSourceRockInput(const double inorganicDensity)
{
  m_sourceRock->setToCIni(m_inData.ToCIni);
  m_sourceRock->setHCVRe05(m_inData.HCVRe05);
  m_sourceRock->setSCVRe05(m_inData.SCVRe05);
  m_sourceRock->computeData(m_formationMgr->getThickness(), inorganicDensity,
                            m_projectMgr->agesFromMajorSnapShots(),
                            m_projectMgr->requestPropertyHistory("Temperature"),
                            m_projectMgr->requestPropertyHistory("Pressure"));
}

void Genex0d::run()
{
  initialize();

// History_Project_genex_Pot_MFSSR_Langhian_HI_1.dat
  ibs::FilePath fp(".");
  fp << "History_Project_genex_Pot_MFSSR_Langhian_HI_1.dat";

  int a = 1;


//  double inorganicDensity = m_formationMgr->getInorganicDensity();
//  LogHandler(LogHandler::INFO_SEVERITY) << " # Inorganic density #" << inorganicDensity;
//  setSourceRockInput(inorganicDensity);
}

void Genex0d::printResults(const std::string & outputFileName) const
{
  const Genex6::SourceRockNode & srNode = m_sourceRock->getSourceRockNode();
  srNode.PrintBenchmarkOutput(outputFileName, m_sourceRock->simulator());
}

} // namespace genex0d
