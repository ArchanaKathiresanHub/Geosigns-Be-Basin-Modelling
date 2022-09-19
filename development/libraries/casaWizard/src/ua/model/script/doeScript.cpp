#include "doeScript.h"

#include "model/doeOption.h"
#include "model/influentialParameter.h"
#include "model/uaScenario.h"

#include <QFile>
#include <QTextStream>
#include <QDateTime>

namespace casaWizard
{

namespace ua
{

DoEScript::DoEScript(UAScenario& scenario) :
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

  QVector<DoeOption*> selectedOptions = uaScenario().doeSelected();
  out << writeDOE(selectedOptions);

  for (auto option : selectedOptions)
  {
     if (option->name() == "UserDefined")
     {
        QVector<QVector<double>> pointsToRun = uaScenario().manualDesignPointManager().pointsToRun();
        for (int i = 0; i < pointsToRun.size(); ++i)
        {
          out << writeAddDesignPoint(pointsToRun[i]);
        }
        break;
     }
  }

  out << writeLocation(uaScenario().runLocation());
  out << writeRun(uaScenario().clusterName());
  out << writeExportDataTxt("DoeIndices", uaScenario().doeIndicesTextFileName());
  out << writeExportDataTxt("RunCasesSimulationStates", uaScenario().simStatesTextFileName());
  out << writeSaveState(uaScenario().updateStateFileNameDoE());
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
