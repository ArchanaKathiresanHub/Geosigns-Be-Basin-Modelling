#include "addCasesScript.h"

#include "model/uaScenario.h"

#include <QFile>
#include <QTextStream>
#include <QDateTime>

namespace casaWizard
{

namespace ua
{

AddCasesScript::AddCasesScript(UAScenario& scenario) :
   UAScript(scenario)
{
}

void AddCasesScript::writeScriptContents(QFile& file) const
{
   QString stateFileNameDoE = uaScenario().runLocation() + "/" + uaScenario().iterationDirName() + "/" + uaScenario().stateFileNameDoE();

   QTextStream out(&file);

   out << writeBaseProject(uaScenario().project3dFilename());
   out << writeLoadState(stateFileNameDoE);

   QVector<QVector<double>> pointsToRun = uaScenario().manualDesignPointManager().pointsToRun();
   for (int i = 0; i < pointsToRun.size(); ++i)
   {
     out << writeAddDesignPoint(pointsToRun[i]);
   }

   out << writeLocation(uaScenario().runLocation(), true);
   out << writeRun(uaScenario().clusterName());
   out << writeExportDataTxt("RunCasesSimulationStates", uaScenario().simStatesTextFileName());
   out << writeExportDataTxt("DoeIndices", uaScenario().doeIndicesTextFileName());

   QString newStateFileName =uaScenario().updateStateFileNameDoE();
   stateFileNameDoE = uaScenario().runLocation() + "/" + uaScenario().iterationDirName() + "/" + newStateFileName;
   out << writeSaveState(stateFileNameDoE);
}

QString AddCasesScript::scriptFilename() const
{
   return QString("addCasesScript.casa");
}

} // namespace ua

} // namespace casaWizard
