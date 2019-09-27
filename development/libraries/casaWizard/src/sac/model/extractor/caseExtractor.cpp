#include "caseExtractor.h"

#include "model/functions/sortedByXWellIndices.h"
#include "model/input/caseDataCreator.h"
#include "model/logger.h"
#include "model/sacScenario.h"
#include "model/script/runScript.h"

#include <QDir>

namespace casaWizard
{

namespace sac
{

CaseExtractor::CaseExtractor(SACScenario& scenario) :
  iterationPath_{""},
  scenario_{scenario}
{
  obtainIterationPath();
}

void CaseExtractor::extract()
{
  const CalibrationTargetManager& ctManager = scenario_.calibrationTargetManager();
  const QVector<Well>& wells = ctManager.wells();
  QVector<int> sortedIndices = casaWizard::functions::sortedByXWellIndices(wells);
  int caseIndex{0};

  for (const int wellIndex : sortedIndices)
  {
    if (wells[wellIndex].isActive() )
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
  return iterationPath_;
}

SACScenario& CaseExtractor::scenario()
{
  return scenario_;
}

void CaseExtractor::extractCase(const Well& well, const int caseIndex)
{
  if (iterationPath_.isEmpty())
  {
    Logger::log() << "Cannot read for " << well.name() << " because the iteration path is not set" << Logger::endl();
    return;
  }
  const QString relativeDataFolder = "Case_" + QString::number(caseIndex);

  QDir data{iterationPath_ + "/" + relativeDataFolder};
  if (!data.exists())
  {
    Logger::log() << "Folder <" << data.absolutePath() << "> for reading " << well.name() << " not found" << Logger::endl();
    return;
  }

  updateCaseScript(well.id(), relativeDataFolder);
}

void CaseExtractor::obtainIterationPath()
{
  const QString runFolder = scenario_.calibrationDirectory() + "/" + scenario_.runLocation();
  QDir dir{runFolder};
  if (!dir.exists())
  {
    Logger::log() << "Error: no run folder found while reading the well trajectory. <" << runFolder << "> not found" << Logger::endl();
    return;
  }
  const QStringList iterations = dir.entryList({"Iteration_*"}, QDir::Filter::Dirs, QDir::SortFlag::Name);
  if (iterations.isEmpty())
  {
    Logger::log() << "Error: no results found in folder <" << runFolder << ">" << Logger::endl();
    return;
  }
  iterationPath_ = runFolder + "/" + iterations.last();
}

} // namespace sac

} // namespace casaWizard
