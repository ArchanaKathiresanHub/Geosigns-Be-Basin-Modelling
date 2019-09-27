// Script to run casa to extract the optimized lithofraction for each case
#pragma once

#include "model/script/runScript.h"

#include <QStringList>
namespace casaWizard
{

namespace sac
{

class SACScenario;

class OptimizedLithofractionScript : public RunScript
{
public:
  explicit OptimizedLithofractionScript(const QString& baseDirectory);
  bool generateCommands() override;

  void addCase(const QString& runFolder);

private:
  void createScriptInFolder(const QString& folderName);

  const QString scriptFilename_;
  QStringList runFolders_;
};

} // namespace sac

} // namespace casaWizard
