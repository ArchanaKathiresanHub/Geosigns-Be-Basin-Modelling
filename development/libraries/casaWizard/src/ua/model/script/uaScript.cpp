#include "uaScript.h"
#include "model/script/WizardDataToCasaScriptMapper.h"

#include "model/input/cmbProjectReader.h"
#include "model/logger.h"
#include "model/proxy.h"
#include "model/uaScenario.h"

#include "Qt_Utils.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QTextStream>

namespace casaWizard
{

namespace ua
{


UAScript::UAScript(UAScenario& scenario) :
   CasaScript{scenario.workingDirectory()},
   scenario_{scenario},
   proxy_{scenario_.proxy()}
{
}

QString UAScript::workingDirectory() const
{
   return QString(scenario_.workingDirectory());
}

const CasaScenario& UAScript::scenario() const
{
   return scenario_;
}

UAScenario& UAScript::uaScenario() const
{
   return scenario_;
}

bool UAScript::validateScenario() const
{
   const QString runFolderStr = scenario_.workingDirectory() + "/" + scenario_.runLocation();
   const QDir runFolderDir = QDir{runFolderStr};

   if (!runFolderDir.exists() ||
       !runFolderDir.isReadable())
   {
      return false;
   }

   const QString absolutePathStateFile = runFolderStr + "/" + scenario_.iterationDirName() + "/" + scenario_.stateFileNameDoE();
   const QFileInfo checkFile(absolutePathStateFile);

   if (!checkFile.exists())
   {
      Logger::log() << "State file \"" << absolutePathStateFile << "\" not found. Please run the DoE first. " << Logger::endl();
      return false;
   }

   if (!checkFile.isFile())
   {
      Logger::log() << "Path to state file is not a file" << Logger::endl();
      return false;
   }
   return true;
}

QString UAScript::proxyName() const
{
   return "\"proxy" + proxyRSPOrderNumberToTextMap().value(proxy_.order()) + proxy_.krigingMethod() + "Kriging" +  "\"";
}

QMap<int, QString> UAScript::proxyRSPOrderNumberToTextMap() const
{
   QMap<int, QString> proxyMap;

   proxyMap.insert(-1, "OrderMinusOne");
   proxyMap.insert(0,  "OrderZero");
   proxyMap.insert(1,  "FirstOrder");
   proxyMap.insert(2,  "SecondOrder");
   proxyMap.insert(3,  "ThirdOrder");

   return proxyMap;
}

QString UAScript::writeResponse(const QString& doeList) const
{
   return "response " + proxyName() + " " + doeList + " " + QString::number(proxy_.order()) + " " + proxy_.krigingMethod() + "\n";
}

QString UAScript::writeExportDataTxt(const QString& exportDataType, const QString& textFileName, const QString& doeList, const QString& proxyName) const
{
   QString exportData = "exportDataTxt \"" + exportDataType + "\" \"" + textFileName + "\"";

   if (!doeList.isEmpty())
   {
      exportData += (" " + doeList);
   }

   if (!proxyName.isEmpty())
   {
      exportData += (" " + proxyName);
   }

   return exportData + "\n";
}

void UAScript::writeTargets(QTextStream& out, bool prediction) const
{
   const CalibrationTargetManager& calibrationTargetManager = scenario_.calibrationTargetManager();
   for (const Well* well : calibrationTargetManager.activeWells())
   {
      for (const CalibrationTarget* calibrationTarget : well->activePropertyTargets())
      {
         out << writeCalibrationTarget(*calibrationTarget, well);
      }
   }

   if (prediction)
   {
      const PredictionTargetManager& predictionTargetManager = scenario_.predictionTargetManager();

      for (const PredictionTarget* predictionTarget : predictionTargetManager.predictionTargetsIncludingTimeSeries())
      {
         out << writePredictionTarget(*predictionTarget);
      }
   }

   out << writeRunDataDigger(); // Always run data digger on the targets
}

QString UAScript::writePredictionTarget(const PredictionTarget& predictionTarget) const
{
   using namespace wizardDataToCasaScriptMapper;

   QString out;
   for (const QString& property : predictionTarget.properties())
   {
      out += "target " + predictionTarget.casaCommand(property) + "\n";
   }

   return out;
}


QString UAScript::writeCalibrationTarget(const CalibrationTarget& target, const Well* well) const
{
   using namespace wizardDataToCasaScriptMapper;

   double ageCalibrationTarget = 0.0; // The measurments are only current day.
   double SAWeightCalibrationTarget = 1.0;

   return QString("target XYZPoint \"" + mapName(scenario_.calibrationTargetManager().getCauldronPropertyName(target.propertyUserName())) + "\" "
                  + qtutils::doubleToQString(well->x()) + " "
                  + qtutils::doubleToQString(well->y()) + " "
                  + qtutils::doubleToQString(target.z()) + " "
                  + qtutils::doubleToQString(ageCalibrationTarget) + " "
                  + qtutils::doubleToQString(target.value()) + " "
                  + qtutils::doubleToQString(target.standardDeviation()) + " "
                  + qtutils::doubleToQString(SAWeightCalibrationTarget) + " "
                  + qtutils::doubleToQString(target.uaWeight()) + " "
                  + "\n"
                  );
}

QString UAScript::writeAddDesignPoint(const QVector<double> parameters) const
{
   QString output{"addDesignPoint"};
   for (const double p : parameters)
   {
      output += " " + QString::number(p);
   }
   output += "\n";
   return output;
}

} // namespace ua

} // namespace casaWizard
