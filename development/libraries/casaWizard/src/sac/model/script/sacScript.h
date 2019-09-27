// Script to run the design of experiment
#pragma once

#include "model/script/casaScript.h"

class QFile;

namespace casaWizard
{

struct Well;

namespace sac
{

class SACScenario;
struct Lithofraction;
struct WellTrajectory;

class SACScript : public CasaScript
{
public:
  explicit SACScript(const SACScenario& scenario, const QString& baseDirectory);
  virtual ~SACScript() = default;

  const CasaScenario& scenario() const override;
  bool prepareKill() const override;
  QString scriptFilename() const override;
  QString workingDirectory() const;

private:
  const SACScenario& scenario_;

  void writeScriptContents(QFile& file) const override;
  bool validateScenario() const override;

  QString writeWellTrajectory(const WellTrajectory& trajectory, const QString& wellName) const;
  QString writeLithofraction(const Lithofraction& lithofraction) const;
};

} // sac

} // namespace casaWizard
