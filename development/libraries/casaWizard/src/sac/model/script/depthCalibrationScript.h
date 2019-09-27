// Class for a fast depth calibration
#pragma once

#include "model/script/runScript.h"

class QFile;

namespace casaWizard
{

namespace sac
{

class SACScenario;

class DepthCalibrationScript : public RunScript
{
public:
  explicit DepthCalibrationScript(const SACScenario& scenario, const QString& baseDirectory);

private:
  bool generateCommands() override;
  void writeScriptContents(QFile& file) const;

  const SACScenario& scenario_;
  QString scriptFilename_;
};

} // namespace sac

} // namespace casaWizard
