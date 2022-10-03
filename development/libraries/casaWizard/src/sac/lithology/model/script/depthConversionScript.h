// Class for a fast depth conversion
#pragma once

#include "model/script/runScript.h"

#include "SDUWorkLoadManager.h"

#include <QString>

#include <memory>

class QFile;

namespace casaWizard
{

namespace sac
{

class SacLithologyScenario;

class DepthConversionScript : public RunScript
{
public:
  explicit DepthConversionScript(const SacLithologyScenario& scenario, const QString& baseDirectory, const workloadmanagers::WorkLoadManagerType& workloadManagerType);
  ~DepthConversionScript() override;

private:
  bool generateCommands() override;
  QString getDepthConversionCommand() const;
  void writeScriptContents(QFile& file) const;
  bool runLocally() const;

  const SacLithologyScenario& scenario_;
  QString scriptFilename_;
  std::unique_ptr<workloadmanagers::WorkLoadManager> workloadManager_;
};

} // namespace sac

} // namespace casaWizard
