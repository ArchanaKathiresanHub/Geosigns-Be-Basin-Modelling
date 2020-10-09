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
#include "Genex0dSimulator.h"
#include "Genex0dSimulatorFactory.h"

// cmbAPI
#include "ErrorHandler.h"

#include "LogHandler.h"

namespace genex0d
{

Genex0d::Genex0d(const Genex0dInputData & inputData) :
  m_inData{inputData},
  m_formationMgr{nullptr},
  m_projectMgr{nullptr},
  m_gnx0dSimulatorFactory{nullptr},
  m_gnx0dSimulator{nullptr}
{
  LogHandler(LogHandler::INFO_SEVERITY) << "Running genex0d on " << m_inData.projectFilename;
}

Genex0d::~Genex0d()
{
}

void Genex0d::loadSimulator()
{
  m_gnx0dSimulatorFactory.reset(new Genex0dSimulatorFactory);
  m_gnx0dSimulator.reset(Genex0dSimulator::CreateFrom(m_inData.projectFilename, m_gnx0dSimulatorFactory.get()));
  if (m_gnx0dSimulator == nullptr)
  {
    throw Genex0dException() << "Genex0d simulator could not be loaded!";
  }
}

void Genex0d::loadFormation()
{
  m_formationMgr.reset(new Genex0dFormationManager(*m_gnx0dSimulator, m_inData.formationName, m_inData.xCoord, m_inData.yCoord));
  LogHandler(LogHandler::INFO_SEVERITY) <<  "The selected formation " << m_inData.formationName << " is "
                                         << (m_formationMgr->isFormationSourceRock() ? "" : "not ") << "source rock";
}

void Genex0d::loadProjectMgr()
{
  m_projectMgr.reset(new Genex0dProjectManager(*m_gnx0dSimulator, m_inData.projectFilename, m_inData.xCoord, m_inData.yCoord, m_formationMgr->topSurfaceName(), m_inData.formationName));
  m_projectMgr->computeAgesFromAllSnapShots(m_formationMgr->depositionTimeTopSurface());
}

void Genex0d::initialize()
{
  loadSimulator();
  loadFormation();
  loadProjectMgr();

  m_gnx0dSimulator->deletePropertyValues(DataAccess::Interface::RESERVOIR , nullptr, nullptr, nullptr, nullptr, nullptr,
                                         DataAccess::Interface::MAP);

  LogHandler(LogHandler::INFO_SEVERITY) << "Successfully initialized the genex0d simulator!";
}

void Genex0d::run()
{
  LogHandler(LogHandler::INFO_SEVERITY) << "Runing genex0d ...";

  m_projectMgr->requestPropertyHistory("Temperature");
  m_projectMgr->requestPropertyHistory("MaxVes");
  m_projectMgr->requestPropertyHistory("Ves");
  m_projectMgr->requestPropertyHistory("Vr");
  m_projectMgr->requestPropertyHistory("Pressure");

  // Only needed for adsorption, only load when needed???
  m_projectMgr->requestPropertyHistory("Permeability");
  m_projectMgr->requestPropertyHistory("Porosity");
  m_projectMgr->requestPropertyHistory("LithoStaticPressure");
  m_projectMgr->requestPropertyHistory("HydroStaticPressure");

  m_projectMgr->extract();

  if (!m_gnx0dSimulator->run(m_formationMgr->formation(), m_inData, m_formationMgr->indI(), m_formationMgr->indJ(),
                             m_formationMgr->getThickness(), m_formationMgr->getInorganicDensity(),
                             m_projectMgr->agesAll(),
                             m_projectMgr->getValues("Temperature"),
                             m_projectMgr->getValues("Ves"),
                             m_projectMgr->getValues("Vr"),
                             m_projectMgr->getValues("Pressure"),
                             m_projectMgr->getValues("Permeability"),
                             m_projectMgr->getValues("Porosity"),
                             m_projectMgr->getValues("LithoStaticPressure"),
                             m_projectMgr->getValues("HydroStaticPressure")))
  {
    throw Genex0dException() << "Genex0d simulator could not be initiated!";
  }

  LogHandler(LogHandler::INFO_SEVERITY) << "Finished running genex0d!";
}

} // namespace genex0d
