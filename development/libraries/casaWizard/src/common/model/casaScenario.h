// Class that contains all data to run a casa scenario
#pragma once

#include "calibrationTargetManager.h"
#include "model/input/projectReader.h"
#include "writable.h"

#include <QMap>
#include <QString>
#include <QVector>

#include <memory>

namespace casaWizard
{

class ProjectWriter;

class CasaScenario : public Writable
{
public:
  CasaScenario(ProjectReader* projectReader);
  virtual ~CasaScenario() override = default;

  QString workingDirectory() const;
  void setWorkingDirectory(const QString& workingDirectory);

  QString original1dDirectory() const;

  QString clusterName() const;
  void setClusterName(const QString& clusterName);

  QString applicationName() const;
  void setApplicationName(const QString& applicationName);

  QString project3dFilename() const;
  QString project3dPath() const;
  void setProject3dFileNameAndLoadFile(const QString& project3dPath);

  QString runLocation() const;
  void setRunLocation(const QString& runLocation);

  bool expertUser() const;
  void setExpertUser(bool expertUser);

  int numberCPUs() const;
  void setNumberCPUs(int numberCPUs);

  void clearWellsAndCalibrationTargets();

  CalibrationTargetManager& calibrationTargetManager();
  const CalibrationTargetManager& calibrationTargetManager() const;

  CalibrationTargetManager& calibrationTargetManagerWellPrep();
  const CalibrationTargetManager& calibrationTargetManagerWellPrep() const;

  ObjectiveFunctionManager& objectiveFunctionManager();
  const ObjectiveFunctionManager& objectiveFunctionManager() const;

  const ProjectReader& projectReader() const;

  virtual void writeToFile(ScenarioWriter& writer) const override;
  virtual void readFromFile(const ScenarioReader& reader) override;
  virtual void clear() override;
  virtual QString iterationDirName() const { return ""; }

  void updateRelevantProperties(casaWizard::ProjectWriter& projectWriter);
  virtual void loadProject3dFile() const;

  void applyObjectiveFunctionOnCalibrationTargets();
  void updateObjectiveFunctionFromTargets();
  bool propertyIsActive(const QString& property) const;

  QString defaultDirectoryLocation(const bool oneFolderHigher = false) const;

  virtual QString amsterdamDirPath() const;
  virtual QString houstonDirPath() const;


private:
  QString applicationName_;
  CalibrationTargetManager calibrationTargetManager_;
  CalibrationTargetManager calibrationTargetManagerWellPrep_;
  ObjectiveFunctionManager objectiveFunctionManager_;
  QString clusterName_;
  bool expertUser_;
  int numberCPUs_;
  QString project3dFilename_;
  std::unique_ptr<ProjectReader> projectReader_;
  QString original1dDirectory_;
  QString runLocation_;
  QString workingDirectory_;
};

} // namespace casaWizard

