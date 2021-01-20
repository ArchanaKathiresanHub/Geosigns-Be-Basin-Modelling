// Class that contains all data to run a casa scenario
#pragma once

#include "calibrationTargetManager.h"
#include "model/input/projectReader.h"
#include "writable.h"

#include <QMap>
#include <QString>
#include <QVector>

#include <memory>

class QTextStream;

namespace casaWizard
{

class ExtractWellDataXlsx;

class CasaScenario : public Writable
{
public:
  CasaScenario(ProjectReader* projectReader);
  virtual ~CasaScenario() = default;

  QString workingDirectory() const;
  void setWorkingDirectory(const QString& workingDirectory);

  QString clusterName() const;
  void setClusterName(const QString& clusterName);

  QString applicationName() const;
  void setApplicationName(const QString& applicationName);

  QString project3dFilename() const;
  QString project3dPath() const;
  void setProject3dFilePath(const QString& project3dPath);

  QString runLocation() const;
  void setRunLocation(const QString& runLocation);

  bool expertUser() const;
  void setExpertUser(bool expertUser);

  int numberCPUs() const;
  void setNumberCPUs(int numberCPUs);

  void clearWellsAndCalibrationTargets();

  CalibrationTargetManager& calibrationTargetManager();
  const CalibrationTargetManager& calibrationTargetManager() const;

  const ProjectReader& projectReader() const;

  virtual void writeToFile(ScenarioWriter& writer) const override;
  virtual void readFromFile(const ScenarioReader& reader) override;
  virtual void clear() override;
  virtual QString iterationDirName() const { return ""; }

  void updateRelevantProperties();
private:
  void loadProject3dFile() const;

  std::unique_ptr<ProjectReader> projectReader_;
  QString workingDirectory_;
  QString clusterName_;
  QString applicationName_;
  QString project3dFilename_;
  QString runLocation_;
  bool expertUser_;

  int numberCPUs_;

  CalibrationTargetManager calibrationTargetManager_;
};

} // namespace casaWizard
