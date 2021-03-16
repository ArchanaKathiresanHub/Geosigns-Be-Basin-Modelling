#include "scenarioBackup.h"

#include "casaScenario.h"
#include "scenarioWriter.h"

#include <QDateTime>
#include <QDir>

namespace casaWizard
{

namespace scenarioBackup
{

void backup(const CasaScenario& scenario)
{
  if (scenario.workingDirectory().isEmpty())
  {
    return;
  }

  const QString autoSaveDir{"autosave"};
  QDir dir{scenario.workingDirectory()};
  dir.mkdir(autoSaveDir);

  QDateTime now{QDateTime::currentDateTime()};
  const QString date{now.toString("yyyy-MM-dd-HH-mm-ss")};
  const QString writeFile{scenario.workingDirectory() + "/" + autoSaveDir + "/autosave-" + date + ".dat"};

  ScenarioWriter writer{writeFile};
  scenario.writeToFile(writer);
  writer.close();
}

} // namespace scenarioBackup

} // namespace casaWizard
