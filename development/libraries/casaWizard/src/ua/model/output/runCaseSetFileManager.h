// Manager class for handling run caseSet directory file features
#pragma once

#include "model/writable.h"

#include <QFileInfo>

#include <QString>

namespace casaWizard
{

namespace ua
{

class RunCaseSetFileManager : public Writable
{
public:
  explicit RunCaseSetFileManager();

  void setIterationPath(const QString& project3dPath);
  double iterationDirFilesSize() const;
  double allIterationDirsFilesSize(const QString& project3dPath) const;
  bool isIterationDirEmpty() const;
  bool removeIterationDir();
  bool removeAllIterationDirs(const QString& project3dPath);
  QString iterationDirName() const;
  bool isIterationDirDeleted(const QString& project3dPath) const;
  QString iterationDirPath() const;
  QString caseSetDirPath() const;

  void writeToFile(ScenarioWriter& writer) const override;
  void readFromFile(const ScenarioReader& reader) override;
  void clear() override;

private:
  QFileInfoList getIterationPathList(const QString& project3dPath) const;

  QFileInfo iterationDirFileInfo_;
};

} // namespace ua

} // namespace casaWizard
