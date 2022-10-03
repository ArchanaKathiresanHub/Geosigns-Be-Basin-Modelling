// Class for running track1d
// Track1D is ran for each property separately. It could be done also with a single call to track1D,
// but by rerunning the application all files are in the same format
#pragma once

#include "model/script/runScript.h"

#include <QStringList>
#include <QVector>

namespace casaWizard
{

namespace sac
{

class Track1DScript : public casaWizard::RunScript
{
public:
  explicit Track1DScript(const QString& baseDirectory, const QString& projectName);
  bool generateCommands() override;
  void addWell(const double x, const double y, const QString& propertyCauldronName, const QString& propertiesUserName, const QString& runFolder);

private:
  void addCommandProject(const int call, const QString& projectName );

  QVector<double> xCoordinate_;
  QVector<double> yCoordinate_;
  QStringList propertiesCauldronName_;
  QStringList propertiesUserName_;
  QStringList runFolders_;
  const QString projectName_;
};

} // namespace sac

} // namespace casaWizard
