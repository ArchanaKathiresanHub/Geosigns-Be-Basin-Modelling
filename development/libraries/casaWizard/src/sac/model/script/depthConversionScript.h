// Class for a fast depth conversion
#pragma once

#include "model/script/runScript.h"

#include <QString>

class QFile;

namespace casaWizard
{

namespace sac
{

class SACScenario;

class DepthConversionScript : public RunScript
{
public:
  explicit DepthConversionScript(const SACScenario& scenario, const QString& baseDirectory);

private:
  bool generateCommands() override;
  QString getDepthConversionCommand() const;
  void writeScriptContents(QFile& file) const;
  bool runLocally() const;

  const SACScenario& scenario_;
  QString scriptFilename_;

};

} // namespace sac

} // namespace casaWizard
