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

//FileSystem
#include "FilePath.h"

// genex6
#include "Simulator.h"

// genex6_kernel
#include "AdsorptionSimulatorFactory.h"
#include "SourceRockNode.h"

#include "LogHandler.h"

#include <fstream>

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
  m_projectMgr.reset(new Genex0dProjectManager(m_inData.projectFilename, m_inData.xCoord, m_inData.yCoord));
  m_formationMgr.reset(new Genex0dFormationManager(m_projectMgr->projectHandle(), m_inData.formationName, m_inData.xCoord, m_inData.yCoord));
  LogHandler(LogHandler::INFO_SEVERITY) <<  "The selected formation " << m_inData.formationName << " is "
                                         << (m_formationMgr->isFormationSourceRock() ? "" : "not ") << "source rock";

  m_projectMgr->computeAgesFromAllSnapShots(m_formationMgr->depositionTimeTopSurface());
  m_projectMgr->setTopSurface(m_formationMgr->topSurfaceName());

  m_sourceRock.reset(new Genex0dSourceRock(m_inData.sourceRockType, *m_projectMgr, *m_formationMgr));
}


void Genex0d::setAdsorptionHistory()
{
  Genex6::SourceRockNode * srNode = new Genex6::SourceRockNode(m_sourceRock->getSourceRockNode());
  const Genex6::Simulator & srSimulator = m_sourceRock->simulator();
  const Genex6::ChemicalModel & chemModel = srSimulator.getChemicalModel();
  DataAccess::Interface::ProjectHandle * projectHandle = m_projectMgr->projectHandle();


  //---------------------
  // preprocess
  //---------------------
  // computeSnapshotIntervals


  //---------------------
  // process
  //---------------------


  Genex6::NodeAdsorptionHistory * adsorptionHistory = Genex6::AdsorptionSimulatorFactory::getInstance().allocateNodeAdsorptionHistory(
        chemModel.getSpeciesManager(),
        projectHandle,
        "GenexSimulator"
        );

  if ( adsorptionHistory ==  nullptr)
  {
    throw Genex0dException() << "Fatal error, node adsorption history could not be initiated!";
  }

  adsorptionHistory->collect(srNode);

  // -------------------------

  std::string fileName;
  std::stringstream buffer;

  buffer << "History_"
         << projectHandle->getProjectName ()
         << "_"
         << "genex"
         << "_" << m_inData.formationName;
  buffer << ".dat";

  fileName = buffer.str();

  if (!projectHandle->makeOutputDir())
  {
    throw Genex0dException() << "Fatal error, output directory doesn't exist or new output directory could not be generated!";
  }

  ibs::FilePath filePath(projectHandle->getOutputDir());
  filePath << fileName;
  std::ofstream historyFile(filePath.cpath(), std::ios::out);

  adsorptionHistory->write(historyFile);
  historyFile.close ();
}



void Genex0d::setSourceRockInput(const double inorganicDensity)
{
  m_sourceRock->setToCIni(m_inData.ToCIni);
  m_sourceRock->setHCVRe05(m_inData.HCVRe05);
  m_sourceRock->setSCVRe05(m_inData.SCVRe05);
  m_sourceRock->computeData(m_formationMgr->getThickness(), inorganicDensity,
                            m_projectMgr->agesAll(),
                            m_projectMgr->requestPropertyHistory("Temperature"),
                            m_projectMgr->requestPropertyHistory("Pressure"));

//  setAdsorptionHistory();
}

void Genex0d::run()
{
  initialize();
  double inorganicDensity = m_formationMgr->getInorganicDensity();
  LogHandler(LogHandler::INFO_SEVERITY) << " # Inorganic density #" << inorganicDensity;
  setSourceRockInput(inorganicDensity);
}

void Genex0d::printResults(const std::string & outputFileName) const
{
  const Genex6::SourceRockNode & srNode = m_sourceRock->getSourceRockNode();
  srNode.PrintBenchmarkOutput(outputFileName, m_sourceRock->simulator());
}

} // namespace genex0d
