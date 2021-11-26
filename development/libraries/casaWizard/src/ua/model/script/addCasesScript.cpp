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

void AddCasesScript::writeScriptContents(QFile& file) const
{
  const QString stateFileNameDoE = uaScenario().runLocation() + "/" + uaScenario().iterationDirName() + "/" + uaScenario().stateFileNameDoE();

  QTextStream out(&file);

  out << writeBaseProject(uaScenario().project3dFilename());
  out << writeLoadState(stateFileNameDoE);

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
  out << writeSaveState(stateFileNameDoE);
}

QString AddCasesScript::scriptFilename() const
{
  return QString("addCasesScript.casa");
}

} // namespace ua

} // namespace casaWizard
