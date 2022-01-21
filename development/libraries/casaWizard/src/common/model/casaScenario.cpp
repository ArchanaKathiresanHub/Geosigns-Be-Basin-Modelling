#include "casaScenario.h"

#include "input/projectReader.h"
#include "model/logger.h"
#include "model/scenarioReader.h"
#include "model/scenarioWriter.h"
#include "model/output/workspaceGenerator.h"
#include "output/projectWriter.h"

#include <QFileInfo>
#include <QSet>
#include <QStringList>
#include <QTextStream>

namespace casaWizard
{

const QString defaultClusterName{"LOCAL"};
const QString defaultApplication{"fastcauldron \"-itcoupled\""};
const QString defaultRunLocation{"CaseSet"};
const bool defaultExpertUser{false};
const int defaultNumberCPUs{1};

CasaScenario::CasaScenario(ProjectReader* projectReader) :
  applicationName_(defaultApplication),
  calibrationTargetManager_(),
  calibrationTargetManagerWellPrep_(),
  objectiveFunctionManager_(),
  clusterName_(defaultClusterName),
  expertUser_(defaultExpertUser),
  numberCPUs_(defaultNumberCPUs),
  project3dFilename_(""),
  projectReader_(projectReader),
  original1dDirectory_{"original1d"},
  runLocation_(defaultRunLocation),
  workingDirectory_("")
{
}

QString CasaScenario::clusterName() const
{
  return clusterName_;
}

void CasaScenario::setClusterName(const QString& clusterName)
{
  clusterName_ = clusterName;
}

QString CasaScenario::applicationName() const
{
  return applicationName_;
}

void CasaScenario::setApplicationName(const QString& applicationName)
{
  applicationName_ = applicationName;
}

QString CasaScenario::project3dFilename() const
{
  return project3dFilename_;
}

QString CasaScenario::project3dPath() const
{
  if (workingDirectory_.isEmpty() && project3dFilename_.isEmpty())
  {
    return "";
  }
  return workingDirectory_ + "/" + project3dFilename_;
}

void CasaScenario::setProject3dFilePath(const QString& project3dPath)
{
  QFileInfo info(project3dPath);
  project3dFilename_ = info.fileName();

  loadProject3dFile();
}

void CasaScenario::loadProject3dFile() const
{
  if (projectReader_)
  {
    projectReader_->load(project3dPath());
  }
}

void CasaScenario::applyObjectiveFunctionOnCalibrationTargets()
{
  calibrationTargetManager_.applyObjectiveFunctionOnCalibrationTargets(objectiveFunctionManager_);
}

void CasaScenario::updateObjectiveFunctionFromTargets()
{
  QSet<QString> targetVariableUserNames;
  for (const CalibrationTarget *const target : calibrationTargetManager_.calibrationTargets())
  {
    targetVariableUserNames.insert(target->propertyUserName());
  }
  objectiveFunctionManager_.setVariables(targetVariableUserNames.toList(), calibrationTargetManager_.userNameToCauldronNameMapping());
  applyObjectiveFunctionOnCalibrationTargets();
}

bool CasaScenario::propertyIsActive(const QString& property) const
{
  return objectiveFunctionManager_.enabled(objectiveFunctionManager_.indexOfUserName(property));
}

void CasaScenario::updateRelevantProperties(ProjectWriter& projectWriter)
{
  QStringList activePropertyUserNames = calibrationTargetManager().activePropertyUserNames();
  QStringList activePropertyCauldronNames;
  for (const QString& activePropertyUserName: activePropertyUserNames)
  {
    activePropertyCauldronNames.push_back(calibrationTargetManager_.getCauldronPropertyName(activePropertyUserName));
  }

  projectWriter.setRelevantOutputParameters(activePropertyCauldronNames);
}

void CasaScenario::setWorkingDirectory(const QString& workingDirectory)
{
  workingDirectory_ = workingDirectory;
}

bool CasaScenario::expertUser() const
{
  return expertUser_;
}

void CasaScenario::setExpertUser(bool expertUser)
{
  expertUser_ = expertUser;
}

int CasaScenario::numberCPUs() const
{
  return numberCPUs_;
}

void CasaScenario::setNumberCPUs(int numberCPUs)
{
  numberCPUs_ = numberCPUs;
}

void CasaScenario::clearWellsAndCalibrationTargets()
{
  calibrationTargetManager_.clear();
}

CalibrationTargetManager& CasaScenario::calibrationTargetManager()
{
  return calibrationTargetManager_;
}

const CalibrationTargetManager& CasaScenario::calibrationTargetManager() const
{
  return calibrationTargetManager_;
}

CalibrationTargetManager& CasaScenario::calibrationTargetManagerWellPrep()
{
  return calibrationTargetManagerWellPrep_;
}

const CalibrationTargetManager& CasaScenario::calibrationTargetManagerWellPrep() const
{
  return calibrationTargetManagerWellPrep_;
}

ObjectiveFunctionManager& CasaScenario::objectiveFunctionManager()
{
  return objectiveFunctionManager_;
}

const ObjectiveFunctionManager& CasaScenario::objectiveFunctionManager() const
{
  return objectiveFunctionManager_;
}

QString CasaScenario::workingDirectory() const
{
  return workingDirectory_;
}

QString CasaScenario::original1dDirectory() const
{
  return workingDirectory() + "/" + original1dDirectory_;
}

QString CasaScenario::runLocation() const
{
  return runLocation_;
}

void CasaScenario::setRunLocation(const QString& runLocation)
{
  runLocation_ = runLocation;
}

const ProjectReader& CasaScenario::projectReader() const
{
  return *projectReader_;
}

void CasaScenario::writeToFile(ScenarioWriter& writer) const
{
  writer.writeValue("CasaScenarioVersion", 2);
  writer.writeValue("workingDirectory", workingDirectory_);  
  writer.writeValue("clusterName", clusterName_);
  writer.writeValue("applicationName", applicationName_);
  writer.writeValue("project3dFilename", project3dFilename_);
  writer.writeValue("runLocation", runLocation_);
  writer.writeValue("expertUser", expertUser_);
  writer.writeValue("numberCPUs", numberCPUs_);

  calibrationTargetManager_.writeToFile(writer);
  objectiveFunctionManager_.writeToFile(writer);
}

void CasaScenario::readFromFile(const ScenarioReader& reader)
{
  const int version = reader.readInt("CasaScenarioVersion");

  workingDirectory_ = reader.readString("workingDirectory");
  clusterName_ = reader.readString("clusterName");
  applicationName_ = reader.readString("applicationName");
  project3dFilename_ = reader.readString("project3dFilename");
  runLocation_ = reader.readString("runLocation");
  expertUser_ = reader.readBool("expertUser");
  numberCPUs_ = reader.readInt("numberCPUs");

  loadProject3dFile();

  calibrationTargetManager_.readFromFile(reader);
  if (version >= 2)
  {
    objectiveFunctionManager_.readFromFile(reader);
  }
  else
  {
    QVector<ObjectiveFunctionValue> values = reader.readVector<ObjectiveFunctionValue>("objectiveFunction");
    objectiveFunctionManager_.setValues(values);
    for (const auto& val : objectiveFunctionManager_.values())
    {
      if (calibrationTargetManager().userNameToCauldronNameMapping().value(val.variableUserName(), "Unknown") == "Unknown")
      {
        calibrationTargetManager_.addToMapping(val.variableUserName(), val.variableUserName());
      }
    }
  }
  objectiveFunctionManager_.setUserNameToCauldronNameMapping(calibrationTargetManager_.userNameToCauldronNameMapping());
}

void CasaScenario::clear()
{
  clusterName_ = defaultClusterName;
  applicationName_ = defaultApplication;
  project3dFilename_ = "";
  runLocation_ = defaultRunLocation;
  expertUser_ = defaultExpertUser;
  numberCPUs_ = defaultNumberCPUs;
  workingDirectory_ = "";

  calibrationTargetManager_.clear();
  objectiveFunctionManager_.clear();
}

} // namespace casaWizard
