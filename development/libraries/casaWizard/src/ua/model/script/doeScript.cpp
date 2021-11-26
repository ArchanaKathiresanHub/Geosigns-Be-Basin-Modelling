#include "doeScript.h"

#include "model/doeOption.h"
#include "model/influentialParameter.h"
#include "model/uaScenario.h"

#include <QFile>
#include <QTextStream>

namespace casaWizard
{

namespace ua
{

DoEScript::DoEScript(const UAScenario& scenario) :
  UAScript(scenario)
{
}

bool DoEScript::validateScenario() const
{
  return true;
}

QString DoEScript::scriptFilename() const
{
  return QString("doeScript.casa");
}

void DoEScript::writeScriptContents(QFile& file) const
{
  QTextStream out(&file);

  out << writeApp(uaScenario().numberCPUs(), uaScenario().applicationName());
  out << writeBaseProject(uaScenario().project3dFilename());

  const InfluentialParameterManager& manager = uaScenario().influentialParameterManager();
  for (const InfluentialParameter* influentialParameter : manager.influentialParameters())
  {
    out << writeInfluentialParameter(influentialParameter);
  }

  out << writeDOE(uaScenario().doeSelected());

  const ManualDesignPointManager& designPointManager = uaScenario().manualDesignPointManager();
  for (int i = 0; i < designPointManager.numberOfPoints(); ++i)
  {
    out << writeAddDesignPoint(designPointManager.getDesignPoint(i));
  }

  out << writeLocation(uaScenario().runLocation());
  out << writeRun(uaScenario().clusterName());
  out << writeSaveState(uaScenario().stateFileNameDoE());
}

QString DoEScript::writeInfluentialParameter(const InfluentialParameter* influentialParameter) const
{
  return QString("varprm " + influentialParameter->casaName() + " " + influentialParameter->arguments().toString()) + "\n";
}

QString DoEScript::writeDOE(const QVector<DoeOption*>& doeOptions) const
{
  QString doeOutput;
  for (const DoeOption* doe : doeOptions)
  {
    if (doe->name() == "UserDefined")
    {
      continue;
    }

    if (doe->hasCalculatedDesignPoints())
    {
      doeOutput += QString("doe \"" + doe->name() + "\"\n");
    }
    else
    {
      doeOutput += QString("doe \"" + doe->name() + "\" " + QString::number(doe->nDesignPoints()) + "\n");
    }
  }
  return doeOutput;
}

} // namespace ua

} // namespace casaWizard
