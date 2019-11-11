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
#include "Genex0dGenexSourceRock.h"
#include "Genex0dInputData.h"
#include "Genex0dProjectManager.h"
#include "Genex0dSourceRock.h"

// CBMGenerics
#include "GenexResultManager.h"

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
  m_sourceRock{nullptr},
  m_genexSourceRock{nullptr}
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

  m_projectMgr->resetWindowingAndSampling(m_formationMgr->indI(), m_formationMgr->indJ());
  m_projectMgr->updateProjecthandle();

  m_projectMgr->computeAgesFromAllSnapShots(m_formationMgr->depositionTimeTopSurface());
  m_projectMgr->setTopSurface(m_formationMgr->topSurfaceName());

  m_sourceRock.reset(new Genex0dSourceRock(m_inData.sourceRockType, *m_projectMgr, m_formationMgr->formation()));

  setRequestedOutputProperties();
  m_genexSourceRock = new Genex0dGenexSourceRock(m_sourceRock->srProperties(), m_inData.formationName, m_inData.sourceRockType, m_projectMgr->projectHandle());
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


void Genex0d::setRequestedOutputProperties()
{
  setRequestedSpeciesOutputProperties();
  CBMGenerics::GenexResultManager & theResultManager = CBMGenerics::GenexResultManager::getInstance();

  DataAccess::Interface::ProjectHandle * projectHandle = m_projectMgr->projectHandle();
  DataAccess::Interface::ModellingMode theMode = projectHandle->getModellingMode();
  if (theMode != DataAccess::Interface::MODE3D)
  {
    throw Genex0dException() << "Can only run 3d mode!";
  }

  database::Table * timeIoTbl = projectHandle->getTable("FilterTimeIoTbl");
  database::Table::iterator tblIter;

  for (tblIter = timeIoTbl->begin(); tblIter != timeIoTbl->end(); ++tblIter)
  {
    database::Record * filterTimeIoRecord = * tblIter;

  }


}

void Genex0d::setRequestedSpeciesOutputProperties()
{
   CBMGenerics::ComponentManager & theManager = CBMGenerics::ComponentManager::getInstance();

   for (int i = 0; i < CBMGenerics::ComponentManager::NUMBER_OF_SPECIES; ++i)
   {
      if(!theManager.isSulphurComponent(i)) {
         m_expelledToCarrierBedProperties.push_back ( theManager.getSpeciesOutputPropertyName ( i, false ));
         m_expelledToSourceRockProperties.push_back ( theManager.getSpeciesOutputPropertyName ( i, true ));
      }
      m_expelledToCarrierBedPropertiesS.push_back ( theManager.getSpeciesOutputPropertyName ( i, false ));
      m_expelledToSourceRockPropertiesS.push_back ( theManager.getSpeciesOutputPropertyName ( i, true ));
   }

   std::sort ( m_expelledToSourceRockProperties.begin (), m_expelledToSourceRockProperties.end ());
   std::sort ( m_expelledToCarrierBedProperties.begin (), m_expelledToCarrierBedProperties.end ());
   std::sort ( m_expelledToSourceRockPropertiesS.begin (), m_expelledToSourceRockPropertiesS.end ());
   std::sort ( m_expelledToCarrierBedPropertiesS.begin (), m_expelledToCarrierBedPropertiesS.end ());
}

} // namespace genex0d
