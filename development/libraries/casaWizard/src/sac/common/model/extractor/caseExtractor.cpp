//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "caseExtractor.h"

#include "model/functions/sortedByXWellIndices.h"
#include "model/input/caseDataCreator.h"
#include "model/logger.h"
#include "model/SacScenario.h"

#include <QDir>

namespace casaWizard
{

namespace sac
{

CaseExtractor::CaseExtractor(const SacScenario& scenario, const QString& iterationPath) :
  m_iterationPath{iterationPath},
  m_scenario{scenario}
{
  if (iterationPath.isEmpty())
  {
    obtainIterationPath();
  }
}

CaseExtractor::~CaseExtractor()
{
}

void CaseExtractor::extract()
{
  const CalibrationTargetManager& ctManager = m_scenario.calibrationTargetManager();
  const QVector<const Well*>& wells = ctManager.wells();
  QVector<int> sortedIndices = casaWizard::functions::sortedByXYWellIndices(wells);
  int caseIndex{0};

  for (const int wellIndex : sortedIndices)
  {
    if ( wells[wellIndex]->isIncludedInOptimization() )
    {
      extractCase(wells[wellIndex], ++caseIndex);
    }
  }
}

void CaseExtractor::readCaseData()
{
  dataCreator().read();
}

QString CaseExtractor::iterationPath() const
{
  return m_iterationPath;
}

const SacScenario& CaseExtractor::scenario()
{
  return m_scenario;
}

void CaseExtractor::extractCase(const Well* well, const int caseIndex)
{
  if (m_iterationPath.isEmpty())
  {
    Logger::log() << "Cannot read for " << well->name() << " because the iteration path is not set" << Logger::endl();
    return;
  }
  QString relativeDataFolder;
  if (m_iterationPath.contains(m_scenario.original1dDirectory()))
  {
    relativeDataFolder = well->name();
  }
  else
  {
    relativeDataFolder = "Case_" + QString::number(caseIndex);
  }

  QDir data{m_iterationPath + "/" + relativeDataFolder};
  if (!data.exists())
  {
    Logger::log() << "Folder <" << data.absolutePath() << "> for reading " << well->name() << " not found" << Logger::endl();
    return;
  }

  updateCaseScript(well->id(), relativeDataFolder);
}

void CaseExtractor::obtainIterationPath()
{
  const QString runFolder = m_scenario.calibrationDirectory() + "/" + m_scenario.runLocation();
  QDir dir{runFolder};
  if (!dir.exists())
  {
    Logger::log() << "Error: no run folder found while reading the well trajectory. <" << runFolder << "> not found" << Logger::endl();
    return;
  }
  const QStringList iterations = dir.entryList({"Iteration_*"}, QDir::Filter::Dirs, QDir::Time | QDir::Reversed);
  if (iterations.isEmpty())
  {
    Logger::log() << "Error: no results found in folder <" << runFolder << ">" << Logger::endl();
    return;
  }

  m_iterationPath = runFolder + "/" + iterations.last();
}

} // namespace sac

} // namespace casaWizard
