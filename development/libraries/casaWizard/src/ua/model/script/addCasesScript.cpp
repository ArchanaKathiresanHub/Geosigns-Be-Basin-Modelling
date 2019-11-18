#include "addCasesScript.h"

#include "model/uaScenario.h"

#include <QFile>
#include <QTextStream>

namespace casaWizard
{

namespace ua
{

AddCasesScript::AddCasesScript(const UAScenario& scenario) :
  UAScript(scenario)
{
}

bool AddCasesScript::prepareKill() const
{
  return createStopExecFile();
}

void AddCasesScript::writeScriptContents(QFile& file) const
{
  QTextStream out(&file);

  out << writeBaseProject(uaScenario().project3dFilename());
  out << writeLoadState(uaScenario().runLocation() + "/" + uaScenario().iterationDirName() + "/" + uaScenario().stateFileNameDoE());

  const ManualDesignPointManager& designPointManager = uaScenario().manualDesignPointManager();
  const QVector<bool> completed = designPointManager.completed();
  for (int i = 0; i < designPointManager.numberOfPoints(); ++i)
  {
    if (!completed[i])
    {
      out << writeAddDesignPoint(designPointManager.getDesignPoint(i));
    }
  }

  out << writeLocation(uaScenario().runLocation(), true);
  out << writeRun(uaScenario().clusterName());
  out << writeSaveState(uaScenario().stateFileNameDoE());
}

} // namespace ua

} // namespace casaWizard
